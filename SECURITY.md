# Security Policy

## Supported Versions

Security fixes for **QP/C++ for Arduino** are provided in new releases. Please make sure
that you can reproduce the problem with the latest release before
reporting it.

| Version          | Supported          |
| ---------------- | ------------------ |
| latest release   | :white_check_mark: |
| older releases   | :x: (please upgrade) |

Commercial licensees receive security fixes as specified in their license
and support agreement.

QP-Arduino packages the [QP/C++](https://github.com/QuantumLeaps/qpcpp)
framework for ARM-based Arduino boards. Vulnerabilities in the framework
itself are fixed in QP/C++ first and then in this package.

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub
issues, discussions, or pull requests.**

Instead, report them privately in one of these ways:

- email **security@state-machine.com**, or
- use GitHub's **"Report a vulnerability"** button on this repository's
  **Security** tab.

Please include the QP/C++ for Arduino version affected, the target (CPU, compiler, port,
or host OS) if relevant, a description of the problem and its impact, and
the steps to reproduce it (or a proof of concept).

## What to Expect

- acknowledgement of your report within **5 business days**
- confirmation (or not) of the vulnerability, usually within **30 days**
- a fixed release and a GitHub security advisory, crediting you if you wish

Please give us reasonable time to fix the problem before disclosing it
publicly. Our full vulnerability disclosure policy is at
**https://www.state-machine.com/security/**
