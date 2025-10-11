# LumaLang
A programming language and virtual machine built for dynamic LED animations on embedded systems.

## Overview
__LumaLang__ is a lightweight language designed for programmable LED effects on embedded systems. It compiles to __Luma Bytecode (LBC)__, executed by the __LumaVM__, a small and efficient virtual machine built for microcontrollers.

The project divides into three main specifications:
- __Language__: syntax and semantics for animations
- __Virtual Machine__: bytecode execution model and core instruction set
- __File Format__: binary structure for compiled programs (```.lbc``` files)

## Design Goals
- __Lightweight__: Minimal memory footprint and simple execution model
- __Extensible__: Modular extension system for hardware features (e.g. LEDs, audio, ...)
- __Portable__: Same bytecode runs across MCUs

## Documentation
Further documentation can be found in the _docs_ folder of this repository.

## Status
:warning: This project is __under active development__. <br>
Expect frequent updates to the language syntax, VM instruction set and file format during early development.

---

<p align="center">
  <sub>Made with ❤️ for embedded lighting systems.</sub>
</p>