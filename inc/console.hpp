#ifndef _console_hpp_
#define _console_hpp_

#include "../lib/hw.h"

class _sem;

//kruzni bafer znakova (fiks velicina)
//nema konstruktor => staticki primerci u .bss
class CharBuffer
{
public:
    static const size_t CAPACITY = 256;

    bool isEmpty() const { return count == 0; }

    bool isFull() const { return count == CAPACITY; }

    bool put(char chr);

    bool get(char &chr);

private:
    char data[CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
};

//kernel klasa koja upravlja konzolom
//izlaz - putc smesta znak u izlazni buffer => korisnicka nit je proizvodjac
//          interna nit jezgra ga prozivanjem prenosi kontroleru - potrosac
//ulaz - prekidna rutina cita znakove izkontrolera u ulazni buffer (proizvodjac)
//      getc ih uzima iz buffera => korisnicka nit je potrosac

//cekanjem na semaforu je izbegnut busy wait

class KConsole
{
public:
    //pravi semafore + nit jezgra za izlaz
    static bool init();

    //implementiraju syscall_c putc i getc
    static void putcSyscall(char chr);
    static int getcSyscall();

    //obrada spolj hardverskog prekida
    static void handleInterrupt();

    //da li ima jos karaktera u izlaznom baferu
    static bool outputPending();

    //ispis bez obzira na bafer
    //za poruke o fatalnoj greski
    static void panic(const char *message, uint64 value = 0);

private:
    //telo kernel niti koja uzima iz outputbafera karaktere i fizicki ih salje
    //kontroleru
    static void outputWorkerBody(void *);

    //odmah salje jedan karakter hardveru - za panic
    static void writeDirect(char chr);

    //niz karaktera, pomocu writeDirect
    static void printDirect(const char *text);

    //koliko max znakova sme da se prihvati u jednom prekidu
    static const size_t MAX_CHARS_PER_INTERRUPT = 64;

    static CharBuffer inputBuffer;
    static CharBuffer outputBuffer;

    static _sem *inputItems;    //znakovi spremni za citanje
    static _sem *outputItems;   //znakovi koji cekaju slanje
    static _sem *outputSpace;   //koliko j eslobodnih mesta u izlaznom baferu

    static bool transmitting;   //znak je izvadjen iz bafera, a jos nije poslat
};

#endif
