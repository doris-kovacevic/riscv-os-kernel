#include "../h/console.hpp"
#include "../h/sem.hpp"
#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/error.hpp"
#include "../h/syscall_c.hpp"


CharBuffer KConsole::inputBuffer;
CharBuffer KConsole::outputBuffer;

_sem *KConsole::inputItems = nullptr;
_sem *KConsole::outputItems = nullptr;
_sem *KConsole::outputSpace = nullptr;

bool KConsole::transmitting = false;

//CharBuffer-------------------------------------------------------

bool CharBuffer::put(char chr)
{
    if (isFull()) { return false; }

    data[tail] = chr;
    tail = (tail + 1) % CAPACITY;
    count++;
    return true;
}

bool CharBuffer::get(char &chr)
{
    if (isEmpty()) { return false; }

    chr = data[head];
    head = (head + 1) % CAPACITY;
    count--;
    return true;
}

//KConsole-------------------------------------------------------

bool KConsole::init()
{
    //na pocetku nema znakova za citanje
    inputItems = _sem::createSemaphore(0);
    //na pocetku nema znakova za slanje
    outputItems = _sem::createSemaphore(0);
    //na pocetku je ceo bafer prazan
    outputSpace = _sem::createSemaphore((unsigned) CharBuffer::CAPACITY);
    if (inputItems == nullptr || outputItems == nullptr || outputSpace == nullptr)
    {
        return false;
    }

    void *stack = MemoryAllocator::kmalloc(DEFAULT_STACK_SIZE);
    if (stack == nullptr) { return false; }

    //kernel nit mora da radi u kernle rezimu (mora da pristupi registrima kontrolera konzole)
    return _thread::createThread(&outputWorkerBody, nullptr,
                                 (char *) stack + DEFAULT_STACK_SIZE,
                                 _thread::KERNEL_MODE) != nullptr;
}

void KConsole::putcSyscall(char chr)
{
    //pre nego sto se konzola inicijalizuje
    if (outputSpace == nullptr) { writeDirect(chr); return; }

    //za izlaz je korisnicka nit proizvodjac
    //ako je buffer pun, blokira se dok se ne oslobodi mesto
    if (outputSpace->wait(1) != ERR_OK) { return; }

    //kada zavrsi sa cekanjem nit ovde dolazi
    //osim ako je semafor zatvoren, tada je povratna vrednost wait-a greska i prelazi u return
    outputBuffer.put(chr);
    //ima jedan vise znak za slanje
    outputItems->signal(1);
}

int KConsole::getcSyscall()
{
    if (inputItems == nullptr) { return EOF; }

    //za ulaz je korisnicka nit potrosac
    //=> ako bafer nema znakova ona se blokira
    if (inputItems->wait(1) != ERR_OK) { return EOF; }

    char chr = 0;
    if (!inputBuffer.get(chr)) { return EOF; }
    return (int) (unsigned char) chr;
}

void KConsole::outputWorkerBody(void *)
{
    //telo kernel niti koja salje znakove iz izlaznog bafera na ekran (u kontroler)
    //nit ne ulazi u jezgro kroz ecall, pa joj prekidi nisu automatski maskirani
    //=> semafore i bafer (deljeni sa sistemskim pozivima) stitimo maskiranjem prekida
    for (;;)
    {
        Riscv::mc_sstatus(Riscv::SSTATUS_SIE);

        //ceka sve dok nema nesto sto treba da posalje kontroleru
        //blokiranje sa maskiranim prekidima je bezbedno: druga nit se vraca kroz
        //svoju prekidnu rutinu, a ova nit se ovde vraca opet sa SIE = 0
        outputItems->wait(1);

        char chr = 0;
        bool taken = outputBuffer.get(chr);
        if (taken)
        {
            //znak je izvadjen, ali jos nije poslat
            transmitting = true;
            //uzet je znak iz izlaznog bafera => jedno mesto vise
            outputSpace->signal(1);
        }

        Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

        if (!taken) { continue; }

        //prozivanje, da li je kontroler spreman za prijem sledeceg znaka
        //van maske i bez uposlenog cekanja - dok nije spreman, procesor se ustupa
        while ((*(volatile char *) CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT) == 0)
        {
            thread_dispatch();
        }
        *(volatile char *) CONSOLE_TX_DATA = chr;
        transmitting = false;
    }
}

void KConsole::handleInterrupt()
{
    int irq = plic_claim();

    //ako je prekid od konzole
    if (irq == (int) CONSOLE_IRQ)
    {

        for (size_t i = 0; i < MAX_CHARS_PER_INTERRUPT; i++)
        {
            //da li je stigao znak sa tastature (postoji znak za cuvanje u inputbuffer)
            if ((*(volatile char *) CONSOLE_STATUS & CONSOLE_RX_STATUS_BIT) == 0) { break; }

            char chr = *(volatile char *) CONSOLE_RX_DATA;

            //ako je bafer pun znak se odbacuje
            //zato sto je proizvodjac hardver a on se ne moze blokirati
            if (inputBuffer.put(chr)) { inputItems->signal(1); }
        }
    }

    if (irq != 0) { plic_complete(irq); }
}

bool KConsole::outputPending()
{
    //ima znak koji je izvadjen iz bafera, a jos nije upisan u kontroler
    return !outputBuffer.isEmpty() || transmitting;
}

void KConsole::writeDirect(char chr)
{
    while ((*(volatile char *) CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT) == 0) {}
    *(volatile char *) CONSOLE_TX_DATA = chr;
}

void KConsole::printDirect(const char *text)
{
    for (const char *p = text; *p != 0; p++) { writeDirect(*p); }
}

void KConsole::panic(const char *message, uint64 value)
{
    printDirect("\r\nFATALNA GRESKA JEZGRA: ");
    printDirect(message);
    printDirect(" 0x");

    for (int shift = 60; shift >= 0; shift -= 4)
    {
        uint64 digit = (value >> shift) & 0xfUL;
        writeDirect((char) (digit < 10 ? 48 + digit : 87 + digit));
    }
    printDirect("\r\n");

    Riscv::halt();
}
