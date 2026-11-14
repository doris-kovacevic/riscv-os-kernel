#ifndef _scheduler_hpp_
#define _scheduler_hpp_

#include "list.hpp"

class _thread;

//scheduler - fifo alg nad redom spremnih
//idle nit - nikada nije u redu spremnih, bira se samo ako je red spreminih prazan
//tj sluzi samo da procesor uvek nesto moze da dobije
class Scheduler
{
public:
    static void put(_thread *thread);

    static _thread *get();

    static void setIdleThread(_thread *thread);

private:
    static ThreadQueue readyQueue;
    static _thread *idleThread;
};

#endif
