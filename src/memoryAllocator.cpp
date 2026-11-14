#include "../h/memoryAllocator.hpp"
#include "../h/error.hpp"

MemoryAllocator::Segment *MemoryAllocator::freeHead = nullptr;
char *MemoryAllocator::heapStart = nullptr;
char *MemoryAllocator::heapEnd = nullptr;
bool MemoryAllocator::initialized = false;

void MemoryAllocator::init()
{
    if (initialized) { return; }

    //pocetak hipa se pomera navise a kraj nanize - do granice bloka
    heapStart = (char *) alignUp((size_t) HEAP_START_ADDR, MEM_BLOCK_SIZE);
    heapEnd = (char *) ((size_t) HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE);

    freeHead = (Segment *) heapStart;
    freeHead->next = nullptr;
    freeHead->size = (size_t) (heapEnd - heapStart);

    initialized = true;
}

void *MemoryAllocator::kmalloc(size_t bytes)
{
    if (!initialized) { init(); }
    if (bytes == 0) { return nullptr; }

    //esgment mora da ima dovoljnu velicinu da sadrzi i zaglavlje i trazeni prostor
    //zaokruzeno navise na ceo broj blokova
    size_t needed = alignUp(bytes + HEADER_SIZE, MEM_BLOCK_SIZE);

    Segment *previous = nullptr;
    for (Segment *current = freeHead; current != nullptr;
         previous = current, current = current->next)
    {
        if (current->size < needed) { continue; }

        if (current->size - needed >= MEM_BLOCK_SIZE)
        {
            //ostatak je dovoljno velik da i dalje bude upotrebljiv segment
            Segment *rest = (Segment *) ((char *) current + needed);
            rest->size = current->size - needed;
            rest->next = current->next;
            current->size = needed;

            if (previous != nullptr) { previous->next = rest; }
            else { freeHead = rest; }
        }
        else
        {
            //ostatak je premali da bi se isplatilo => dodeljuje se ceo segment
            if (previous != nullptr) { previous->next = current->next; }
            else { freeHead = current->next; }
        }

        current->next = nullptr;
        return (char *) current + HEADER_SIZE;
    }

    return nullptr;
}

int MemoryAllocator::kfree(void *ptr)
{
    if (!initialized || ptr == nullptr) { return ERR_INVALID_ARG; }

    char *raw = (char *) ptr - HEADER_SIZE;

    //da li argument zaista dolazi iz kmalloc - mora da bud eu hipu i poravnat na pocetak bloka
    if (raw < heapStart || raw >= heapEnd) { return ERR_INVALID_ARG; }
    if ((size_t) (raw - heapStart) % MEM_BLOCK_SIZE != 0) { return ERR_INVALID_ARG; }

    Segment *segment = (Segment *) raw;
    if (segment->size == 0 || raw + segment->size > heapEnd) { return ERR_INVALID_ARG; }

    //ubacivanje u listu slobodnih
    Segment *previous = nullptr;
    Segment *current = freeHead;
    while (current != nullptr && current < segment)
    {
        previous = current;
        current = current->next;
    }
    if (current == segment) { return ERR_INVALID_ARG; }   //oslobadja se slobodan
    //segment lezi unutar prethodnog slobodnog segmenta
    if (previous != nullptr && (char *) previous + previous->size > raw) { return ERR_INVALID_ARG; }

    segment->next = current;
    if (previous != nullptr) { previous->next = segment; }
    else { freeHead = segment; }

    //sprecavanje fragmentacije - spajanje sa susednima
    tryJoinWithNext(segment);
    if (previous != nullptr) { tryJoinWithNext(previous); }

    return ERR_OK;
}

void MemoryAllocator::tryJoinWithNext(Segment *segment)
{
    if (segment->next == nullptr) { return; }
    if ((char *) segment + segment->size != (char *) segment->next) { return; }

    segment->size += segment->next->size;
    segment->next = segment->next->next;
}

size_t MemoryAllocator::freeBytes()
{
    size_t total = 0;
    for (Segment *current = freeHead; current != nullptr; current = current->next)
    {
        total += current->size - HEADER_SIZE;
    }
    return total;
}

size_t MemoryAllocator::freeSegments()
{
    size_t count = 0;
    for (Segment *current = freeHead; current != nullptr; current = current->next)
    {
        count++;
    }
    return count;
}
