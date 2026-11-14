#include "../h/riscv.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/console.hpp"
#include "../h/timer.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_c.hpp"

//korisnicki program
extern void userMain();

static sem_t userMainFinished = nullptr;


static void idleBody(void *)
{
    for (;;) { thread_dispatch(); }
}

//omotac koji jezgru govori da li je korisnicki program gotov
static void userMainBody(void *)
{
    userMain();
    sem_signal(userMainFinished);
}

int main()
{
    //prvo se alokator inicijalizuje
    MemoryAllocator::init();

    //u stvec se stavi adresa prekidne rutine u koju se skace za svaki prekid
    Riscv::w_stvec((uint64) &trapEntry);

    //napravi se main thread tj funkcija main je od sad nit jezgra
    if (TCB::createMainThread() == nullptr)
    {
        KConsole::panic("nema memorije za glavnu nit");
    }

    //napravi se idle nit da procesor uvek ima sta da izvrsava
    void *idleStack = MemoryAllocator::kmalloc(DEFAULT_STACK_SIZE);
    TCB *idle = (idleStack == nullptr)
                ? nullptr
                : TCB::createThread(&idleBody, nullptr,
                                    (char *) idleStack + DEFAULT_STACK_SIZE,
                                    TCB::KERNEL_MODE, false);
    if (idle == nullptr) { KConsole::panic("nema memorije za besposlenu nit"); }
    //ne ide u red spremnih
    Scheduler::setIdleThread(idle);

    //inicijalizacija konzole (baferi, semafori i kernel nit za izlaz)
    if (!KConsole::init()) { KConsole::panic("konzola nije inicijalizovana"); }

    //od sad jezgro prima prekide ot timera i konzole
    //tj pocinje deljenje vremena i asinhrona promena konteksta
    Riscv::ms_sie(Riscv::SIE_SSIE | Riscv::SIE_SEIE);
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    //pokrece se korisnicki program u posebnoj niti
    if (sem_open(&userMainFinished, 0) != 0)
    {
        KConsole::panic("nema memorije za semafor zavrsetka");
    }

    thread_t userMainThread = nullptr;
    if (thread_create(&userMainThread, &userMainBody, nullptr) != 0)
    {
        KConsole::panic("korisnicka nit nije pokrenuta");
    }

    //ceka se da korisnicki program zabrsi
    sem_wait(userMainFinished);
    sem_close(userMainFinished);

    //praznjenje izlaznog bafera pre gasenja
    while (KConsole::outputPending()) { thread_dispatch(); }

    Riscv::halt();
    return 0;
}
