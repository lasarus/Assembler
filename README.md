# Assembler
This assembler is not currently meant for general use.
It supports only the instructions and features emitted (and used) in my [C compiler](https://github.com/lasarus/C-Compiler).
It is also able to assemble the self-compiled assembly output from that compiler.

## Build instructions

    make
## Usage

    as INPUT.s OUTPUT.o

The output is a 64-bit ELF object file.
