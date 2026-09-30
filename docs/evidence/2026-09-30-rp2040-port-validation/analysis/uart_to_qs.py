#!/usr/bin/env python3
"""Turn sigrok's UART decoder output (-P uart:...:format=hex -A uart=rx-data)
into a raw binary QS stream that `qspy -f` can decode offline.

Lines look like `uart-1: 7E` or `uart-1: 7E 01 ...`; everything after the
colon is hex bytes."""
import re
import sys

src, dst = sys.argv[1], sys.argv[2]
data = bytearray()
for line in open(src, errors="replace"):
    if ":" not in line:
        continue
    for tok in re.findall(r"\b[0-9A-Fa-f]{2}\b", line.split(":", 1)[1]):
        data.append(int(tok, 16))
open(dst, "wb").write(data)
print(f"{len(data)} bytes -> {dst}")
