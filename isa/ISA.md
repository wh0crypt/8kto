# 8kto - Instruction Set Architecture (ISA) Specification

## Specifications

- **Harvard** memory model (independent program and data addressing)
- **8-bit** data word and registers
- **8-bit** fixed-width instruction encoding
- **Sequential** execution model
- **Unconditional**, **conditional**, and **relative** branching

## Data Representation & Signed Arithmetic

All general-purpose registers (`$r0`-`$r3`) and RAM memory addresses hold 8-bit values represented in **two's complement** format.

- **Word Size:** 8 bits (1 byte).
- **Representable Range:** `-128` (`0x80`) to `+127` (`0x7F`).
- **Zero Representation:** `0x00` represents strictly positive/neutral zero (`R0` is permanently wired to `0x00`).
- **Overflow / Wrap-around:** Arithmetic operations (`add`, `sub`) follow standard modular arithmetic ($2^8$). Carry and overflow flags are not architecturally exposed; operations wrap around without trapping.
- **Immediate / Relative Offsets:**
  - 2-bit signed immediate (`imm2`) used in `adi` is sign-extended to 8 bits (`[-2, +1]`).
  - 4-bit signed immediate (`off`) used in `jre` is sign-extended to 8 bits, yielding a relative jump range of `[-8, +7]` instructions.

## Registers

- `r0` -> Hardwired constant **0** (write-ignored).
- `r1` -> General-purpose register.
- `r2` -> General-purpose register.
- `r3` -> General-purpose / Dedicated Accumulator for immediate loading (`lui`/`lli`) and target address staging.

The Program Counter (`pc`) is an internal hardware register modified sequentially or via control instructions (`jmp`, `jez`, `jnz`, `jre`).

## Instruction Encoding Formats

All instructions are exactly 8 bits wide, beginning with a 4-bit opcode (`[7:4]`):

### Standard Register ALU & Memory Operations

| Opcode [7:4] | Rs [3:2] | Rd [1:0] | Instructions |
| :---: | :---: | :---: | :---: |
| 0000-0101 | Rs | Rd | `add`, `sub`, `and`, `orr`, `xor`, `not` |
| 1001-1010 | Rs | Rd | `ldm`, `stm` |
| 1100-1101 | Rs | Rd | `jez`, `jnz` |

- In `ldm`: `Rs` contains the memory address; `Rd` is the destination register.
- In `stm`: `Rs` contains the data; `Rd` contains the destination memory address.
- In `jez` / `jnz`: `Rs` is the condition register; `Rd` contains the jump target address.

### Add Immediate (`adi`)

| Opcode [7:4] | Imm [3:2] | Rd [1:0] |
| :---: | :---: | :---: |
| 0110 | 2-bit signed immediate (`s2`) | Rd |

- Operates in-place: `Rd <= Rd + sign_extend(Imm)`.

### Immediate Nibble Loads (`lui`, `lli`)

| Opcode [7:4] | Imm [3:0] |
| :---: | :---: |
| 0111 (`lui`) - 1000 (`lli`) | 4-bit unsigned immediate (`u4`) |

- The destination register is implicitly `$r3`.

### Indirect Jump (`jmp`)

| Opcode [7:4] | Padding [3:2] | Rd [1:0] |
| :---: | :---: | :---: |
| 1011 | 00 | Rd |

### Relative Jump (`jre`)

| Opcode [7:4] | Offset [3:0] |
| :---: | :---: |
| 1110 | 4-bit signed relative offset |

### Halt (`hlt`)

| Opcode [7:4] | Padding [3:0] |
| :---: | :---: |
| 1111 | 0000 |

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
| `0110` | ALU | `adi` | `Imm, Rd` | `Rs <= Imm + Rd` | Add an immediate into destination |
| `0111` | ALU | `lui` | `Imm` | `R3 <= Imm << 4` | Load upper nibble into `$r3`, clearing lower nibble |
| `1000` | ALU | `lli` | `Imm` | `R3 <= (R3 & 0xF0) \| Imm` | Load lower nibble into `$r3`, preserving upper nibble |

### Memory Instructions

| Opcode | Type | Mnemonic | Operands | Operation | Description |
| :---: | :---: | :--- | :--- | :--- | :--- |
| `1001` | Mem | `ldm` | `[Rs], Rd` | `Rd <= MEM[Rs]` | Load from data memory |
| `1010` | Mem | `stm` | `Rs, [Rd]` | `MEM[Rd] <= Rs` | Store to data memory |

### Control Instructions

| Opcode | Type | Mnemonic | Operands | Operation | Description |
| :---: | :---: | :--- | :--- | :--- | :--- |
| `1011` | Jump | `jmp` | `Rd` | `PC <= Rd` | Indirect unconditional jump |
| `1100` | Jump | `jez` | `Rs, Rd` | `if (Rs == 0) PC <= Rd` | Jump if source equals zero |
| `1101` | Jump | `jnz` | `Rs, Rd` | `if (Rs != 0) PC <= Rd` | Jump if source is not zero |
| `1110` | Jump | `jre` | `Offset(4b)` | `PC <= PC_current + sign_extend(Offset)` | Signed relative branch |
| `1111` | Ctrl | `hlt` | — | Halt | Halts CPU execution |

---

## Pseudo-instructions & Idioms

| Mnemonic | Expansions | ROM Bytes | Description |
| :--- | :--- | :--- | :--- |
| `nop` | `add r0, r0` | 1 | No operation |
| `clr Rd` | `xor Rd, Rd` | 1 | Clears register to zero |
| `mov Rs, Rd` | `xor Rd, Rd`<br>`add Rs, Rd` | 2 | Copy `Rs` into `Rd` |
| `inc Rd` | `adi 1, Rd` | 1 | Increment `Rd` by 1 |
| `dec Rd` | `adi -11, Rd` | 1 | Decrement `Rd` by 1 |
| `ldi $r3, Imm8` | `lui (Imm8 >> 4)`<br>`lli (Imm8 & 0x0F)` | 2 | Synthesize arbitrary 8-bit immediate into `$r3` |
| `jla Target` | `ldi $r3, Target`<br>`jmp $r3` | 3 | Jump to absolute 8-bit label |
---

## Assembly Syntax Conventions

Register identifiers are prefixed with `$`:

| ISA Register | Assembly Syntax | Architectural Role |
| :---: | :---: | :---: |
| `r0` | `$r0` | Constant zero |
| `r1` | `$r1` | General purpose |
| `r2` | `$r2` | General purpose |
| `r3` | `$r3` | General purpose / Accumulator for `lui`, `lli`, `ldi`, `jla` |

Example:

```asm
start:
    clr $r1              ; $r1 <= 0
    inc $r1              ; $r1 <= 1 (via adi 1, $r1)
    mov $r1, $r2         ; $r2 <= 1
    jla target           ; Fixed 3-byte expansion (lui + lli + jmp $r3)

target:
    hlt
```
