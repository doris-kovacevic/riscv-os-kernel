#ifndef _error_hpp_
#define _error_hpp_

//kodovi gresaka, negativne vrednosti, uspeh je 0
enum ErrorCode
{
    ERR_OK             =  0,
    ERR_INVALID_ARG    = -1,
    ERR_NO_MEMORY      = -2,
    ERR_INVALID_HANDLE = -3,   //rucka ne ukazuje na postojeci objekat
    ERR_SEM_CLOSED     = -4,   //semafor je dealociran (u toku cekanja)
    ERR_ILLEGAL_CALL   = -5
};

#endif
