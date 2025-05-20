#include "generic.h"
#include "ftd2xx.h"

#define CBUS_BIT(n, v)					((1 << (4 + n)) | (v << n))
#define PRINT_FT_ERROR(func, status)	kprintf(L"ERROR " TEXT(__FUNCTION__) L" ; " func L" : %S (%lu)\n", FT_STATUS_to_NAME(status), status)
PCSTR FT_STATUS_to_NAME(FT_STATUS status);

BOOL FTDI_Open(PGENERIC_COMMUNICATOR Communicator, int argc, wchar_t* argv[])
{
	BOOL ret = FALSE;
	FT_STATUS status;
	DWORD libftd2xx = 0;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	__try
	{
		status = FT_GetLibraryVersion(&libftd2xx);
		if (FT_SUCCESS(status))
		{
			kprintf(L"D2XX %hx.%hhx.%hhx\n", (WORD)(libftd2xx >> 16), (BYTE)(libftd2xx >> 8), (BYTE)(libftd2xx));

			status = FT_Open(0, &Communicator->hCom);
			if (FT_SUCCESS(status))
			{
				ret = TRUE;
			}
			else PRINT_FT_ERROR(L"FT_Open", status);
		}
		else PRINT_FT_ERROR(L"FT_GetLibraryVersion", status);
	}
	__except (GetExceptionCode() == ERROR_DLL_NOT_FOUND)
	{
		PRINT_ERROR(L"No FTDI library available\n");
	}

	return ret;
}

BOOL FTDI_Setup(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = FT_SetChars(Communicator->hCom, 0x00, 0, 0x00, 0);
	if (FT_SUCCESS(status))
	{
		status = FT_SetDataCharacteristics(Communicator->hCom, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_EVEN);
		if (FT_SUCCESS(status))
		{
			status = FT_SetFlowControl(Communicator->hCom, FT_FLOW_NONE, 0xff, 0xff);
			if (FT_SUCCESS(status))
			{
				status = FT_SetBaudRate(Communicator->hCom, FT_BAUD_9600);
				if (FT_SUCCESS(status))
				{
					status = FT_SetTimeouts(Communicator->hCom, 0, 0); //
					if (FT_SUCCESS(status))
					{
						status = FT_Purge(Communicator->hCom, FT_PURGE_RX | FT_PURGE_TX);
						if (FT_SUCCESS(status))
						{
							COM_IO_RESET(Communicator, 0);
							COM_IO_TEST(Communicator, 0);

							ret = TRUE;
						}
						else PRINT_FT_ERROR(L"FT_Purge", status);
					}
					else PRINT_FT_ERROR(L"FT_SetTimeouts", status);
				}
				else PRINT_FT_ERROR(L"FT_SetBaudRate", status);
			}
			else PRINT_FT_ERROR(L"FT_SetFlowControl", status);
		}
		else PRINT_FT_ERROR(L"FT_SetDataCharacteristics", status);
	}
	else PRINT_FT_ERROR(L"FT_SetChars", status);

	return ret;
}

BOOL FTDI_SetBaudrate(PGENERIC_COMMUNICATOR Communicator, DWORD Baudrate)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = FT_SetBaudRate(Communicator->hCom, Baudrate);
	if (FT_SUCCESS(status))
	{
		ret = TRUE;
	}
	else PRINT_FT_ERROR(L"FT_SetBaudRate", status);

	return ret;
}

BOOL FTDI_IO_RESET(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = bValue ? FT_ClrDtr(Communicator->hCom) : FT_SetDtr(Communicator->hCom);
	if (FT_SUCCESS(status))
	{
		ret = TRUE;
	}
	else PRINT_FT_ERROR(L"FT_ClrDtr/FT_SetDtr", status);

	return ret;
}

BOOL FTDI_IO_TEST(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = bValue ? FT_ClrRts(Communicator->hCom) : FT_SetRts(Communicator->hCom);
	if (FT_SUCCESS(status))
	{
		ret = TRUE;
	}
	else PRINT_FT_ERROR(L"FT_ClrRts/FT_SetRts", status);

	return ret;
}

BOOL FTDI_Send(PGENERIC_COMMUNICATOR Communicator, LPCVOID lpBuffer, DWORD cbToSend)
{
	BOOL ret = FALSE;
	FT_STATUS status;
	DWORD NumberOfBytesWritten;

	status = FT_Write(Communicator->hCom, (LPVOID)lpBuffer, cbToSend, &NumberOfBytesWritten);
	if (FT_SUCCESS(status))
	{
		if (NumberOfBytesWritten == cbToSend)
		{
			ret = TRUE;
		}
		else PRINT_ERROR(L"NumberOfBytesWritten: %u (wanted %u)\n", NumberOfBytesWritten, cbToSend);
	}
	else PRINT_FT_ERROR(L"FT_Write", status);

	return ret;
}
BOOL FTDI_Recv(PGENERIC_COMMUNICATOR Communicator, LPVOID lpBuffer, DWORD cbToRead)
{
	BOOL ret = FALSE;
	FT_STATUS status;
	DWORD NumberOfBytesRead;

	status = FT_Read(Communicator->hCom, (LPVOID)lpBuffer, cbToRead, &NumberOfBytesRead);
	if (FT_SUCCESS(status))
	{
		if (NumberOfBytesRead == cbToRead)
		{
			ret = TRUE;
		}
		else PRINT_ERROR(L"NumberOfBytesRead: %u (wanted %hu)\n", NumberOfBytesRead, cbToRead);
	}
	else PRINT_FT_ERROR(L"FT_Read", status);

	return ret;
}

BOOL FTDI_Close(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = FT_Close(Communicator->hCom);
	if (FT_SUCCESS(status))
	{
		ret = TRUE;
	}
	else PRINT_FT_ERROR(L"FT_Close", status);

	return ret;
}

/*  Specific for FT234XD, no DTR# pin, but a CBUS0 (C0) available
 *  When configured as GPIO with FT_Prog, it can be used with Bitmode CBUS Bit Bang to generate RESET signal as DTR# does.
 */
BOOL FT234XD_IO_RESET(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = FT_SetBitMode(Communicator->hCom, CBUS_BIT(0, bValue), FT_BITMODE_CBUS_BITBANG);
	if (FT_SUCCESS(status))
	{
		ret = TRUE;
	}
	else PRINT_FT_ERROR(L"FT_SetBitMode", status);

	return ret;
}

const GENERIC_COM COM_FTDI = {
	.Name = L"ftdi",
	.Open = FTDI_Open,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FTDI_IO_RESET,
	.IoTest = FTDI_IO_TEST,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

const GENERIC_COM COM_FT234XD = {
	.Name = L"ft234xd",
	.Open = FTDI_Open,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FT234XD_IO_RESET,
	.IoTest = FTDI_IO_TEST,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

typedef struct _DUAL_STRING_FT_STATUS {
	PCSTR name;
	FT_STATUS status;
} DUAL_STRING_FT_STATUS, * PDUAL_STRING_FT_STATUS;

const PCSTR FT_STATUS_UNK = "FT_?";
const DUAL_STRING_FT_STATUS FT_STATUS_MESSAGES[] = {
	{"FT_OK",							FT_OK},
	{"FT_INVALID_HANDLE",				FT_INVALID_HANDLE},
	{"FT_DEVICE_NOT_FOUND",				FT_DEVICE_NOT_FOUND},
	{"FT_DEVICE_NOT_OPENED",			FT_DEVICE_NOT_OPENED},
	{"FT_IO_ERROR",						FT_IO_ERROR},
	{"FT_INSUFFICIENT_RESOURCES",		FT_INSUFFICIENT_RESOURCES},
	{"FT_INVALID_PARAMETER",			FT_INVALID_PARAMETER},
	{"FT_INVALID_BAUD_RATE",			FT_INVALID_BAUD_RATE},

	{"FT_DEVICE_NOT_OPENED_FOR_ERASE",	FT_DEVICE_NOT_OPENED_FOR_ERASE},
	{"FT_DEVICE_NOT_OPENED_FOR_WRITE",	FT_DEVICE_NOT_OPENED_FOR_WRITE},
	{"FT_FAILED_TO_WRITE_DEVICE",		FT_FAILED_TO_WRITE_DEVICE},

	{"FT_EEPROM_READ_FAILED",			FT_EEPROM_READ_FAILED},
	{"FT_EEPROM_WRITE_FAILED",			FT_EEPROM_WRITE_FAILED},
	{"FT_EEPROM_ERASE_FAILED",			FT_EEPROM_ERASE_FAILED},
	{"FT_EEPROM_NOT_PRESENT",			FT_EEPROM_NOT_PRESENT},
	{"FT_EEPROM_NOT_PROGRAMMED",		FT_EEPROM_NOT_PROGRAMMED},

	{"FT_INVALID_ARGS",					FT_INVALID_ARGS},
	{"FT_NOT_SUPPORTED",				FT_NOT_SUPPORTED},
	{"FT_OTHER_ERROR",					FT_OTHER_ERROR},
	{"FT_DEVICE_LIST_NOT_READY",		FT_DEVICE_LIST_NOT_READY},
};

PCSTR FT_STATUS_to_NAME(FT_STATUS status)
{
	DWORD i;
	PCSTR ret = FT_STATUS_UNK;

	for (i = 0; i < ARRAYSIZE(FT_STATUS_MESSAGES); i++)
	{
		if (FT_STATUS_MESSAGES[i].status == status)
		{
			ret = FT_STATUS_MESSAGES[i].name + 3;
			break;
		}
	}

	return ret;
}