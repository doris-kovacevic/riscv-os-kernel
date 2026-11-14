#include "../h/timer.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/error.hpp"

SleepList Timer::sleepingThreads;

void Timer::onTick()
{
    //poziva se na svaki otkucaj timera
    //prvo budjenje uspavanih niti
    //posle toga smanjivanje vremena spavanja ostalih niti za jednu periodu
    sleepingThreads.tick();

    for (_thread *awakened = sleepingThreads.removeExpired();
         awakened != nullptr;
         awakened = sleepingThreads.removeExpired())
    {
        awakened->setBlocked(false);
        Scheduler::put(awakened);
    }

    if (_thread::running->onTimerTick()) { _thread::dispatch(); }
}

int Timer::sleep(time_t period)
{
    //Spavanje nula perioda znaci samo ustupanje procesora
    if (period == 0)
    {
        _thread::dispatch();
        return ERR_OK;
    }

    _thread *thread = _thread::running;
    thread->setBlocked(true);
    sleepingThreads.insert(thread, period);

    _thread::dispatch();
    return ERR_OK;
}
