#include "../isa/8kto_isa.asm"

start:
    clr $r1              ; $r1 <= 0
    inc $r1              ; $r1 <= 1 (via adi 1, $r1)
    mov $r1, $r2         ; $r2 <= 1
    jla target           ; Fixed 3-byte expansion (lui + lli + jmp $r3)
    nop
    nop
    nop
    add $r2, $r2         ; $r2 <= 4

target:
    hlt
