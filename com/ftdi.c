#include "generic.h"
#include "ftd2xx.h"

#define CBUS_BIT(n, v)					((1 << (4 + (n))) | ((v) << (n)))
#define PRINT_FT_ERROR(func, status)	kprintf(L"ERROR " TEXT(__FUNCTION__) L" ; " func L" : %S (%lu)\n", FT_STATUS_to_NAME(status), status)
PCSTR FT_STATUS_to_NAME(FT_STATUS status);
PCSTR FT_X_SERIES_CBUS_to_NAME(UCHAR value);
BOOL FT234XD_CBUS0_Config(PGENERIC_COMMUNICATOR Communicator);
BOOL KIWI_CBUS_Config(PGENERIC_COMMUNICATOR Communicator);

BOOL FTDI_Open(PGENERIC_COMMUNICATOR Communicator, int argc, wchar_t* argv[])
{
	BOOL ret = FALSE;
	FT_STATUS status;
	DWORD libftd2xx = 0;
	LONG ComPortNumber;

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
				status = FT_GetComPortNumber(Communicator->hCom, &ComPortNumber);
				if (FT_SUCCESS(status))
				{
					kprintf(L"| Port available @ COM%i\n", ComPortNumber);
				}

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
							Communicator->Misc = 0x00000000;
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

BOOL KIWI_IO_TEST(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	return FTDI_IO_TEST(Communicator, !bValue);
}

BOOL FT230XQ_KIWI_IO_Generic(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	status = FT_SetBitMode(Communicator->hCom, CBUS_BIT(0, Communicator->Misc & 0x00000001) | CBUS_BIT(3, (Communicator->Misc >> 1) & 0x00000001), FT_BITMODE_CBUS_BITBANG);
	if (FT_SUCCESS(status))
	{
		ret = TRUE;
	}
	else PRINT_FT_ERROR(L"FT_SetBitMode", status);

	return ret;
}

BOOL FT230XQ_KIWI_IO_RESET(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	if (bValue)
	{
		Communicator->Misc |= 0x00000001;
	}
	else
	{
		Communicator->Misc &= ~0x00000001;
	}

	return FT230XQ_KIWI_IO_Generic(Communicator);
}

BOOL FT230XQ_KIWI_IO_TEST(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	if (bValue)
	{
		Communicator->Misc |= 0x00000002;
	}
	else
	{
		Communicator->Misc &= ~0x00000002;
	}

	return FT230XQ_KIWI_IO_Generic(Communicator);
}

BOOL FT234XD_CBUS0_Config(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;
	FT_STATUS status;
	FT_EEPROM_X_SERIES ft_eeprom_x_series;
	char Manufacturer[64] = { 0 }, ManufacturerId[64] = { 0 }, Description[64] = { 0 }, SerialNumber[64] = { 0 };

	ft_eeprom_x_series.common.deviceType = FT_DEVICE_X_SERIES;
	status = FT_EEPROM_Read(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
	if (FT_SUCCESS(status))
	{
		kprintf(L"| Manufacturer: %S (%S)\n| Description : %S\n| SerialNumber: %S\n\nCBUS0 config: 0x%02hhx (%S) - ", Manufacturer, ManufacturerId, Description, SerialNumber, ft_eeprom_x_series.Cbus0, FT_X_SERIES_CBUS_to_NAME(ft_eeprom_x_series.Cbus0));
		if (ft_eeprom_x_series.Cbus0 == FT_X_SERIES_CBUS_IOMODE)
		{
			kprintf(L"OK\n");
			ret = TRUE;
		}
		else
		{
			kprintf(L"KO -- will adjust config\n| EEPROM program  : ");
			ft_eeprom_x_series.Cbus0 = FT_X_SERIES_CBUS_IOMODE;
			status = FT_EEPROM_Program(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
			if (FT_SUCCESS(status))
			{
				kprintf(L"OK\n| Cycle port      : ");
				status = FT_CyclePort(Communicator->hCom);
				if (FT_SUCCESS(status))
				{
					kprintf(L"OK\n| Close old handle: ");
					status = FT_Close(Communicator->hCom);
					if (FT_SUCCESS(status))
					{
						kprintf(L"OK\n -- wait 5s ... --\n");
						Sleep(5000);
						kprintf(L"| Re-open device  : ");
						status = FT_Open(0, &Communicator->hCom);
						if (FT_SUCCESS(status))
						{
							kprintf(L"OK\n");
							status = FT_EEPROM_Read(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
							if (FT_SUCCESS(status))
							{
								kprintf(L"\nCBUS0 config: 0x%02hhx (%S) - ", ft_eeprom_x_series.Cbus0, FT_X_SERIES_CBUS_to_NAME(ft_eeprom_x_series.Cbus0));
								if (ft_eeprom_x_series.Cbus0 == FT_X_SERIES_CBUS_IOMODE)
								{
									kprintf(L"OK\n");
									ret = TRUE;
								}
								else
								{
									kprintf(L"KO\n");
								}
							}
							else PRINT_FT_ERROR(L"FT_EEPROM_Read", status);
						}
						else PRINT_FT_ERROR(L"FT_Open", status);
					}
					else PRINT_FT_ERROR(L"FT_Close", status);
				}
				else PRINT_FT_ERROR(L"FT_CyclePort", status);
			}
			else PRINT_FT_ERROR(L"FT_EEPROM_Program", status);
		}
	}
	else PRINT_FT_ERROR(L"FT_EEPROM_Read", status);

	return ret;
}

const GENERIC_COM COM_FTDI = {
	.Name = L"ftdi",
	.Open = FTDI_Open,
	.Config = NULL,
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
	.Config = FT234XD_CBUS0_Config,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FT234XD_IO_RESET,
	.IoTest = FTDI_IO_TEST,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

const GENERIC_COM COM_KIWI_old = {
	.Name = L"kiwi_old",
	.Open = FTDI_Open,
	.Config = FT234XD_CBUS0_Config,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FT234XD_IO_RESET,
	.IoTest = KIWI_IO_TEST,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

void FTDI_CBUS_PrintConfig(PFT_EEPROM_X_SERIES pEeprom, BYTE cb)
{
	BYTE i, v;
	for (i = 0; i < cb; i++)
	{
		v = *(&pEeprom->Cbus0 + i);
		kprintf(L"  CBUS%hhu config: 0x%02hhx (%S)\n", i, v, FT_X_SERIES_CBUS_to_NAME(v));
	}
}

BOOL KIWI_CBUS_Config(PGENERIC_COMMUNICATOR Communicator)
{
	BOOL ret = FALSE;
	FT_STATUS status;
	FT_EEPROM_X_SERIES ft_eeprom_x_series;
	char Manufacturer[64] = { 0 }, ManufacturerId[64] = { 0 }, Description[64] = { 0 }, SerialNumber[64] = { 0 };

	ft_eeprom_x_series.common.deviceType = FT_DEVICE_X_SERIES;
	status = FT_EEPROM_Read(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
	if (FT_SUCCESS(status))
	{
		kprintf(L"| Manufacturer: %S (%S)\n| Description : %S\n| SerialNumber: %S\n\n| CBUS configuration :\n", Manufacturer, ManufacturerId, Description, SerialNumber);
		FTDI_CBUS_PrintConfig(&ft_eeprom_x_series, 4);
		kprintf(L"> CBUS is: ");
		if ((ft_eeprom_x_series.Cbus0 == FT_X_SERIES_CBUS_IOMODE) && (ft_eeprom_x_series.Cbus1 == FT_X_SERIES_CBUS_TXLED) && (ft_eeprom_x_series.Cbus2 == FT_X_SERIES_CBUS_RXLED) && (ft_eeprom_x_series.Cbus3 == FT_X_SERIES_CBUS_IOMODE))
		{
			kprintf(L"OK\n");
			ret = TRUE;
		}
		else
		{
			kprintf(L"KO -- will adjust config\n| EEPROM program  : ");
			ft_eeprom_x_series.Cbus0 = FT_X_SERIES_CBUS_IOMODE;
			ft_eeprom_x_series.Cbus1 = FT_X_SERIES_CBUS_TXLED;
			ft_eeprom_x_series.Cbus2 = FT_X_SERIES_CBUS_RXLED;
			ft_eeprom_x_series.Cbus3 = FT_X_SERIES_CBUS_IOMODE;

			status = FT_EEPROM_Program(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
			if (FT_SUCCESS(status))
			{
				kprintf(L"OK\n| Cycle port      : ");
				status = FT_CyclePort(Communicator->hCom);
				if (FT_SUCCESS(status))
				{
					kprintf(L"OK\n| Close old handle: ");
					status = FT_Close(Communicator->hCom);
					if (FT_SUCCESS(status))
					{
						kprintf(L"OK\n -- wait 5s ... --\n");
						Sleep(5000);
						kprintf(L"| Re-open device  : ");
						status = FT_Open(0, &Communicator->hCom);
						if (FT_SUCCESS(status))
						{
							kprintf(L"OK\n");
							status = FT_EEPROM_Read(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
							if (FT_SUCCESS(status))
							{
								FTDI_CBUS_PrintConfig(&ft_eeprom_x_series, 4);
								kprintf(L"| CBUS configuration : ");
								if ((ft_eeprom_x_series.Cbus0 == FT_X_SERIES_CBUS_IOMODE) && (ft_eeprom_x_series.Cbus1 == FT_X_SERIES_CBUS_TXLED) && (ft_eeprom_x_series.Cbus2 == FT_X_SERIES_CBUS_RXLED) && (ft_eeprom_x_series.Cbus3 == FT_X_SERIES_CBUS_IOMODE))
								{
									kprintf(L"OK\n");
									ret = TRUE;
								}
								else
								{
									kprintf(L"KO\n");
								}
							}
							else PRINT_FT_ERROR(L"FT_EEPROM_Read", status);
						}
						else PRINT_FT_ERROR(L"FT_Open", status);
					}
					else PRINT_FT_ERROR(L"FT_Close", status);
				}
				else PRINT_FT_ERROR(L"FT_CyclePort", status);
			}
			else PRINT_FT_ERROR(L"FT_EEPROM_Program", status);
		}
	}
	else PRINT_FT_ERROR(L"FT_EEPROM_Read", status);

	return ret;
}

const GENERIC_COM COM_KIWI = {
	.Name = L"kiwi",
	.Open = FTDI_Open,
	.Config = KIWI_CBUS_Config,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FT230XQ_KIWI_IO_RESET,
	.IoTest = FT230XQ_KIWI_IO_TEST,
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

typedef struct _DUAL_STRING_FT_X_SERIES_CBUS {
	PCSTR name;
	UCHAR value;
} DUAL_STRING_FT_X_SERIES_CBUS, *PDUAL_STRING_FT_X_SERIES_CBUS;

const PCSTR FT_CBUS_UNK = "FT_X_SERIES_CBUS_?";
const DUAL_STRING_FT_X_SERIES_CBUS FT_X_SERIES_CBUS_MESSAGES[] = {
	{"FT_X_SERIES_CBUS_TRISTATE",		FT_X_SERIES_CBUS_TRISTATE},
	{"FT_X_SERIES_CBUS_TXLED",			FT_X_SERIES_CBUS_TXLED},
	{"FT_X_SERIES_CBUS_RXLED",			FT_X_SERIES_CBUS_RXLED},
	{"FT_X_SERIES_CBUS_TXRXLED",		FT_X_SERIES_CBUS_TXRXLED},
	{"FT_X_SERIES_CBUS_PWREN",			FT_X_SERIES_CBUS_PWREN},
	{"FT_X_SERIES_CBUS_SLEEP",			FT_X_SERIES_CBUS_SLEEP},
	{"FT_X_SERIES_CBUS_DRIVE_0",		FT_X_SERIES_CBUS_DRIVE_0},
	{"FT_X_SERIES_CBUS_DRIVE_1",		FT_X_SERIES_CBUS_DRIVE_1},
	{"FT_X_SERIES_CBUS_IOMODE",			FT_X_SERIES_CBUS_IOMODE},
	{"FT_X_SERIES_CBUS_TXDEN",			FT_X_SERIES_CBUS_TXDEN},
	{"FT_X_SERIES_CBUS_CLK24",			FT_X_SERIES_CBUS_CLK24},
	{"FT_X_SERIES_CBUS_CLK12",			FT_X_SERIES_CBUS_CLK12},
	{"FT_X_SERIES_CBUS_CLK6",			FT_X_SERIES_CBUS_CLK6},
	{"FT_X_SERIES_CBUS_BCD_CHARGER",	FT_X_SERIES_CBUS_BCD_CHARGER},
	{"FT_X_SERIES_CBUS_BCD_CHARGER_N",	FT_X_SERIES_CBUS_BCD_CHARGER_N},
	{"FT_X_SERIES_CBUS_I2C_TXE",		FT_X_SERIES_CBUS_I2C_TXE},
	{"FT_X_SERIES_CBUS_I2C_RXF",		FT_X_SERIES_CBUS_I2C_RXF},
	{"FT_X_SERIES_CBUS_VBUS_SENSE",		FT_X_SERIES_CBUS_VBUS_SENSE},
	{"FT_X_SERIES_CBUS_BITBANG_WR",		FT_X_SERIES_CBUS_BITBANG_WR},
	{"FT_X_SERIES_CBUS_BITBANG_RD",		FT_X_SERIES_CBUS_BITBANG_RD},
	{"FT_X_SERIES_CBUS_TIMESTAMP",		FT_X_SERIES_CBUS_TIMESTAMP},
	{"FT_X_SERIES_CBUS_KEEP_AWAKE",		FT_X_SERIES_CBUS_KEEP_AWAKE},
};

PCSTR FT_X_SERIES_CBUS_to_NAME(UCHAR value)
{
	DWORD i;
	PCSTR ret = FT_CBUS_UNK;

	for (i = 0; i < ARRAYSIZE(FT_X_SERIES_CBUS_MESSAGES); i++)
	{
		if (FT_X_SERIES_CBUS_MESSAGES[i].value == value)
		{
			ret = FT_X_SERIES_CBUS_MESSAGES[i].name + 17;
			break;
		}
	}

	return ret;
}