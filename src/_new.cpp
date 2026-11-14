#include "../h/syscall_cpp.hpp"

//kod jezgra ove operatore ne koristi (tako bi jezgro pozivalo sopstveni sis poziv)
//nego klase jezgra preklapaju sopstvene operatore new i delete koji
//idu direkt u MemoryAllocator


void *operator new(size_t size)
{
    return mem_alloc(size);
}

void *operator new[](size_t size)
{
    return mem_alloc(size);
}

void operator delete(void *ptr) noexcept
{
    mem_free(ptr);
}

void operator delete[](void *ptr) noexcept
{
    mem_free(ptr);
}

//prevodilac za neke oblike brisanja generise i verziju sa velicinom
void operator delete(void *ptr, size_t) noexcept
{
    mem_free(ptr);
}

void operator delete[](void *ptr, size_t) noexcept
{
    mem_free(ptr);
}
