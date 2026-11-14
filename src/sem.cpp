#include "../h/sem.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/error.hpp"

_sem *_sem::createSemaphore(unsigned init)
{
    return new _sem(init);
}

int _sem::wait(unsigned n)
{
    if (closed) { return ERR_SEM_CLOSED; }
    if (n == 0) { return ERR_OK; }

    if (value >= n)
    {
        value -= n;
        return ERR_OK;
    }

    //nema dovoljno jedinica => nit se blokira
    //pamti seee u samoj niti koliko je trazila jedinica
    //za sad je rezultat cekanja 0 jer nije zatvoren semafor
    _thread *thread = _thread::running;
    thread->setSemRequest(n);
    thread->setSemResult(ERR_OK);
    thread->setBlocked(true);
    blockedQueue.addLast(thread);

    _thread::dispatch();

    //posle povratka se ne sme pristupiti semaforu jer je moguce da je umedjuvremenu zatvoren i unisten
    //zato je semresult morao biti u samoj niti
    return thread->getSemResult();
}

int _sem::signal(unsigned n)
{
    if (closed) { return ERR_SEM_CLOSED; }

    value += n;

    //odblokira koliko god nizi moze da prodje, redom kojim su ulazile u red cekanja
    while (!blockedQueue.isEmpty() && blockedQueue.peekFirst()->getSemRequest() <= value)
    {
        _thread *thread = blockedQueue.removeFirst();
        value -= thread->getSemRequest();
        thread->setSemResult(ERR_OK);
        thread->setBlocked(false);
        Scheduler::put(thread);
    }

    return ERR_OK;
}

int _sem::close()
{
    if (closed) { return ERR_SEM_CLOSED; }
    closed = true;

    //sve niti koje su bile blokirane se odblokiraju ali sa greskom kao rezultatom
    while (!blockedQueue.isEmpty())
    {
        _thread *thread = blockedQueue.removeFirst();
        thread->setSemResult(ERR_SEM_CLOSED);
        thread->setBlocked(false);
        Scheduler::put(thread);
    }

    return ERR_OK;
}
