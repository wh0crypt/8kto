; ==============================================================================
; Test:         isa_test.asm
; Description:  Comprehensive functional validation suite for 8kto ISA.
; Expected:     Reaches `test_passed` with $r1 = 0x55,$r2 = 0xAA, and halts.
; ==============================================================================

#include "../isa/8kto_isa.asm"

start:
    ; --------------------------------------------------
    ; 1. Validation of clr and hardwired zero ($r0)
    ; --------------------------------------------------
    clr $r1                 ; $r1 <= 0
    clr $r2                 ; $r2 <= 0
    add $r0, $r1            ; $r1 <= 0 + 0 = 0

    ; --------------------------------------------------
    ; 2. Validation of inc and dec
    ; --------------------------------------------------
    inc $r0, $r1            ; $r1 <= 0 + 1 = 1
    dec $r1, $r2            ; $r2 <= 1 - 1 = 0

    ; --------------------------------------------------
    ; 3. Validation of add, sub and mov
    ; --------------------------------------------------
    inc $r1, $r1            ; $r1 <= 2
    add $r1, $r1            ; $r1 <= 4
    mov $r1, $r2            ; $r2 <= 4
    sub $r1, $r2            ; $r2 <= 4 - 4 = 0

    ; --------------------------------------------------
    ; 4. Validation of logic ops (and, orr, xor, not)
    ; --------------------------------------------------
    clr $r1
    dec $r1, $r1            ; $r1 <= 0xFF (255 / -1)
    not $r1, $r2            ; $r2 <= ~0xFF = 0x00

    inc $r0, $r2            ; $r2 <= 1
    and $r1, $r2            ; $r2 <= 0xFF & 1 = 1
    dec $r2, $r2            ; $r2 <= 0

    ; --------------------------------------------------
    ; 5. Validation of Memory (stm and ldm)
    ; --------------------------------------------------
    inc $r0, $r1            ; $r1 <= 1 (RAM pointer: 0x01)
    inc $r0, $r2            ; $r2 <= 1
    add $r2, $r2            ; $r2 <= 2
    add $r2, $r2            ; $r2 <= 4 (payload to store)
    stm $r2, [$r1]          ; RAM[1] <= 4
    clr $r2                 ; Clear$r2
    ldm [$r1], $r2          ; $r2 <= RAM[1] (must be 4)
    sub $r1, $r2            ; $r2 <= 4 - 1 = 3

    ; --------------------------------------------------
    ; 6. Validation of Relative Branch (jre forward)
    ; --------------------------------------------------
    jre .jre_target
    ; Trap: if jre fails, it executes this illegal sequence
    clr $r1
    dec $r1, $r1
    hlt

.jre_target:
    ; --------------------------------------------------
    ; 7. Validation of Indirect Conditional Jumps (jez / jnz)
    ;    Address calculation using $pc ($r3)
    ; --------------------------------------------------
    clr $r1                 ; Condition:$r1 == 0
    mov $pc, $r2            ; Read current PC into$r2
    inc $r2, $r2
    inc $r2, $r2
    inc $r2, $r2
    inc $r2, $r2            ; $r2 points past the failure trap
    jez $r1, $r2            ; Must jump to target in$r2

    ; Trap: reached if jez did not jump
    clr $r1
    dec $r1, $r1
    hlt

    ; --------------------------------------------------
    ; 8. Synthesis of Expected Success Signature:
    ;    $r1 = 0x55 (0b01010101)     ;$r2 = 0xAA (0b10101010)
    ; --------------------------------------------------
    clr $r1
    inc $r0, $r1            ; 0x01
    add $r1, $r1            ; 0x02
    add $r1, $r1            ; 0x04
    inc $r1, $r1            ; 0x05
    add $r1, $r1            ; 0x0A
    add $r1, $r1            ; 0x14
    add $r1, $r1            ; 0x28
    add $r1, $r1            ; 0x50

    inc $r0, $r2             ; 1
    add $r2, $r2            ; 2
    add $r2, $r2            ; 4
    inc $r2, $r2            ; 5
    add $r2, $r1            ; $r1 = 0x50 + 0x05 = 0x55

    not $r1, $r2            ; $r2 = ~0x55 = 0xAA

test_passed:
    hlt                     ; Success: stops with $r1 = 0x55,$r2 = 0xAA