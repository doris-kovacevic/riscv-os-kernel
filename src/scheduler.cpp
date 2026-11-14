#include "../h/scheduler.hpp"
#include "../h/tcb.hpp"

ThreadQueue Scheduler::readyQueue;
_thread *Scheduler::idleThread = nullptr;

void Scheduler::put(_thread *thread)
{
    //u red se ne sme staviti idle nit
    if (thread == nullptr || thread == idleThread) { return; }

    readyQueue.addLast(thread);
}

_thread *Scheduler::get()
{
    _thread *thread = readyQueue.removeFirst();
    //ako je nullptr znaci da je red spremnih prazan => vraca se idle nit
    return thread != nullptr ? thread : idleThread;
}

void Scheduler::setIdleThread(_thread *thread)
{
    idleThread = thread;
}
