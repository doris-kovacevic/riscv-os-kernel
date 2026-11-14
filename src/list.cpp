#include "../h/list.hpp"
#include "../h/tcb.hpp"

//ThreadQueue-------------------------------------------------------

void ThreadQueue::addLast(_thread *thread)
{
    if (thread == nullptr) { return; }

    thread->next = nullptr;
    if (tail != nullptr) { tail->next = thread; }
    else { head = thread; }
    tail = thread;
}

_thread *ThreadQueue::removeFirst()
{
    if (head == nullptr) { return nullptr; }

    _thread *thread = head;
    head = head->next;
    if (head == nullptr) { tail = nullptr; }

    thread->next = nullptr;
    return thread;
}

bool ThreadQueue::remove(_thread *thread)
{
    if (thread == nullptr) { return false; }

    _thread *previous = nullptr;
    _thread *current = head;

    while (current != nullptr && current != thread)
    {
        previous = current;
        current = current->next;
    }

    if (current == nullptr) { return false; }

    if (previous != nullptr) { previous->next = current->next; }
    else { head = current->next; }

    //ako je izbacen poslednji, rep pomeramo na prethodnika
    if (tail == current) { tail = previous; }

    current->next = nullptr;
    return true;
}

//SleepList-------------------------------------------------------

void SleepList::insert(_thread *thread, time_t ticks)
{
    _thread *previous = nullptr;
    _thread *current = head;

    while (current != nullptr && current->sleepTicks <= ticks)
    {
        ticks -= current->sleepTicks;
        previous = current;
        current = current->next;
    }

    thread->sleepTicks = ticks;
    thread->next = current;
    if (previous != nullptr) { previous->next = thread; }
    else { head = thread; }

    //mora i sledbenik da se popravi jer ceka za ticks manje
    //(relativno u odnosu na ubacenu)
    if (current != nullptr) { current->sleepTicks -= ticks; }
}

void SleepList::tick()
{
    if (head != nullptr && head->sleepTicks > 0) { head->sleepTicks--; }
}

_thread *SleepList::removeExpired()
{
    //vraca samo jedan cvor, pozivacemo vise puta da se vrate svi istekli
    //pozivacemo dok ne dobijemo nullptr
    if (head == nullptr || head->sleepTicks > 0) { return nullptr; }

    _thread *thread = head;
    head = head->next;
    thread->next = nullptr;
    return thread;
}

bool SleepList::remove(_thread *thread)
{
    if (thread == nullptr) { return false; }

    _thread *previous = nullptr;
    _thread *current = head;

    while (current != nullptr && current != thread)
    {
        previous = current;
        current = current->next;
    }

    if (current == nullptr) { return false; }

    //sledbenik je cuvao razliku u odnosu na izbacenu nit,
    //pa mu se dodaje njeno relativno vreme da mu apsolutno vreme budjenja ostane isto
    if (current->next != nullptr) { current->next->sleepTicks += current->sleepTicks; }

    if (previous != nullptr) { previous->next = current->next; }
    else { head = current->next; }

    current->next = nullptr;
    current->sleepTicks = 0;
    return true;
}
