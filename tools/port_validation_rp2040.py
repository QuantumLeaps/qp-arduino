#!/usr/bin/env python3
"""Host-driven port-validation procedure for examples/port_validation-rp2040.

Drives the target through QSPY's UDP front-end (the same protocol QUTest and
QView use) and checks the QS records that come back. It needs no QUTest build
on the target: the real QV kernel, tick and transport are what is under test.

Usage (fixture on /dev/ttyACM0, qspy built by tools/quantum_leaps_tools.py):

    python tools/port_validation_rp2040.py --port /dev/ttyACM0 \
        --qspy .tools/quantum-leaps/qtools-7.3.4/bin/qspy --out results.json

The script starts qspy itself (text output to a file next to --out), attaches
to its UDP port, then runs these procedures and records pass/fail for each:

  info          QS-RX: target answers QS_RX_INFO with Trg-Info
  ticking       time events + dispatch: VALIDATION_STAT counters advance
  publish       pub/sub: command 1 publishes VALIDATE_SIG, Validator receives it
  dynamic       dynamic events + queue: command 2 posts a pool event, DYN_SIG
                is dispatched and the pool is used and freed
  roles         probe role switching: command 5 answers with PROBE_ROLE
  reset-count   command 7 resets the counters, command 6 reports them
  assertion     command 4 provokes Q_ERROR: the target emits =ASSERT= and
                reboots; the port re-enumerates and the target comes back
                with Trg-RST (needs a second qspy attach)

Exit status is 0 when every procedure passed.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import socket
import struct
import subprocess
import sys
import threading
import time
from collections import deque
from pathlib import Path

# QSPY front-end protocol (see qutest.py in QTools 7.3.4)
QSPY_ATTACH = 128
QSPY_DETACH = 129
PKT_TEXT_ECHO = 0
PKT_TARGET_INFO = 64
PKT_ASSERTION = 69
PKT_ATTACH_CONF = 128
TO_TRG_INFO = 0
TO_TRG_COMMAND = 1
TO_TRG_RESET = 2
TO_TRG_GLB_FILTER = 10


class QSpyLink:
    """Owns a qspy subprocess and the UDP socket attached to it."""

    def __init__(self, qspy: str, port: str, baud: int, udp_port: int, workdir: Path, tag: str):
        self.qspy = qspy
        self.port = port
        self.baud = baud
        self.udp_port = udp_port
        self.workdir = workdir
        self.tag = tag
        self.proc: subprocess.Popen | None = None
        self.sock: socket.socket | None = None
        self.lines: deque[str] = deque()
        self.lock = threading.Lock()
        self.seq = 0
        self.attached = False
        self.have_assert = False
        self.target_info: dict | None = None
        self._stop = threading.Event()
        self._reader: threading.Thread | None = None

    # -- lifecycle ---------------------------------------------------------
    def start(self, timeout_s: float = 5.0) -> bool:
        self.workdir.mkdir(parents=True, exist_ok=True)
        log = open(self.workdir / f"qspy-{self.tag}-stdout.txt", "wb")
        self.proc = subprocess.Popen(
            [self.qspy, "-k", "-c", self.port, "-b", str(self.baud), "-u", str(self.udp_port), "-o", "-r", "c0"],
            cwd=str(self.workdir), stdout=log, stderr=subprocess.STDOUT,
        )
        time.sleep(0.5)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.settimeout(0.2)
        self._reader = threading.Thread(target=self._read_loop, daemon=True)
        self._reader.start()
        deadline = time.time() + timeout_s
        while time.time() < deadline and not self.attached:
            self.send(struct.pack("<BB", QSPY_ATTACH, 0x3))  # text + binary channels
            time.sleep(0.3)
        return self.attached

    def stop(self) -> None:
        self._stop.set()
        if self.sock is not None:
            try:
                self.send(struct.pack("<B", QSPY_DETACH))
            except OSError:
                pass
            time.sleep(0.2)
            self.sock.close()
        if self.proc is not None and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(3)
            except subprocess.TimeoutExpired:
                self.proc.kill()

    # -- transport ----------------------------------------------------------
    def send(self, packet: bytes) -> None:
        assert self.sock is not None
        self.sock.sendto(bytes([self.seq]) + packet, ("127.0.0.1", self.udp_port))
        self.seq = (self.seq + 1) & 0xFF

    def _read_loop(self) -> None:
        assert self.sock is not None
        while not self._stop.is_set():
            try:
                pkt = self.sock.recv(4096)
            except socket.timeout:
                continue
            except OSError:
                return
            if len(pkt) < 2:
                continue
            rec = pkt[1]
            if rec == PKT_ATTACH_CONF:
                self.attached = True
            elif rec == PKT_TEXT_ECHO:
                text = pkt[3:].decode("utf-8", "replace")
                if len(pkt) > 3 and pkt[2] == PKT_ASSERTION:
                    self.have_assert = True
                with self.lock:
                    self.lines.append(text)
            elif rec == PKT_TARGET_INFO and len(pkt) == 18:
                self.target_info = {"qp_version": pkt[3] + (pkt[4] << 8)}

    # -- helpers ------------------------------------------------------------
    def drain(self) -> list[str]:
        with self.lock:
            out = list(self.lines)
            self.lines.clear()
        return out

    def expect(self, pattern: str, timeout_s: float, log: list[str]) -> re.Match | None:
        """Wait until a received text line matches pattern; earlier lines are kept in log."""
        rx = re.compile(pattern)
        deadline = time.time() + timeout_s
        while time.time() < deadline:
            with self.lock:
                while self.lines:
                    line = self.lines.popleft()
                    log.append(line)
                    m = rx.search(line)
                    if m:
                        return m
            time.sleep(0.02)
        return None

    def command(self, cmd_id: int, p1: int = 0, p2: int = 0, p3: int = 0) -> None:
        self.send(struct.pack("<BBIII", TO_TRG_COMMAND, cmd_id, p1, p2, p3))


def wait_for_port(port: str, present: bool, timeout_s: float) -> bool:
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        if os.path.exists(port) == present:
            return True
        time.sleep(0.1)
    return False


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--port", default="/dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--qspy", default=".tools/quantum-leaps/qtools-7.3.4/bin/qspy")
    ap.add_argument("--udp-port", type=int, default=7701)
    ap.add_argument("--out", default="port_validation_results.json")
    ap.add_argument("--skip-assertion", action="store_true", help="do not run the reboot-inducing assertion test")
    ap.add_argument("--only-set-role", type=int, default=None, metavar="ROLE",
                    help="instead of the procedures: attach, select the GP25 role (0-3), hold the trace host "
                         "attached for --hold seconds (for an analyzer capture), restore role 1 and exit")
    ap.add_argument("--hold", type=float, default=10.0, help="seconds to hold with --only-set-role")
    args = ap.parse_args(argv)

    if args.only_set_role is not None:
        out = Path(args.out)
        link = QSpyLink(args.qspy, args.port, args.baud, args.udp_port,
                        out.parent if out.parent != Path("") else Path("."), f"role{args.only_set_role}")
        if not link.start():
            print("could not attach to qspy", file=sys.stderr)
            link.stop()
            return 1
        time.sleep(0.5)
        link.drain()
        link.command(5, args.only_set_role)
        m = link.expect(r"PROBE_ROLE\s+(\d+)", 3.0, [])
        print(f"role set: {m.group(1) if m else 'no echo'}", flush=True)
        time.sleep(args.hold)
        link.command(5, 1)
        link.expect(r"PROBE_ROLE\s+1", 3.0, [])
        link.stop()
        return 0 if m else 1

    out = Path(args.out)
    workdir = out.parent if out.parent != Path("") else Path(".")
    results: dict[str, dict] = {}
    transcript: list[str] = []

    def record(name: str, ok: bool, detail: str) -> None:
        results[name] = {"pass": bool(ok), "detail": detail}
        print(f"[{'PASS' if ok else 'FAIL'}] {name}: {detail}", flush=True)

    link = QSpyLink(args.qspy, args.port, args.baud, args.udp_port, workdir, "session1")
    if not link.start():
        record("attach", False, "could not attach to qspy over UDP")
        link.stop()
        out.write_text(json.dumps({"results": results}, indent=2))
        return 1
    record("attach", True, f"attached to qspy on UDP {args.udp_port}")
    time.sleep(1.0)
    link.drain()

    # info: QS-RX round trip
    link.send(struct.pack("<B", TO_TRG_INFO))
    m = link.expect(r"Trg-Info\s+QP-Ver=(\d+)", 3.0, transcript)
    record("info", m is not None, f"Trg-Info QP-Ver={m.group(1)}" if m else "no Trg-Info within 3 s")

    # ticking: counters advance between two reports one second apart
    def counters() -> tuple[int, int, int] | None:
        link.command(6)
        mm = link.expect(r"VALIDATION_STAT\s+(\d+)\s+(\d+)\s+(\d+)", 3.0, transcript)
        return tuple(int(x) for x in mm.groups()) if mm else None

    c1 = counters()
    time.sleep(1.0)
    c2 = counters()
    if c1 and c2:
        dt = c2[0] - c1[0]
        ok = 90 <= dt <= 115 and c2[1] - c1[1] >= dt - 2 and c2[2] - c1[2] >= dt - 2
        record("ticking", ok, f"ticks {c1[0]}->{c2[0]} (+{dt} in ~1 s), events +{c2[1]-c1[1]}, dynamic +{c2[2]-c1[2]}")
    else:
        record("ticking", False, "no VALIDATION_STAT reply to command 6")

    # publish: command 1 publishes VALIDATE_SIG from the command source
    link.drain()
    link.command(1)
    m = link.expect(r"Disp===>\s+Obj=Validator::inst,Sig=VALIDATE_SIG", 3.0, transcript)
    record("publish", m is not None, "VALIDATE_SIG published by command 1 was dispatched to Validator" if m else "no dispatch of VALIDATE_SIG seen")

    # dynamic: command 2 posts a pool-allocated DYN_SIG with seq=param1
    link.drain()
    link.command(2, 1)
    got_new = link.expect(r"(QF-New\s+Sig=DYN_SIG|MP-Get\s+Obj=)", 3.0, transcript) is not None
    got_disp = link.expect(r"Disp===>\s+Obj=Validator::inst,Sig=DYN_SIG", 3.0, transcript) is not None
    got_gc = link.expect(r"(QF-gc\s+Evt<Sig=DYN_SIG|MP-Put\s+Obj=)", 3.0, transcript) is not None
    record("dynamic", got_new and got_disp and got_gc,
           f"pool get/QF-New={got_new} dispatch={got_disp} pool put/QF-gc={got_gc} for a host-posted DYN_SIG")

    # roles: switch the second probe through all roles and back to dispatch
    roles_ok = True
    for role in (0, 2, 3, 1):
        link.drain()
        link.command(5, role)
        mm = link.expect(r"PROBE_ROLE\s+(\d+)", 3.0, transcript)
        roles_ok = roles_ok and mm is not None and int(mm.group(1)) == role
    record("roles", roles_ok, "PROBE_ROLE echoed 0,2,3,1" if roles_ok else "role echo missing or wrong")

    # reset-count: command 7 then 6
    link.drain()
    link.command(7)
    time.sleep(0.05)
    c3 = counters()
    record("reset-count", c3 is not None and c3[0] < 20, f"counters after reset: {c3}")

    if args.skip_assertion:
        record("assertion", True, "skipped by --skip-assertion")
    else:
        # assertion: command 4 -> Q_ERROR_ID(400); target records it and reboots
        link.drain()
        link.have_assert = False
        link.command(4)
        m = link.expect(r"=ASSERT=\s+Mod=(\S+),Loc=(\d+)", 5.0, transcript)
        seen = m is not None
        detail = f"=ASSERT= Mod={m.group(1)} Loc={m.group(2)}" if m else "no =ASSERT= record within 5 s"
        gone = wait_for_port(args.port, False, 8.0)
        back = wait_for_port(args.port, True, 8.0) if gone else False
        link.stop()
        link2 = None
        rst = False
        if back:
            time.sleep(0.3)
            link2 = QSpyLink(args.qspy, args.port, args.baud, args.udp_port, workdir, "session2")
            if link2.start():
                rst = link2.expect(r"Trg-RST\s+QP-Ver", 8.0, transcript) is not None
        record("assertion", seen and gone and back and rst,
               f"{detail}; port disappeared={gone}; came back={back}; Trg-RST after reboot={rst}")
        if link2 is not None:
            link2.stop()
        link = None  # type: ignore

    if link is not None:
        link.stop()
    all_ok = all(r["pass"] for r in results.values())
    out.write_text(json.dumps({"port": args.port, "results": results, "transcript_tail": transcript[-200:]}, indent=2))
    print(f"\n{'ALL PASSED' if all_ok else 'FAILURES'}: {sum(r['pass'] for r in results.values())}/{len(results)} -> {out}")
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
