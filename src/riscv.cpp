#include "../h/riscv.hpp"
#include "../h/tcb.hpp"
#include "../h/sem.hpp"
#include "../h/timer.hpp"
#include "../h/console.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_c.hpp"
#include "../h/syscall_codes.hpp"
#include "../h/error.hpp"

const uint64 Riscv::SCAUSE_ECALL_FROM_USER;
const uint64 Riscv::SCAUSE_ECALL_FROM_SYSTEM;
const uint64 Riscv::SCAUSE_TIMER;
const uint64 Riscv::SCAUSE_EXTERNAL;

//Most izmedju trapEntry i C++ koda jezgra
extern "C" void handleSupervisorTrap(uint64 *savedRegisters)
{
    Riscv::handleTrap(savedRegisters);
}

void Riscv::popSppSpie()
{
    //sret neophodan da bismo presli u rezim koji odredjuje SPP
    //on takodje postavlja SIE = SPIE
    //u sepc upisemo ra (da se vratimo u pozivaoca funkcije)
    __asm__ volatile ("csrw sepc, ra");
    __asm__ volatile ("sret");
}

void Riscv::halt()
{
    //Upisom vrednosti 0x5555 na adresu 0x100000 zaustavlja se emulator
    *(volatile uint32 *) 0x100000 = 0x5555;
    for (;;) {}
}

void Riscv::handleTrap(uint64 *savedRegisters)
{
    uint64 scause = r_scause();

    //sepc i sstatus moraju da se cuvaju na steku tekuce niti
    //jer obrada moze da izazove promenu konteksta
    //tad bi se desilo da se u tim registrima nalaze druge vrednosti kad se vratimo
    //volatile => prevodilac ih drzi na steku niti, a ne u registru
    uint64 volatile sepc = r_sepc();
    uint64 volatile sstatus = r_sstatus();

    if (scause == SCAUSE_ECALL_FROM_USER || scause == SCAUSE_ECALL_FROM_SYSTEM)
    {
        //sepc ukazuje tacno na ecall, a pri povratku treba da idemo na prvu sledecu instr
        //(sve instr = 32 bita = 4B)
        sepc += 4;
        handleSystemCall(savedRegisters);
    }
    else if (scause == SCAUSE_TIMER)
    {
        //prekid od tajmera stize kao softverski prekid
        //ponistavamo odmah da se ne bi ponovo pojavio
        mc_sip(SIP_SSIP);
        Timer::onTick();
    }
    else if (scause == SCAUSE_EXTERNAL)
    {
        KConsole::handleInterrupt();
    }
    else
    {
        KConsole::panic("neocekivan uzrok prekida, scause = ", scause);
    }

    w_sstatus(sstatus);
    w_sepc(sepc);
}

void Riscv::handleSystemCall(uint64 *regs)
{
    //razgranati skok
    //kod funkcije se cita iz a0
    //argumenti se citaju iz  a1..a4
    //povratna vrednost se upisuje u a0
    switch (regs[REG_A0])
    {
    case SYS_MEM_ALLOC:
    {
        //na nivou abi-a velicina je izrazena u blokovima, kmalloc prima u bajtovima
        //proizvod blokovi * MEM_BLOCK_SIZE ne sme da se prelije
        size_t blocks = (size_t) regs[REG_A1];
        if (blocks == 0 || blocks > ((size_t) -1) / MEM_BLOCK_SIZE)
        {
            regs[REG_A0] = 0;
        }
        else
        {
            regs[REG_A0] = (uint64) MemoryAllocator::kmalloc(blocks * MEM_BLOCK_SIZE);
        }
        break;
    }

    case SYS_MEM_FREE:
        regs[REG_A0] = (uint64) MemoryAllocator::kfree((void *) regs[REG_A1]);
        break;

    case SYS_THREAD_CREATE:
    {
        //u a1 je samo 64-bitna vrednost (registri su sirine 64 bita)
        //pa se mora eksplicitno cast-ovati, a znamo da je na a1 sigurno thread_t*
        //jer je to definicija thread_create-a koji postavlja a1 itd.
        thread_t *handle = (thread_t *) regs[REG_A1];
        TCB::Body body = (TCB::Body) regs[REG_A2];
        void *arg = (void *) regs[REG_A3];
        void *stackTop = (void *) regs[REG_A4];

        //user mode jer smo ovde dosli iz korisnickog rezima, da treba kernel mode
        //samo se pozove createThread jer je on dostupan u kernel modu
        TCB *thread = TCB::createThread(body, arg, stackTop, TCB::USER_MODE);
        if (handle != nullptr) { *handle = thread; }
        regs[REG_A0] = (uint64) (thread != nullptr ? ERR_OK : ERR_NO_MEMORY);
        break;
    }

    case SYS_THREAD_EXIT:
        regs[REG_A0] = (uint64) ERR_OK;
        TCB::exit();            //ako se exit pravilno izvrsi, nikada se necemo ovde vratiti iz nje
        break;

    case SYS_THREAD_DISPATCH:
        TCB::dispatch();
        break;

    case SYS_SEM_OPEN:
    {
        sem_t *handle = (sem_t *) regs[REG_A1];
        KSemaphore *semaphore = KSemaphore::createSemaphore((unsigned) regs[REG_A2]);
        if (handle != nullptr) { *handle = semaphore; }
        regs[REG_A0] = (uint64) (semaphore != nullptr ? ERR_OK : ERR_NO_MEMORY);
        break;
    }

    case SYS_SEM_CLOSE:
    {
        KSemaphore *semaphore = (KSemaphore *) regs[REG_A1];
        if (semaphore == nullptr) { regs[REG_A0] = (uint64) ERR_INVALID_HANDLE; break; }

        int result = semaphore->close();
        if (result == ERR_OK) { delete semaphore; }
        regs[REG_A0] = (uint64) result;
        break;
    }

    case SYS_SEM_WAIT:
    case SYS_SEM_WAIT_N:
    {
        KSemaphore *semaphore = (KSemaphore *) regs[REG_A1];
        unsigned units = (regs[REG_A0] == SYS_SEM_WAIT) ? 1u : (unsigned) regs[REG_A2];

        if (semaphore == nullptr) { regs[REG_A0] = (uint64) ERR_INVALID_HANDLE; break; }
        regs[REG_A0] = (uint64) semaphore->wait(units);
        break;
    }

    case SYS_SEM_SIGNAL:
    case SYS_SEM_SIGNAL_N:
    {
        KSemaphore *semaphore = (KSemaphore *) regs[REG_A1];
        unsigned units = (regs[REG_A0] == SYS_SEM_SIGNAL) ? 1u : (unsigned) regs[REG_A2];

        if (semaphore == nullptr) { regs[REG_A0] = (uint64) ERR_INVALID_HANDLE; break; }
        regs[REG_A0] = (uint64) semaphore->signal(units);
        break;
    }

    case SYS_TIME_SLEEP:
        regs[REG_A0] = (uint64) Timer::sleep((time_t) regs[REG_A1]);
        break;

    case SYS_GETC:
        //mora long jer je eof negativan
        regs[REG_A0] = (uint64) (long) KConsole::getcSyscall();
        break;

    case SYS_PUTC:
        KConsole::putcSyscall((char) regs[REG_A1]);
        break;

    default:
        regs[REG_A0] = (uint64) ERR_ILLEGAL_CALL;
        break;
    }
}
