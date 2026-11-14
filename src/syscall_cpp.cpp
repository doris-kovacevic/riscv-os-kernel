#include "../h/syscall_cpp.hpp"
#include "../h/error.hpp"

//C++ API - objektni omotac
//svaki objekat u sebi nosi rucku odg objekta jezgra
//f-je clanice samo zovu f-je C API-ja (prosledjuju rucke)

//Thread--------------------------------------------------------------

Thread::Thread(void (*body)(void *), void *arg) :
        myHandle(nullptr), body(body), arg(arg)
{
}

Thread::Thread() : myHandle(nullptr), body(nullptr), arg(nullptr)
{
}

Thread::~Thread()
{
}

int Thread::start()
{
    //ako je myHandle vec postavljen onda je nit vec pokrenuta
    if (myHandle != nullptr) { return ERR_ILLEGAL_CALL; }

    //ako je konstruktorom dato telo onda pocinje izvrsavanje odatle, run se ignorise
    if (body != nullptr) { return thread_create(&myHandle, body, arg); }

    //ako nema telo funkcije, onda se poziva runwrapper koja ce pozvati polimorfnu run funkciju
    return thread_create(&myHandle, &Thread::runWrapper, this);
}

void Thread::runWrapper(void *thread)
{
    if (thread != nullptr) { ((Thread *) thread)->run(); }
}

void Thread::dispatch()
{
    thread_dispatch();
}

int Thread::sleep(time_t period)
{
    return time_sleep(period);
}

//Semaphore--------------------------------------------------------------

Semaphore::Semaphore(unsigned init) : myHandle(nullptr)
{
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore()
{
    if (myHandle != nullptr)
    {
        sem_close(myHandle);
        myHandle = nullptr;
    }
}

int Semaphore::wait()
{
    if (myHandle == nullptr) { return ERR_INVALID_HANDLE; }
    return sem_wait(myHandle);
}

int Semaphore::signal()
{
    if (myHandle == nullptr) { return ERR_INVALID_HANDLE; }
    return sem_signal(myHandle);
}

//PeriodicThread--------------------------------------------------------------

//telo se zadaje konstruktorom, pa Thread::start ne ide preko virtuelne run()
PeriodicThread::PeriodicThread(time_t period)
        : Thread(&PeriodicThread::periodicBody, this), period(period)
{
}

void PeriodicThread::terminate()
{
    //gasenje <=> perioda = 0
    period = 0;
}

void PeriodicThread::periodicBody(void *arg)
{
    PeriodicThread *thread = (PeriodicThread *) arg;

    while (thread->period != 0)
    {
        //virtuelni poziv - izvrsava se redefinicija iz izvedene klase
        thread->periodicActivation();

        if (thread->period == 0) { break; }
        Thread::sleep(thread->period);
    }
}

//Console-------------------------------------------------------------

char Console::getc()
{
    return ::getc();
}

void Console::putc(char chr)
{
    ::putc(chr);
}
