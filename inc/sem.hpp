#ifndef _sem_hpp_
#define _sem_hpp_

#include "../lib/hw.h"
#include "list.hpp"
#include "memoryAllocator.hpp"

//semafor sa redom blokiranih niti
//sem_t = _sem*
//unutar jezgra sinonim KSemaphore
//wait i signal rade sa proizvoljnim brojem jedinica
//tj za wait i signal bez argumenta se samo poziva n = 1
class _sem
{
public:
    static _sem *createSemaphore(unsigned init);

    //trazi n jedinica, blokira nit ako ih tr nema toliko
    int wait(unsigned n);

    //vraca n jedinica, odblokira niti koje mogu da uzmu koliko su trazile
    int signal(unsigned n);

    //zatvara semafor - sve niti koje su blokriane su odblokirane sa greskom
    int close();

    bool isClosed() const { return closed; }

    unsigned getValue() const { return value; }

    static void *operator new(size_t size) noexcept { return MemoryAllocator::kmalloc(size); }

    static void operator delete(void *ptr) noexcept { MemoryAllocator::kfree(ptr); }

private:
    explicit _sem(unsigned init) : value(init), closed(false), blockedQueue() {}

    unsigned value;
    bool closed;
    ThreadQueue blockedQueue;
};

typedef _sem KSemaphore;

#endif
