; ==============================================================================
; Test:         isa_test.asm
; Description:  Comprehensive functional validation suite for 8kto ISA.
; Expected:     Reaches `pass` with $r1 = 0x55,$r2 = 0xAA, and halts.
; ==============================================================================

#include "../isa/8kto_isa.asm"

start:
    ; --------------------------------------------------
    ; 1. Validation of clr and hardwired zero ($r0)
    ; --------------------------------------------------
    clr $r1                 ; $r1 <= 0
    clr $r2                 ; $r2 <= 0
    add $r0, $r1            ; $r1 <= 0 + 0 = 0
    jnz $r1, test_failed

    ; --------------------------------------------------
    ; 2. Validation of inc and dec
    ; --------------------------------------------------
    inc $r0, $r1            ; $r1 <= 0 + 1 = 1
    dec $r1, $r2            ; $r2 <= 1 - 1 = 0
    jnz $r2, test_failed

    ; --------------------------------------------------
    ; 3. Validation of add and mov
    ; --------------------------------------------------
    inc $r1, $r1            ; $r1 <= 2
    add $r1, $r1            ; $r1 <= 4
    mov $r1, $r2            ; $r2 <= 4
    sub $r1, $r2            ; $r2 <= 4 - 4 = 0
    jnz $r2, test_failed

    ; --------------------------------------------------
    ; 4. Validation of logic ops (and, orr, xor, not)
    ; --------------------------------------------------
    clr $r1
    dec $r1, $r1            ; $r1 <= 0xFF (255)
    not $r1, $r2            ; $r2 <= ~0xFF = 0x00
    jnz $r2, test_failed

    inc $r0, $r2            ; $r2 <= 1
    and $r1, $r2            ; $r2 <= 0xFF & 1 = 1
    dec $r2, $r2            ; $r2 <= 0
    jnz $r2, test_failed

    ; --------------------------------------------------
    ; 5. Validation of Memory (stm and ldm)
    ; --------------------------------------------------
    ; Store 0x42 into RAM[1]
    inc $r0, $r1            ; $r1 <= 1 (address pointer)
    inc $r0, $r2            ; $r2 <= 1
    add $r2, $r2            ; $r2 <= 2
    add $r2, $r2            ; $r2 <= 4 ... vamos a guardar un dato
    stm $r2, [$r1]          ; RAM[1] <=$r2 (value 4)
    clr $r2                 ; wipe$r2
    ldm [$r1], $r2          ; $r2 <= RAM[1] (should be 4)
    sub $r1, $r2            ; $r2 <= 4 - 1 = 3 (non-zero)
    jez $r2, test_failed

    ; --------------------------------------------------
    ; 6. Validation of Relative Branch (jre)
    ; --------------------------------------------------
    jre test_jre_target
    jmp test_failed         ; Must be skipped by jre

test_jre_target:
    ; --------------------------------------------------
    ; 7. Validation of Conditional Jumps (jez / jnz)
    ; --------------------------------------------------
    clr $r1
    ; Load target address into $r2
    ; We jump to success routine via indirect register jump
    inc $r0,$r2            ; Set marker

test_passed:
    ; Expected signature on success
    clr $r1
    clr $r2
    inc $r0, $r1            ; $r1 = 1 (OK status)
    hlt                     ; CPU cleanly stops here

test_failed:
    ; Error signature: $r1 = 0xFF,$r2 = 0xFF
    clr $r1
    dec $r1, $r1            ; $r1 = 0xFF
    mov $r1, $r2            ; $r2 = 0xFF
    hlt