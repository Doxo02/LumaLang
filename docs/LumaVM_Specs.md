# Specifications for the LumaVM

## VM model
- Word size: 32-bit signed integer
- Registers: ```R0..R7``` (8 x 32-bit)
- Stack: 256 words, ```sp``` is 8-bit unsigned (wrap-around)
- Memory: 256 words for globals / persistent data
- PC: 16-bit program counter
- Extensions: dynamic registration table of up to 256 handlers (index = ExtID)

## Opcode ranges
- ```0x00-0x1F```: Core data/math/logic opcodes
- ```0x20-0x2F```: Comparisons
- ```0x30-0x3F```: Control flow
- ```0xD0-0xDF```: Built-in extension opcodes
- ```0xE0```: EXT dynamic extension prefix (```E0 [ExtID][SubOp][args...]```)
- ```0xE1-0xEF```: (reserved) for future / optional built-in extensions
- ```0xF0-0xFF```: System / reserved (HALT, DEBUG, META)

```E0``` gives up to 256 extension IDs x 256 subops

## Core instruction set
All multi-register instructions use a single operand byte: high nibble = ```dst``` (0-7), low nibble = ```src``` (0-7).

### Data movement/stack

| Opcode           | Hex        | Encoding                | Semantics                   |
| :--------------- | :--------- | :---------------------- | :-------------------------- |
| NOOP             | ```0x00``` | N/A                     | no-op                       |
| MOVI Rdst, imm   | ```0x01``` | ```[01][Rdst][imm32]``` | ```Rdst = imm32```          |
| MOV Rdst, Rsrc   | ```0x02``` | ```[02][dstsrc]```      | ```Rdst = Rsrc```           |
| LOAD Rdst, addr  | ```0x03``` | ```[03][Rdst][addr8]``` | ```Rdst = mem[addr8]```     |
| STORE addr, Rsrc | ```0x04``` | ```[04][addr8][Rsrc]``` | ```mem[addr8] = Rsrc```     |
| PUSH Rsrc        | ```0x05``` | ```[05][Rsrc]```        | push ```Rsrc```             |
| POP Rdst         | ```0x06``` | ```[06][Rdst]```        | pop → ```Rdst```            |
| LDC Rdst, idx    | ```0x07``` | ```[07][Rdst][idx8]```  | ```Rdst = constant[idx8]``` |

### Arithmetic

| Opcode         | Hex        | Encoding           | Semantics                    |
| :------------- | :--------- | :----------------- | :--------------------------- |
| ADD Rdst, Rsrc | ```0x10``` | ```[10][dstsrc]``` | ```Rdst += Rsrc```           |
| SUB Rdst, Rsrc | ```0x11``` | ```[11][dstsrc]``` | ```Rdst -= Rsrc```           |
| MUL Rdst, Rsrc | ```0x12``` | ```[12][dstsrc]``` | ```Rdst *= Rsrc```           |
| DIV Rdst, Rsrc | ```0x13``` | ```[13][dstsrc]``` | ```Rdst /= Rsrc```           |
| MOD Rdst, Rsrc | ```0x14``` | ```[14][dstsrc]``` | ```Rdst %= Rsrc```           |
| ABS Rdst       | ```0x15``` | ```[15][Rdst]```   | ```Rdst = abs(Rdst)```       |
| MAX Rdst, Rsrc | ```0x16``` | ```[16][dstsrc]``` | ```Rdst = max(Rdst, Rsrc)``` |
| MIN Rdst, Rsrc | ```0x17``` | ```[17][dstsrc]``` | ```Rdst = min(Rdst, Rsrc)``` |
| AND Rdst, Rsrc | ```0x18``` | ```[18][dstsrc]``` | ```Rdst &= Rsrc```           |
| OR Rdst, Rsrc  | ```0x19``` | ```[18][dstsrc]``` | ```Rdst \|= Rsrc```          |
| XOR Rdst, Rsrc | ```0x1A``` | ```[18][dstsrc]``` | ```Rdst ^= Rsrc```           |
| NOT Rdst       | ```0x1B``` | ```[18][Rdst]```   | ```Rdst = !Rdst```           |

### Comparisons

All comparison results produce 1 for true and 0 for false in ```Rdst```

| Opcode         | Hex        | Encoding           | Semantics                   |
| :------------- | :--------- | :----------------- | :-------------------------- |
| EQ Rdst, Rsrc  | ```0x20``` | ```[20][dstsrc]``` | ```Rdst = (Rdst == Rsrc)``` |
| NEQ Rdst, Rsrc | ```0x21``` | ```[21][dstsrc]``` | ```Rdst = (Rdst != Rsrc)``` |
| GEQ Rdst, Rsrc | ```0x22``` | ```[22][dstsrc]``` | ```Rdst = (Rdst >= Rsrc)``` |
| LEQ Rdst, Rsrc | ```0x23``` | ```[23][dstsrc]``` | ```Rdst = (Rdst <= Rsrc)``` |
| GT Rdst, Rsrc  | ```0x24``` | ```[24][dstsrc]``` | ```Rdst = (Rdst > Rsrc)```  |
| LT Rdst, Rsrc  | ```0x25``` | ```[25][dstsrc]``` | ```Rdst = (Rdst < Rsrc)```  |

### Control flow

| Opcode    | Hex        | Encoding                 | Semantics                            |
| :-------- | :--------- | :----------------------- | :----------------------------------- |
| JMPA abs  | ```0x30``` | ```[30][abs16]```        | ```pc = abs16```                     |
| JMPR rel  | ```0x31``` | ```[31][rel8]```         | ```pc += rel8```                     |
| JZA abs   | ```0x32``` | ```[32][Rcond][abs16]``` | if ```Rcond == 0``` ```pc = abs16``` |
| JZR rel   | ```0x33``` | ```[33][Rcond][rel8]```  | if ```Rcond == 0``` ```pc += rel8``` |
| JNZA abs  | ```0x34``` | ```[34][Rcond][abs16]``` | if ```Rcond != 0``` ```pc = abs16``` |
| JNZR rel  | ```0x35``` | ```[35][Rcond][rel8]```  | if ```Rcond != 0``` ```pc += rel8``` |
| CALLA abs | ```0x36``` | ```[36][abs16]```        | push return pc, ```pc = abs16```     |
| CALLR rel | ```0x37``` | ```[37][rel8]```         | push return pc, ```pc += rel8```     |
| RET       | ```0x38``` | ```[38]```               | pop return pc                        |

### System

| Opcode         | Hex        | Encoding   | Semantics                            |
| :------------- | :--------- | :--------- | :----------------------------------- |
| HALT           | ```0xFF``` | ```[FF]``` | stop execution                       |
| DBG (reserved) | ```0xFE``` | var        | debug/trap (implementation specific) |

## Extension system

### Standard Extension ID registry

|         ID | Name     | Use                                       |
| ---------: | :------- | :---------------------------------------- |
| ```0x01``` | NEOPIXEL | addressable LED strip (NeoPixel / WS281x) |
| ```0x02``` | MIC      | microphone / audio input                  |

### Extension calls
```
0xE0 [ExtID:1] [SubOp:1] [args...]
```
VM fetches ExtID and SubOp then delegates to ```ext_dispatch(vm, ExtID, SubOp)``` which consumes any further args.

#### Built-in opcodes for common extensions

| Opcode    | Hex        | Encoding         | Semantics                                      |
| :-------- | :--------- | :--------------- | :--------------------------------------------- |
| SRGB      | ```0xD0``` | ```[D0]```       | set pixel at ```R0``` to ```RGB(R1, R2, R3)``` |
| FRGB      | ```0xD1``` | ```[D1]```       | fill all with ```RGB(R0, R1, R2)```            |
| SHOW      | ```0xD2``` | ```[D2]```       | flush LED buffer to strip                      |
| CLR       | ```0xD3``` | ```[D3]```       | clear LED buffer to 0                          |
| NLED Rdst | ```0xD4``` | ```[D4][Rdst]``` | ```Rdst = configured LED count```              |

