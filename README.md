# SCC

Simplified C Compiler

```
        USAGE:
          scc [input] [-o output] [ options ]
        OPTIONS:
          -o,--output <path>          executable output path [default: a.out]
          -O,--asm-output <path>      asm output path [default: a.S]
          -a,--assembler <path>       assembler path [default: mipsel-linux-musl-cc]
          -e,--execute                execute after compile
          -X,--emulator <path>        set emulator path [default: qemu-mipsel]
          -h,--help                   print this help message and exit
```
