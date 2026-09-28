# 8kto - Instruction Set Architecture (ISA) Specification

## Specifications

- **Harvard** memory model (independent program and data addressing)
- **8-bit** data word and registers
- **8-bit** fixed-width instruction encoding
- **Sequential** execution model
- **Unconditional**, **conditional**, and **relative** branching

## Data Representation & Signed Arithmetic

All general-purpose registers (R0-R3) and RAM memory addresses hold 8-bit values represented in **two's complement** format.

- **Word Size:** 8 bits (1 byte).
- **Representable Range:** `-128` (`0x80`) to `+127` (`0x7F`).
- **Zero Representation:** `0x00` represents strictly positive/neutral zero (`R0` is permanently wired to `0x00`).
- **Overflow / Wrap-around:** Arithmetic operations (`ADD`, `SUB`, `INC`, `DEC`) follow standard modular arithmetic ($2^8$). Carry and overflow flags are not architecturally exposed; operations wrap around without trapping.
- **Immediate / Relative Offsets:** 4-bit offset fields used in relative control-flow instructions are sign-extended to 8 bits, yielding an effective branch range of `[-8, +7]` instructions.

## Registers

- `r0` -> Constant **0** (Hardwired zero, write-ignored)
- `r1` -> General-purpose register
- `r2` -> General-purpose register
- `r3` / `pc` -> Program Counter (holds address of the next instruction, **only modifiable with jump instructions**)

## Instruction Encoding Formats

> Unused register fields marked with `X` are ignored by the hardware decoder. The assembler defaults to emitting `00`.

All instructions are 8 bits wide, starting with a 4-bit opcode:

### Arithmetic-Logic

| Opcode [7:4] | Rs [3:2] | Rd [1:0] |
| :---: | :---: | :---: |
| 0XXX | `Rs` | `Rd` |

### Memory Operations

| Opcode [7:4] | Rs [3:2] | Rd [1:0] |
| :---: | :---: | :---: |
| 100X | `Rs` | `Rd` |

In `ldm`:

- `Rs` contains the memory address from where to load the data
- `Rd` is the register where the data will be stored

In `stm`:

- `Rs` is the register containing the data that will be stored in memory
- `Rd` contains the memory address where the data will be stored

### Control Operations (Indirect / Conditional)

| Opcode [7:4] | Rs / Padding [3:2] | Rd [1:0] |
| :---: | :---: | :---: |
| 1010 - 1100 | `Rs` / Padding | `Rd` |

- Jump target address is stored in `Rd`
- For unconditional jumps (`jmp`), `Rs` is ignored / padding

### Relative Jump Operation (`jre`)

| Opcode [7:4] | Offset [3:0] |
| :---: | :---: |
| 1101 | 4-bit signed immediate (-8 to +7) |

### Halt Operation (`hlt`)

| Opcode [7:4] | Padding [3:0] |
| :---: | :---: |
| 1111 | XXXX (ignored) |

---

## Instruction Set Architecture (ISA)

### ALU Instructions

| Opcode | Type | Mnemonic | Operands | Operation | Description |
| :---: | :---: | :--- | :--- | :--- | :--- |
| `0000` | ALU | `add` | `Rs, Rd` | `Rd <= Rd + Rs` | Addition |
| `0001` | ALU | `sub` | `Rs, Rd` | `Rd <= Rd - Rs` | Subtraction |
| `0010` | ALU | `and` | `Rs, Rd` | `Rd <= Rd & Rs` | Bitwise AND |
| `0011` | ALU | `orr` | `Rs, Rd` | `Rd <= Rd \| Rs` | Bitwise OR |
| `0100` | ALU | `xor` | `Rs, Rd` | `Rd <= Rd ^ Rs` | Bitwise XOR |
| `0101` | ALU | `not` | `Rs, Rd` | `Rd <= ~Rs` | Bitwise NOT |
| `0110` | ALU | `inc` | `Rs, Rd` | `Rd <= Rs + 1` | Increment source into destination |
| `0111` | ALU | `dec` | `Rs, Rd` | `Rd <= Rs - 1` | Decrement source into destination |

### Memory Instructions

| Opcode | Type | Mnemonic | Operands | Operation | Description |
| :---: | :---: | :--- | :--- | :--- | :--- |
| `1000` | Mem | `ldm` | `[Rs], Rd` | `Rd <= MEM[Rs]` | Load from data memory |
| `1001` | Mem | `stm` | `Rs, [Rd]` | `MEM[Rd] <= Rs` | Store to data memory |

### Control Instructions

| Opcode | Type | Mnemonic | Operands | Operation | Description |
| :---: | :---: | :--- | :--- | :--- | :--- |
| `1010` | Jump | `jmp` | `Rd` | `PC <= Rd` | Indirect unconditional jump |
| `1011` | Jump | `jez` | `Rs, Rd` | `if (Rs == 0) PC <= Rd` | Jump if source equals zero |
| `1100` | Jump | `jnz` | `Rs, Rd` | `if (Rs != 0) PC <= Rd` | Jump if source is not zero |
| `1101` | Jump | `jre` | `Offset(4b)` | `PC <= PC_current + sign_extend(Offset)` | Signed relative branch |
| `1110` | Free | *res* | — | — | Reserved |
| `1111` | Ctrl | `hlt` | — | Halt | Halts CPU execution |

---

## Pseudo-instructions & Idioms

| Mnemonic | Equivalent Instruction(s) | Effect | Description |
| :--- | :--- | :--- | :--- |
| `nop` | `add r0, r0` | — | No operation |
| `clr Rd` | `xor Rd, Rd` | `Rd <= 0` | Clears register to zero |
| `mov Rs, Rd` | `xor Rd, Rd`<br>`add Rs, Rd` | `Rd <= Rs` | Copies value from `Rs` into `Rd` |

---

## Assembly Syntax Conventions

When targeting the reference assembler (`customasm`), register identifiers are prefixed with `$` to distinguish architectural registers from user symbols and labels:

| ISA Register | Assembly Syntax | Aliases |
| :---: | :---: | :---: |
| `r0` | `$r0` | `$0` |
| `r1` | `$r1` | — |
| `r2` | `$r2` | — |
| `r3` | `$r3` | `$pc` |

Example:

```asm
    clr $r1
    inc $r1,$r1
    mov $r1,$r2
```
