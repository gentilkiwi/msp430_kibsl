#include "generic.h"

BOOL Serial_Open(PGENERIC_COMMUNICATOR Communicator, int argc, wchar_t* argv[])
{
	BOOL ret = FALSE;
	LPCWSTR szPortName;
	PWSTR szCom;

	if (GET_CLI_ARG(L"port", &szPortName))
	{
		if (kull_m_string_sprintf(&szCom, L"\\\\.\\%s", szPortName))
		{
			kprintf(L"Using serial port: %s (%s)...: ", szPortName, szCom);

			Communicator->hCom = CreateFile(szCom, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (Communicator->hCom && (Communicator->hCom != INVALID_HANDLE_VALUE))
			{
				kprintf(L"OK\n");
				ret = TRUE;
			}
			else PRINT_ERROR_AUTO(L"CreateFile");

			LocalFree(szCom);
		}
	}
	else PRINT_ERROR(L"Argument /port:COM# needed\n");

	return ret;
}

BOOL Serial_Setup(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;
	DCB Config = { 0 };
	COMMTIMEOUTS Timeouts = { 0, 0, 0, 0, 0 };

	Config.DCBlength = sizeof(DCB);
	Config.BaudRate = CBR_9600;
	Config.fBinary = TRUE;
	Config.fParity = TRUE;
	Config.fDtrControl = DTR_CONTROL_ENABLE; // CLR_RST
	Config.fRtsControl = RTS_CONTROL_ENABLE; // CLR_TEST
	Config.ByteSize = 8;
	Config.Parity = EVENPARITY;
	Config.StopBits = ONESTOPBIT;

	if (SetupComm(Communicator->hCom, 512, 512))
	{
		if (SetCommState(Communicator->hCom, &Config))
		{
			if (SetCommTimeouts(Communicator->hCom, &Timeouts))
			{
				if (PurgeComm(Communicator->hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
				{
					ret = TRUE;
				}
				else PRINT_ERROR_AUTO(L"PurgeComm");
			}
			else PRINT_ERROR_AUTO(L"SetCommTimeouts");
		}
		else PRINT_ERROR_AUTO(L"SetCommState");
	}
	else PRINT_ERROR_AUTO(L"SetupComm");

	return ret;
}

BOOL Serial_SetBaudrate(PGENERIC_COMMUNICATOR Communicator, DWORD Baudrate)
{
	BOOL ret = FALSE;
	DCB Config;

	if (GetCommState(Communicator->hCom, &Config))
	{
		if (Config.BaudRate != Baudrate)
		{
			Config.BaudRate = Baudrate;
			Config.fDtrControl = DTR_CONTROL_DISABLE; // to comment
			Config.fRtsControl = RTS_CONTROL_ENABLE;

			if (SetCommState(Communicator->hCom, &Config))
			{
				ret = TRUE;
			}
			else PRINT_ERROR_AUTO(L"SetCommState");
		}
		else
		{
			ret = TRUE;
		}
	}
	else PRINT_ERROR_AUTO(L"GetCommState");

	return ret;
}

BOOL Serial_IO_RESET(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	BOOL ret = FALSE;

	if (EscapeCommFunction(Communicator->hCom, bValue ? CLRDTR : SETDTR))
	{
		ret = TRUE;
	}
	else PRINT_ERROR_AUTO(L"EscapeCommFunction");

	return ret;
}

BOOL Serial_IO_TEST(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	BOOL ret = FALSE;

	if (EscapeCommFunction(Communicator->hCom, bValue ? CLRRTS : SETRTS))
	{
		ret = TRUE;
	}
	else PRINT_ERROR_AUTO(L"EscapeCommFunction");

	return ret;
}

BOOL Serial_Send(PGENERIC_COMMUNICATOR Communicator, LPCVOID lpBuffer, DWORD cbToSend)
{
	BOOL ret = FALSE;
	DWORD NumberOfBytesWritten;

	if (WriteFile(Communicator->hCom, lpBuffer, cbToSend, &NumberOfBytesWritten, NULL))
	{
		if (NumberOfBytesWritten == cbToSend)
		{
			ret = TRUE;
		}
		else PRINT_ERROR(L"NumberOfBytesWritten: %u (wanted %u)\n", NumberOfBytesWritten, cbToSend);
	}
	else PRINT_ERROR_AUTO(L"WriteFile");

	return ret;
}
BOOL Serial_Recv(PGENERIC_COMMUNICATOR Communicator, LPVOID lpBuffer, DWORD cbToRead)
{
	BOOL ret = FALSE;
	DWORD NumberOfBytesRead;

	if (ReadFile(Communicator->hCom, lpBuffer, cbToRead, &NumberOfBytesRead, NULL))
	{
		if (NumberOfBytesRead == cbToRead)
		{
			ret = TRUE;
		}
		else PRINT_ERROR(L"NumberOfBytesRead: %u (wanted %hu)\n", NumberOfBytesRead, cbToRead);
	}
	else PRINT_ERROR_AUTO(L"ReadFile");

	return ret;
}

BOOL Serial_Close(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;

	if (CloseHandle(Communicator->hCom))
	{
		ret = TRUE;
	}
	else PRINT_ERROR_AUTO(L"CloseHandle");

	return ret;
}

const GENERIC_COM COM_SERIAL = {
	.Name = L"serial",
	.Open = Serial_Open,
	.Setup = Serial_Setup,
	.SetBaudrate = Serial_SetBaudrate,
	.IoReset = Serial_IO_RESET,
	.IoTest = Serial_IO_TEST,
	.Send = Serial_Send,
	.Recv = Serial_Recv,
	.Close = Serial_Close,
};