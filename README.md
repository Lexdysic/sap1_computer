# SAP1 Breadboard Computer
The "Simple as Possible" design of computer.

![Current Progress](docs/images/2017_07_20.jpg)
![Block Diagram](docs/images/current.jpg)

## Features
* 8-bit shared data/address bus.
* 256 bytes memory.
* Variable rate clock, between 1hz and 10khz, with manual step mode.
* Supports adding *and* subtracting!
* Display in signed or unsigned mode.

## Future Work
* More ALU operations including OR, AND, XOR
* Conditional Branching
* Storage module

## Example

**add.asm**
```asm
    mov a, #14
    add a, #28
    out a
    hlt
```

**fibonacci.asm**
```asm
    mov a, #0
    mov @99, a
    mov a, #1
    mov @100, a
loop:
    mov a, @99
    add a, @100
    mov @100, a
    mov @99, b
    out a
    jmp @loop
```

## Instructions

### Notation

`[label:] <verb> [args...]`

* `A` - Refers to the A-register.
* `B` - Refers to the B-register.
* `#lit` - A literal value given by the number `lit`, can be between 0 and 255.
* `@loc` - Refers to a location- literal address or value at a labeled location.
  * When `loc` is a number, this is considered an address in memory. Values between 0 and 255 are accepted as RAM is only 8-bit addressable.
  * When `loc` is an identifier, this is either the name of a register or the name of a label. When the name of a register is used, the location is the memory address given by the value stored in the register. When a label is given, the location is computed by the assembler then inserted in place of the label.

### Utility

| Instruction | Cycle Count | Description |
|---|---|---|
| `nop` | 2-cylces | No-operation will be executed for a single. |
| `hlt` | 3-cycles | Stop the cpu clock. No further operations will be run until reset button is pressed. |

### Output

`out <source>` - Sets the displayed value to be that of `<source>`.

| Instruction | Cycle Count | Description |
|---|---|---|
| `out @47` | 5-cylces | Output the value at literal memory address. |
| `out #23` | 4-cylces | Output a literal value. |
| `out A` | 3-cycles | Output the value in the A-register. |
| `out B` | 3-cycles | Output the value in the B-register. |

### Move

`mov <target>, <source>` - Move a word value from one location to another. This instruction handles moving into and out of registers from memory, between registers, and setting literal values into the destination location.

| Instruction | Cycle Count | Description |
|---|---|---|
| `mov A, @47` | 4-cycles | Move the value at a literal memory address into the A-register. |
| `mov A, #23` | 4-cylces | Move a literal value into the A-register. |
| `mov A, B` | 3-cylces | Move the value in the B-register into the A-register. |
| `mov B, @47` | 4-cycles | Move the value at a literal memory address into the B-register. |
| `mov B, #23` | 4-cycles | Move a literal value into the B-register. |
| `mov B, A` | 3-cycles | Move the value in the A-register into the B-register. |
| `mov @47, A` | 5-cycles | Move the value in the A-register to the memory location given by a literal. |
| `mov @47, B` | 5-cycles | Move the value in the B-register to the memory location given by a literal. |
| `mov @47, #23` | Unimplemented | Move a literal value into the memory location given by a literal. |

### Addition

`add <target>, <source>` - Perform addition such that `A = A + <source>`


| Instruction | Cycle Count | Description |
|---|---|---|
| `add A, @47` | 6-cycles | Add the value at a literal memory location to the value stored in the A-register, the result is stored back in the A-register. |
| `add A, #23` | 5-cycles | Add a literal value to the value stored in the A-register, the result is stored back in the A-register. |

### Subtraction

`sub <target>, <source>` - Perform subtraction such that `A = A - <source>`.

| Instruction | Cycle Count | Description |
|---|---|---|
| `sub A, @47` | 6-cycles | Subtract the value at a literal memory location from the value stored in the A-register, the result is stored back in the A-register. |
| `sub A, #23` | 5-cycles | Subtract a literal value from the value stored in the A-register, the result is stored back in the A-register. |

### Jump

`jmp <target>` - Unconditionally update the Program Counter so the next cycle will read the instruction at `<target>` location in memory.

| Instruction | Cycle Count | Description |
|---|---|---|
| `jmp #23` | 4-cycles | Set a literal memory location as the next location to be executed. |
| `jmp @lbl` | 4-cycles | Set the next location to be executed to the memory location at a label. |
