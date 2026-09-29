; ==============================================================================
; File:        8kto_isa.asm
; Description: Customasm target rule definitions and ISA specification for 8kto.
; Architecture: 8-bit Harvard, fixed 8-bit instruction width, single-cycle sequential.
; ==============================================================================

#once

; ------------------------------------------------------------------------------
; Register Encoding Rules
; ------------------------------------------------------------------------------
; Encoding:
;   $0 / $r0 -> 0b00 (Constant zero)
;   $r1      -> 0b01 (General purpose)
;   $r2      -> 0b10 (General purpose)
;   $pc / $r3-> 0b11 (Program Counter, read-only via ALU)
; ------------------------------------------------------------------------------
#subruledef reg
{
    $0        => 0b00
    $r{n: u2} => n`2
    $pc       => 0b11
}

; ------------------------------------------------------------------------------
; Instruction Encoding Rules
; ------------------------------------------------------------------------------
; Format: [7:4] Opcode | [3:2] Rs / Imm / Padding | [1:0] Rd / Offset
; ------------------------------------------------------------------------------
#ruledef
{
    ; --- Arithmetic-Logic Operations (0000 - 0111) ---
    add {rs: reg}, {rd: reg} => 0b0000 @ rs @ rd  ; Rd <= Rd + Rs
    sub {rs: reg}, {rd: reg} => 0b0001 @ rs @ rd  ; Rd <= Rd - Rs
    and {rs: reg}, {rd: reg} => 0b0010 @ rs @ rd  ; Rd <= Rd & Rs
    orr {rs: reg}, {rd: reg} => 0b0011 @ rs @ rd  ; Rd <= Rd | Rs
    xor {rs: reg}, {rd: reg} => 0b0100 @ rs @ rd  ; Rd <= Rd ^ Rs
    not {rs: reg}, {rd: reg} => 0b0101 @ rs @ rd  ; Rd <= ~Rs
    inc {rs: reg}, {rd: reg} => 0b0110 @ rs @ rd  ; Rd <= Rs + 1
    dec {rs: reg}, {rd: reg} => 0b0111 @ rs @ rd  ; Rd <= Rs - 1

    ; --- Data Memory Operations (1000 - 1001) ---
    ldm [{rs: reg}], {rd: reg} => 0b1000 @ rs @ rd ; Rd <= RAM[Rs]
    stm {rs: reg}, [{rd: reg}] => 0b1001 @ rs @ rd ; RAM[Rd] <= Rs

    ; --- Control Flow & Jump Operations (1010 - 1101, 1111) ---
    jmp            {rd: reg} => 0b1010 @ 0b00 @ rd ; PC <= Rd (Unconditional indirect)
    jez {rs: reg}, {rd: reg} => 0b1011 @ rs @ rd   ; if (Rs == 0) PC <= Rd
    jnz {rs: reg}, {rd: reg} => 0b1100 @ rs @ rd   ; if (Rs != 0) PC <= Rd
    jre            {addr}    => {                  ; PC <= PC_current + sign_ext(offset)
        reladdr = addr - $
        assert(reladdr <= 7)
        assert(reladdr >= -8)
        0b1101 @ (reladdr`4)
    }
    hlt                      => 0b1111 @ 0b0000    ; Halt processor execution

    ; --- Pseudo-instructions & Idioms ---
    nop => asm {
        add $r0,$r0                               ; No operation
    }

    clr {rd: reg} => asm {
        xor {rd}, {rd}                             ; Rd <= 0
    }

    mov {rs: reg}, {rd: reg} => asm {
        xor {rd}, {rd}                             ; Clear destination
        add {rs}, {rd}                             ; Rd <= Rs
    }
}

; ------------------------------------------------------------------------------
; Memory Map Configuration (ROM)
; ------------------------------------------------------------------------------
#bankdef rom
{
    #bits 8
    #addr 0x00
    #size 256
    #fill 0x00
    #outp 0
}
