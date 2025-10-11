# File Layout (binary ```.lbc```)
```
+-------------------+
| Header (fixed)    |   (16 bytes)
+-------------------+
| Extension table   |   (variable)
+-------------------+
| Constant pool     |   (optional variable)
+-------------------+
| Code section      |   (variable)
+-------------------+
```

## Header (16 bytes)

| Offset | Size | Field      | Description                                          |
| :----- | :--- | :--------- | :--------------------------------------------------- |
| 0x00   | 4    | Magic      | ASCII ```LVM1```                                     |
| 0x04   | 1    | Version    | Bytecode format version (0x01)                       |
| 0x05   | 1    | Flags      | bitfield (see below)                                 |
| 0x06   | 1    | ExtCount   | number of extension records                          |
| 0x07   | 1    | ConstCount | number of constant entries                           |
| 0x08   | 2    | CodeOffset | offset from file start to code section (uint16)      |
| 0x0A   | 2    | EntryPoint | offset into code section to start executing (uint16) |
| 0x0C   | 4    | Reserved   | zero for v1.0                                        |

### Flags (byte at 0x05)
| Bit  | Field | Description |
| :--- | :---- | :---------- |
| 0    |       |             |
| 1    |       |             |
| 2    |       |             |
| 3    |       |             |
| 4    |       |             |
| 5    |       |             |
| 6    |       |             |
| 7    |       |             |

## Extension table
```ExtCount``` entries, each:
```
[ExtID:1][Flags:1][ConfigLen:1][ConfigData:ConfigLen]
```
- ```ExtID``` = 8-bit extension identifier (0x01 - 0xFF)
- ```Flags``` = extension flags (see below)
- ```ConfigLen``` = length of ```ConfigData``` in bytes
- ```ConfigData``` = extension-specific bytes

### Flags
| Bit  | Field | Description |
| :--- | :---- | :---------- |
| 0    |       |             |
| 1    |       |             |
| 2    |       |             |
| 3    |       |             |
| 4    |       |             |
| 5    |       |             |
| 6    |       |             |
| 7    |       |             |

## Constant pool
```ConstCount``` entries, each 4 bytes (int32). Access via ```LDC Rdst, idx``` (see instructions).

## Code section
Starts at ```CodeOffset```. ```EntryPoint``` is offset into this code section.