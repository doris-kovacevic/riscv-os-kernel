#ifndef _timer_hpp_
#define _timer_hpp_

#include "../lib/hw.h"
#include "list.hpp"


//timer generise prekid 10 puta u sekundi
//=> perioda = 100ms
class Timer
{
public:
    //poziva se iz prekidne rutine na svaki prekid timera (na svaku periodu)
    static void onTick();

    //impl. time_sleep sis poziva
    static int sleep(time_t period);

private:
    static SleepList sleepingThreads;
};

#endif
