#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/riscv.hpp"
#include "../h/console.hpp"
#include "../h/error.hpp"
#include "../h/syscall_c.hpp"

_thread *_thread::running = nullptr;
ThreadQueue _thread::finishedQueue;

_thread::_thread(Body body, void *arg, void *stackTop, Mode mode) :
        body(body),
        arg(arg),
        stackBase(stackTop == nullptr
                  ? nullptr
                  : (void *) ((char *) stackTop - DEFAULT_STACK_SIZE)),
        //pocetni kontekst - povratna adresa je omotac od kog nit pocinje
        //vrh steka = vrh njoj dodeljenog prostora
        //prva promena konteksta na ovu nit (instrukcijom ret) skace u omotac
        context({(uint64) &threadWrapper, (uint64) stackTop}),
        mode(mode),
        finished(false),
        blocked(false),
        timeSlice(DEFAULT_TIME_SLICE),
        elapsedTicks(0),
        semRequest(0),
        semResult(ERR_OK),
        sleepTicks(0),
        next(nullptr)
{
}

_thread::~_thread()
{
    if (stackBase != nullptr) { MemoryAllocator::kfree(stackBase); }
}

_thread *_thread::createThread(Body body, void *arg, void *stackTop, Mode mode, bool ready)
{
    if (body == nullptr || stackTop == nullptr) { return nullptr; }

    _thread *thread = new _thread(body, arg, stackTop, mode);
    if (thread == nullptr) { return nullptr; }


    //ako je spremna za izvrsavanje scheduler je stavlja u red spreminh
    if (ready) { Scheduler::put(thread); }
    return thread;
}

_thread *_thread::createMainThread()
{
    //nit u kojoj vec tece main()
    //nema telo ni novi stek - nastavlja da radi na zatecenom (pocetnom) steku prog
    running = new _thread(nullptr, nullptr, nullptr, KERNEL_MODE);
    return running;
}

void _thread::dispatch()
{
    //running je tr tekuca, njoj se oduzima (potencijalno) procesor
    _thread *old = running;

    if (old->isFinished())
    {
        //ako se nit ugasila ona ne sme sama sebi da oslobodi stek
        //jer jos uvek radi na njemu (zove dispatch u exit)
        //=> to mora nit koja dobije procesor da uradi
        finishedQueue.addLast(old);
    }
    else if (!old->isBlocked())
    {
        //ako nije blokirana => scheduler je stavlja u red spremnih
        Scheduler::put(old);
    }

    running = Scheduler::get();
    if (running == nullptr) { KConsole::panic("nema nijedne spremne niti"); }
    running->elapsedTicks = 0;

    contextSwitch(&old->context, &running->context);

    //nova nit ce ovo uraditi, ako se old ubacilo u finished red nikada ona nece doci do ovde jer
    //se ona ne vraca iz exit => ne vraca se iz contextSwitch na ovu liniju
    //npr. A je izgubila procesor od strane B, nije finished, kad dobije opet procesor, A se vrati ovde
    //i uradi reapFinished i to je bezbedno jer A nije u finished redu jer nije gotova (da jeste ne bi dosla
    //do reapfinished)
    reapFinished();
}

void _thread::exit()
{
    running->setFinished(true);
    //ako je ova nit gotova mora dati drugoj niti procesor
    dispatch();
}

void _thread::reapFinished()
{
    while (!finishedQueue.isEmpty())
    {
        delete finishedQueue.removeFirst();
    }
}

bool _thread::onTimerTick()
{
    if (++elapsedTicks < timeSlice) { return false; }

    elapsedTicks = 0;
    return true;
}

void _thread::threadWrapper()
{

    //nova nit dobija procesor unutar jezgra
    //=> u sistemskom rezimu + maskirani prekidi
    //=> pre poziva tela niti bit SPP se mora postaviti prema rezimu niti
    //i bit spie na 1
    //zatim kad se pomocu sret iz popSppSpie vratimo ovde bicemo u odg rezimu sa
    //dozvoljenim prekidima
    if (running->mode == KERNEL_MODE) { Riscv::ms_sstatus(Riscv::SSTATUS_SPP); }
    else { Riscv::mc_sstatus(Riscv::SSTATUS_SPP); }
    Riscv::ms_sstatus(Riscv::SSTATUS_SPIE);
    Riscv::popSppSpie();

    //sada se tek izvrsava telo funkcije niti koje je zadato pri kreiranju
    running->body(running->arg);

    //iz omotaca se ne smemo vratiti zato sto je vestacki pozvana tj nema povratna adresa
    //tj moramo ugasiti nit ako se nije ugasila pre povratka ovde
    thread_exit();
}
