//ulaz u jezgro - samo ovuda se moze preci iz korisnickog koda u kod jezgra i nazad
//stvec ukazuje na ovu rutinu => procesor skace ovde za sve uzroke
//rutina radi na steku niti koja je prekinuta

.extern handleSupervisorTrap

.section .text
//mora biti poravnata na 4
.align 4
.global trapEntry
//trapEntry je tipa funkcije
.type trapEntry, @function
trapEntry:
    addi sp, sp, -256               // prostor za 32 reg po 8B

    sd x1, 1 * 8(sp)                # x0 (ozicen na 0) i x2 (sp) se ne cuvaju
    .irp index, 3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(sp)
    .endr

    mv a0, sp                       // arg = adresa sacuvanog okvira ("pocetak niza sacuvanih reg")
    call handleSupervisorTrap

    ld x1, 1 * 8(sp)
    .irp index, 3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(sp)
    .endr

    addi sp, sp, 256
    sret
