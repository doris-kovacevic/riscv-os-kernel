//promena konteksta izmedju dve niti (ra, sp, s0..s11)
//privremene reg i reg arg je sacuvao trapEntry (ovde se ulazi preko thread_dispatch itd)
//a0 - kontekst stare niti, a1 - kontekst nove niti
//u a0 cuvamo tr ra, sp i s0..s11
//u ra, sp i s0..s11 ucitavamo vrednosti iz a1 (kontekst nove niti)
//pomeraji odgovaraju redosledu polja u strukturi Context (tcb.hpp)

.section .text
.global contextSwitch
.type contextSwitch, @function
contextSwitch:
    sd ra, 0 * 8(a0)
    sd sp, 1 * 8(a0)
    sd s0, 2 * 8(a0)
    sd s1, 3 * 8(a0)
    sd s2, 4 * 8(a0)
    sd s3, 5 * 8(a0)
    sd s4, 6 * 8(a0)
    sd s5, 7 * 8(a0)
    sd s6, 8 * 8(a0)
    sd s7, 9 * 8(a0)
    sd s8, 10 * 8(a0)
    sd s9, 11 * 8(a0)
    sd s10, 12 * 8(a0)
    sd s11, 13 * 8(a0)

    ld ra, 0 * 8(a1)
    ld sp, 1 * 8(a1)
    ld s0, 2 * 8(a1)
    ld s1, 3 * 8(a1)
    ld s2, 4 * 8(a1)
    ld s3, 5 * 8(a1)
    ld s4, 6 * 8(a1)
    ld s5, 7 * 8(a1)
    ld s6, 8 * 8(a1)
    ld s7, 9 * 8(a1)
    ld s8, 10 * 8(a1)
    ld s9, 11 * 8(a1)
    ld s10, 12 * 8(a1)
    ld s11, 13 * 8(a1)

    ret
