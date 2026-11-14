#ifndef _memoryAllocator_hpp_
#define _memoryAllocator_hpp_

#include "../lib/hw.h"

//kontinualna alokacija memorije, first fit
//ceo hip je niz segmenata
//svaki segment pocinje zaglavljem Segment
//slobodni segmenti ulancani u listu sortiranu po rastucim adresama
//klasa je usluzna, ne singleton (ima samo staticke clanove)
class MemoryAllocator
{
public:
    //postavlja ceo hip u jedan slobodan segment
    static void init();

    //vraca pokazivac iza kog ima min bytes upotrebljivih B
    //tj nullptr ako nema takvog
    static void *kmalloc(size_t bytes);

    //oslobadja prostor dobijen izkmalloc
    static int kfree(void *ptr);

    //koliko ima ukupno slobodnih bajtova
    static size_t freeBytes();

    //koliko ima ukupno slobodnih segmenata
    static size_t freeSegments();

private:
    struct Segment
    {
        Segment *next;   //sledeci slobodan segment
        size_t size;     //velicina segmenta ZAJEDNO SA ZAGLAVLJEM
    };

    //zaglavlje je 16B
    //prostor koji korisnik dobija je odmah iz anjega
    static const size_t HEADER_SIZE = sizeof(Segment);

    static void tryJoinWithNext(Segment *segment);

    static size_t alignUp(size_t value, size_t alignment)
    {
        return (value + alignment - 1) / alignment * alignment;
    }

    static Segment *freeHead;
    static char *heapStart;
    static char *heapEnd;
    static bool initialized;
};

#endif
