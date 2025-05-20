#pragma once
#include "../utils.h"

typedef struct _GENERIC_COMMUNICATOR {
	HANDLE hCom;
	const struct _GENERIC_COM* Com;
} GENERIC_COMMUNICATOR, * PGENERIC_COMMUNICATOR;

BOOL Generic_Com_From_Args(PGENERIC_COMMUNICATOR Communicator, int argc, wchar_t* argv[]);

typedef BOOL(*PCOM_OPEN) (PGENERIC_COMMUNICATOR Communicator, int argc, wchar_t* argv[]);
typedef BOOL(*PCOM_SETUP) (PGENERIC_COMMUNICATOR Communicator);
typedef BOOL(*PCOM_SETBAUDRATE) (PGENERIC_COMMUNICATOR Communicator, DWORD Baudrate);
typedef BOOL(*PCOM_IO_RESET) (PGENERIC_COMMUNICATOR Communicator, BYTE bValue);
typedef BOOL(*PCOM_IO_TEST) (PGENERIC_COMMUNICATOR Communicator, BYTE bValue);
typedef BOOL(*PCOM_SEND) (PGENERIC_COMMUNICATOR Communicator, LPCVOID lpBuffer, DWORD cbToSend);
typedef BOOL(*PCOM_RECV) (PGENERIC_COMMUNICATOR Communicator, LPVOID lpBuffer, DWORD cbToRead);
typedef BOOL(*PCOM_CLOSE) (PGENERIC_COMMUNICATOR Communicator);

typedef struct _GENERIC_COM {
	LPCWSTR Name;
	PCOM_OPEN Open;
	PCOM_SETUP Setup;
	PCOM_SETBAUDRATE SetBaudrate;
	PCOM_IO_RESET IoReset;
	PCOM_IO_TEST IoTest;
	PCOM_SEND Send;
	PCOM_RECV Recv;
	PCOM_CLOSE Close;
} GENERIC_COM, *PGENERIC_COM;

#define COM_OPEN(This, argc, argv)			(This)->Com->Open(This, argc, argv)
#define COM_SETUP(This)						(This)->Com->Setup(This)
#define COM_SETBAUDRATE(This, Baudrate)		(This)->Com->SetBaudrate(This, Baudrate)
#define COM_IO_RESET(This, bValue)			(This)->Com->IoReset(This, bValue)
#define COM_IO_TEST(This, bValue)			(This)->Com->IoTest(This, bValue)
#define COM_SEND(This, lpBuffer, cbToSend) 	(This)->Com->Send(This, lpBuffer, cbToSend)
#define COM_RECV(This, lpBuffer, cbToRead) 	(This)->Com->Recv(This, lpBuffer, cbToRead)
#define COM_CLOSE(This)						(This)->Com->Close(This)

extern const GENERIC_COM COM_SERIAL;
extern const GENERIC_COM COM_FTDI;
extern const GENERIC_COM COM_FT234XD;
