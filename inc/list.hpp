#ifndef _list_hpp_
#define _list_hpp_

#include "../lib/hw.h"

class _thread;

//fifo red niti
//red spremnih, red blokranih na sem i red ugasenih
//ulancavanje pomocu pokazivaca unutar same niti
//nit u svakom tr sme da bude u max jednom redu
class ThreadQueue
{
public:
    constexpr ThreadQueue() : head(nullptr), tail(nullptr) {}

    ThreadQueue(const ThreadQueue &) = delete;

    ThreadQueue &operator=(const ThreadQueue &) = delete;

    bool isEmpty() const { return head == nullptr; }

    _thread *peekFirst() const { return head; }

    void addLast(_thread *thread);

    _thread *removeFirst();

    //izbacuje zadatu nit iz reda ako se nalazi u njemu
    //vraca true ako je nit pronadjena i izbacena
    bool remove(_thread *thread);

private:
    _thread *head;
    _thread *tail;
};

//lista uspavanih niti, uredjena u tr budjenja
//nit pamti razliku vremena budjenja (kao u tss)
//tj A se budi za 3, B za 4, C za 4 => a(3)->b(1)->c(0)
class SleepList
{
public:
    constexpr SleepList() : head(nullptr) {}

    SleepList(const SleepList &) = delete;

    SleepList &operator=(const SleepList &) = delete;

    bool isEmpty() const { return head == nullptr; }

    //uspavljivanje niti na ticks perioda timera
    void insert(_thread *thread, time_t ticks);

    //poziva se na svaku periodu timera
    void tick();

    //vraca redom niti kojima je vreme spavanja isteklo
    //vraca i nullptr kad vrati sve kojima je isteklo
    _thread *removeExpired();

    //izbacuje zadatu nit iz liste ako se nalazi u njoj
    //vraca true ako je nit pronadjena i izbacena
    bool remove(_thread *thread);

private:
    _thread *head;
};

#endif
