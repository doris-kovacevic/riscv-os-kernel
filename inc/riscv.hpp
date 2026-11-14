#ifndef _riscv_hpp_
#define _riscv_hpp_

#include "../lib/hw.h"


//iz trap.S
extern "C" void trapEntry();

//poziva se iz trapEntry
extern "C" void handleSupervisorTrap(uint64 *savedRegisters);

//indeksi u okviru sacuvanih reg

enum SavedRegister
{
    REG_RA = 1,
    REG_A0 = 10, REG_A1 = 11, REG_A2 = 12, REG_A3 = 13,
    REG_A4 = 14, REG_A5 = 15, REG_A6 = 16, REG_A7 = 17
};

class Riscv
{
public:
    enum BitMaskSstatus
    {
        SSTATUS_SIE  = (1 << 1),   //dozvoljeni prekidi u sistemskom rezimu
        SSTATUS_SPIE = (1 << 5),   //prethodna vrednost bita SIE
        SSTATUS_SPP  = (1 << 8)    //rezim iz kog se uslo u jezgro
    };

    enum BitMaskSip
    {
        SIP_SSIP = (1 << 1),
        SIP_STIP = (1 << 5),
        SIP_SEIP = (1 << 9)
    };

    enum BitMaskSie
    {
        SIE_SSIE = (1 << 1),
        SIE_STIE = (1 << 5),
        SIE_SEIE = (1 << 9)
    };

    //moguce vrednosti scause-a (razlozi prekida)
    static const uint64 SCAUSE_ECALL_FROM_USER   = 0x0000000000000008UL;
    static const uint64 SCAUSE_ECALL_FROM_SYSTEM = 0x0000000000000009UL;
    static const uint64 SCAUSE_TIMER             = 0x8000000000000001UL;
    static const uint64 SCAUSE_EXTERNAL          = 0x8000000000000009UL;

    //prelazak u telo nove niti
    static void popSppSpie();

    //poziva je handleSupervisorTrap, obrada uzroka prelaska u jezgro
    static void handleTrap(uint64 *savedRegisters);

    //zaustavljanje emulatora
    static void halt();

    static uint64 r_scause();

    static uint64 r_sepc();

    static void w_sepc(uint64 sepc);

    static uint64 r_stvec();

    static void w_stvec(uint64 stvec);

    static uint64 r_sstatus();

    static void w_sstatus(uint64 sstatus);

    static void ms_sstatus(uint64 mask);

    static void mc_sstatus(uint64 mask);

    static uint64 r_sip();

    static void ms_sip(uint64 mask);

    static void mc_sip(uint64 mask);

    static uint64 r_sie();

    static void ms_sie(uint64 mask);

    static void mc_sie(uint64 mask);

private:
    //switch case skok na impl sis poziva
    static void handleSystemCall(uint64 *savedRegisters);
};

inline uint64 Riscv::r_scause()
{
    uint64 volatile scause;
    __asm__ volatile ("csrr %[scause], scause" : [scause] "=r"(scause));
    return scause;
}

inline uint64 Riscv::r_sepc()
{
    uint64 volatile sepc;
    __asm__ volatile ("csrr %[sepc], sepc" : [sepc] "=r"(sepc));
    return sepc;
}

inline void Riscv::w_sepc(uint64 sepc)
{
    __asm__ volatile ("csrw sepc, %[sepc]" : : [sepc] "r"(sepc));
}

inline uint64 Riscv::r_stvec()
{
    uint64 volatile stvec;
    __asm__ volatile ("csrr %[stvec], stvec" : [stvec] "=r"(stvec));
    return stvec;
}

inline void Riscv::w_stvec(uint64 stvec)
{
    __asm__ volatile ("csrw stvec, %[stvec]" : : [stvec] "r"(stvec));
}

inline uint64 Riscv::r_sstatus()
{
    uint64 volatile sstatus;
    __asm__ volatile ("csrr %[sstatus], sstatus" : [sstatus] "=r"(sstatus));
    return sstatus;
}

inline void Riscv::w_sstatus(uint64 sstatus)
{
    __asm__ volatile ("csrw sstatus, %[sstatus]" : : [sstatus] "r"(sstatus));
}

inline void Riscv::ms_sstatus(uint64 mask)
{
    __asm__ volatile ("csrs sstatus, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sstatus(uint64 mask)
{
    __asm__ volatile ("csrc sstatus, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sip()
{
    uint64 volatile sip;
    __asm__ volatile ("csrr %[sip], sip" : [sip] "=r"(sip));
    return sip;
}

inline void Riscv::ms_sip(uint64 mask)
{
    __asm__ volatile ("csrs sip, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sip(uint64 mask)
{
    __asm__ volatile ("csrc sip, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sie()
{
    uint64 volatile sie;
    __asm__ volatile ("csrr %[sie], sie" : [sie] "=r"(sie));
    return sie;
}

inline void Riscv::ms_sie(uint64 mask)
{
    __asm__ volatile ("csrs sie, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sie(uint64 mask)
{
    __asm__ volatile ("csrc sie, %[mask]" : : [mask] "r"(mask));
}

#endif
