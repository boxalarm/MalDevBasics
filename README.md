# MalDevBasics

Foundational building blocks for malware development and Windows internals,
written while working through various courses, including:

- [Malware Analysis and Development](https://trainsec.net/courses/malware-analysis-and-development/)
- [Windows Internals Master](https://trainsec.net/windows-internals-master) (specifically **x64 Architecture and Programming**)
- [MalDev Academy](https://maldevacademy.com)

These are educational implementations focused on understanding the underlying
mechanics of malware dev primitives and Windows internals.

## ⚠️ Disclaimer

This code is for **educational purposes only**. It exists to demonstrate malware development
techniques in a learning context only. Do not use against systems you
do not own or have explicit authorization to test.

## Companion Articles

I write about some of these techniques on Medium:

- [MalDev 101: Writing Your First Shellcode Runner in C](https://medium.com/@boxalarm/maldev-101-writing-your-first-shellcode-runner-in-c-3bc861169796)
- More to come!

## Building

All code targets Windows x64 and is built with the MSVC toolchain.
Visual Studio 2019+ or VS Build Tools required.

From a Developer Command Prompt:

```bash
cl.exe BasicShellcodeRunner.c
```

For projects with multiple source files or specific linker requirements,
see the comments or per-folder README.
