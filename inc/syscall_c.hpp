#ifndef _syscall_c_hpp_
#define _syscall_c_hpp_

#include "../lib/hw.h"

//C API

//memorija---------------------------------------------------------------------

void *mem_alloc(size_t size);

int mem_free(void *ptr);

//niti---------------------------------------------------------------------

class _thread;
typedef _thread *thread_t;

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg);

int thread_exit();

void thread_dispatch();

//semafori---------------------------------------------------------------------

class _sem;
typedef _sem *sem_t;

int sem_open(sem_t *handle, unsigned init);

int sem_close(sem_t handle);

int sem_wait(sem_t id);

int sem_signal(sem_t id);

int sem_wait_n(sem_t id, unsigned n);

int sem_signal_n(sem_t id, unsigned n);

//timer---------------------------------------------------------------------

int time_sleep(time_t period);

//konzola---------------------------------------------------------------------

const int EOF = -1;

char getc();

void putc(char chr);

#endif
