#include "../h/syscall_c.hpp"
#include "../h/syscall_codes.hpp"
#include "../h/error.hpp"

//zaj deo svih sistemskih poziva
//samo ovde se ecall izvrsava
//abi sloj: a0 = kod poziva; a1..a4 = arg (sleva nadesno); a0 = povratna vrednost
//mora preko lokalnih promenljivih (da je preko asm blokova ti registri preko kojih se upisuje
//bi mogli biti prepisani umedjuvremenu
static uint64 syscall(uint64 code, uint64 arg1 = 0, uint64 arg2 = 0,
                      uint64 arg3 = 0, uint64 arg4 = 0)
{
    register uint64 a0 __asm__("a0") = code;
    register uint64 a1 __asm__("a1") = arg1;
    register uint64 a2 __asm__("a2") = arg2;
    register uint64 a3 __asm__("a3") = arg3;
    register uint64 a4 __asm__("a4") = arg4;


    //+r da bi u a0 moglo i da se cita i pise, ostali r jer se samo cita
    __asm__ volatile ("ecall"
                      : "+r"(a0)
                      : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
                      : "memory");

    return a0;
}

//memroija-------------------------------------------------------------------

void *mem_alloc(size_t size)
{
    if (size == 0) { return nullptr; }

    //abi ocekuje velicinu u blokovima
    size_t blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;

    return (void *) syscall(SYS_MEM_ALLOC, blocks);
}

int mem_free(void *ptr)
{
    return (int) syscall(SYS_MEM_FREE, (uint64) ptr);
}

//niti-------------------------------------------------------------------

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg)
{
    if (handle == nullptr || start_routine == nullptr) { return ERR_INVALID_ARG; }

    //abi sloj ocekuje odvojen prostor za stek
    void *stack = mem_alloc(DEFAULT_STACK_SIZE);
    if (stack == nullptr) { return ERR_NO_MEMORY; }

    //argument je najveca lokacija steka, jer stek popunjava lokacije odozdo nagore
    //tj raste ka manjim adresama (nizim)
    void *stackTop = (void *) ((char *) stack + DEFAULT_STACK_SIZE);

    int result = (int) syscall(SYS_THREAD_CREATE, (uint64) handle,
                               (uint64) start_routine, (uint64) arg, (uint64) stackTop);

    //ne moze odmah return result jer mora stek da se oslobodi ako cela nit nije alocirana kako treba
    if (result != ERR_OK) { mem_free(stack); }
    return result;
}

int thread_exit()
{
    return (int) syscall(SYS_THREAD_EXIT);
}

void thread_dispatch()
{
    syscall(SYS_THREAD_DISPATCH);
}

//semafori-------------------------------------------------------------------

int sem_open(sem_t *handle, unsigned init)
{
    if (handle == nullptr) { return ERR_INVALID_ARG; }

    return (int) syscall(SYS_SEM_OPEN, (uint64) handle, (uint64) init);
}

int sem_close(sem_t handle)
{
    return (int) syscall(SYS_SEM_CLOSE, (uint64) handle);
}

int sem_wait(sem_t id)
{
    return (int) syscall(SYS_SEM_WAIT, (uint64) id);
}

int sem_signal(sem_t id)
{
    return (int) syscall(SYS_SEM_SIGNAL, (uint64) id);
}

int sem_wait_n(sem_t id, unsigned n)
{
    return (int) syscall(SYS_SEM_WAIT_N, (uint64) id, (uint64) n);
}

int sem_signal_n(sem_t id, unsigned n)
{
    return (int) syscall(SYS_SEM_SIGNAL_N, (uint64) id, (uint64) n);
}

//timeri-------------------------------------------------------------------

int time_sleep(time_t period)
{
    return (int) syscall(SYS_TIME_SLEEP, (uint64) period);
}

//konzola-------------------------------------------------------------------

char getc()
{
    return (char) syscall(SYS_GETC);
}

void putc(char chr)
{
    syscall(SYS_PUTC, (uint64) chr);
}
