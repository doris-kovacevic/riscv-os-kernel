#ifndef _tcb_hpp_
#define _tcb_hpp_

#include "../lib/hw.h"
#include "list.hpp"
#include "memoryAllocator.hpp"


//kontekst niti, rucno se cuva pri promeni konteksta
//cuvaju se ra (ret addr), sp i registri s0..s11
//prevodilac pretpostavlja da se s0..s11 ne menjaju preko poziva funkcije, a
//contextSwitch se "vraca" u drugoj niti => moraju se cuvati po niti
//privremene registre i registre argumenata sacuvala je trap rutina

struct Context
{
    uint64 ra;
    uint64 sp;
    uint64 s[12];   //s0..s11
};

extern "C" void contextSwitch(Context *oldContext, Context *newContext);

//_thread je pcb, unutar jezgra se korisit sinonim tcb
class _thread
{
public:
    //telo funkcije
    using Body = void (*)(void *);

    //rezim u kom se izvrsava telo niti
    enum Mode
    {
        USER_MODE, KERNEL_MODE
    };

    //pravi niti i postavlja u red spremnih
    //stakcTop - adresa odmah iznad posl B odvojenog za stek (najveca addr)
    static _thread *createThread(Body body, void *arg, void *stackTop, Mode mode,
                                 bool ready = true);

    //pravi tcb za nit u kojoj vec tece main()
    static _thread *createMainThread();

    static void dispatch();

    //gasi tekucu i prelazi na sl nit (nikada se ne vraca ako se izvrsi pravilno)
    static void exit();

    //nit koja tr ima procesor
    static _thread *running;

    bool isFinished() const { return finished; }

    void setFinished(bool value) { finished = value; }

    bool isBlocked() const { return blocked; }

    void setBlocked(bool value) { blocked = value; }

    Mode getMode() const { return mode; }

    unsigned getSemRequest() const { return semRequest; }

    void setSemRequest(unsigned value) { semRequest = value; }

    int getSemResult() const { return semResult; }

    void setSemResult(int value) { semResult = value; }

    //trosi jednu periodu timera i vraca true ako joj je kvantum istekao
    bool onTimerTick();


    //da se niti alociraju globalnim operatorom new ond bi jezgro zvalo sopstveni sistemski poziv
    //=> moramo redefinisati operator na alokator jezgra
    static void *operator new(size_t size) noexcept { return MemoryAllocator::kmalloc(size); }

    //isto kao gore
    static void operator delete(void *ptr) noexcept { MemoryAllocator::kfree(ptr); }

private:
    _thread(Body body, void *arg, void *stackTop, Mode mode);

    ~_thread();

    //od ove fje pocinje svaka nit
    static void threadWrapper();

    //oslobadja tcb-ove ugasenih niti, radi je nit koja dobije procesor
    static void reapFinished();

    Body body;
    void *arg;
    //kfree dobija najmanju adr zauzetog prostora => ne moze se stackTop proslediti
    void *stackBase;
    Context context;        //sacuvani ra i sp
    Mode mode;
    bool finished;
    bool blocked;
    time_t timeSlice;       //kvantum, u periodama timera
    time_t elapsedTicks;    //potrosene periode
    unsigned semRequest;    //koliko jedinica nit trazi na semaforu
    int semResult;          //rez cekanja na semaforu
    time_t sleepTicks;      //relativno vreme budjenja
    _thread *next;          //za ulancavanje u red


    //nit koja dobije procesor oslobadja stekove zavrsenih niti
    //zavrsene ne smeju same svoj stek da oslobadjaju
    static ThreadQueue finishedQueue;

    friend class ThreadQueue;

    friend class SleepList;
};

typedef _thread TCB;

#endif
