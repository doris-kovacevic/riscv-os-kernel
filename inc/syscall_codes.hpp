#ifndef _syscall_codes_hpp_
#define _syscall_codes_hpp_

//kodovi sis poziva
//kod se prenosi u a0, argumenti u a1..a4
//povratna vrednost u a0
enum SyscallCode
{
    SYS_MEM_ALLOC       = 0x01,
    SYS_MEM_FREE        = 0x02,

    SYS_THREAD_CREATE   = 0x11,
    SYS_THREAD_EXIT     = 0x12,
    SYS_THREAD_DISPATCH = 0x13,

    SYS_SEM_OPEN        = 0x21,
    SYS_SEM_CLOSE       = 0x22,
    SYS_SEM_WAIT        = 0x23,
    SYS_SEM_SIGNAL      = 0x24,
    SYS_SEM_WAIT_N      = 0x25,
    SYS_SEM_SIGNAL_N    = 0x26,

    SYS_TIME_SLEEP      = 0x31,

    SYS_GETC            = 0x41,
    SYS_PUTC            = 0x42
};

#endif
