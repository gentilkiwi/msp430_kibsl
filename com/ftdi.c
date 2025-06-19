#include "generic.h"
#include "ftd2xx.h"

#define PRINT_FT_ERROR(func, status)	kprintf(L"ERROR " TEXT(__FUNCTION__) L" ; " func L" : %S (%lu)\n", FT_STATUS_to_NAME(status), status)
PCSTR FT_STATUS_to_NAME(FT_STATUS status);
PCSTR FT_X_SERIES_CBUS_to_NAME(UCHAR value);

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
					kprintf(L"| Port available @ ");
					if (ComPortNumber > 0)
					{
						kprintf(L"COM%i\n", ComPortNumber);
					}
					else
					{
						kprintf(L"D2XX Direct\n");
					}
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

BOOL FTDI_DTR_IO_RESET(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
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

BOOL FTDI_RTS_IO_TEST(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
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

#define CBUS_BITN(n)					(1 << (4 + (n)))
#define CBUS_BITV(n, v)					((v) << (n))
#define CBUS_BIT(n, v)					(CBUS_BITN(n) | CBUS_BITV(n, v))

BOOL FTDI_X_CBx(PGENERIC_COMMUNICATOR Communicator, BYTE x, BYTE bValue)
{
	BOOL ret = FALSE;
	FT_STATUS status;

	if (Communicator->Misc & CBUS_BITN(x))
	{
		if (bValue)
		{
			Communicator->Misc |= CBUS_BITV(x, 1);
		}
		else
		{
			Communicator->Misc &= ~CBUS_BITV(x, 1);;
		}

		status = FT_SetBitMode(Communicator->hCom, Communicator->Misc & 0xff, FT_BITMODE_CBUS_BITBANG);
		if (FT_SUCCESS(status))
		{
			ret = TRUE;
		}
		else PRINT_FT_ERROR(L"FT_SetBitMode", status);
	}
	else PRINT_ERROR("CBUS %hhu was not configured in Misc\n", x);

	return ret;
}

BOOL FTDI_X_CB0(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	return FTDI_X_CBx(Communicator, 0, bValue);
}

BOOL FTDI_X_CB1(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	return FTDI_X_CBx(Communicator, 1, bValue);
}

BOOL FTDI_X_CB2(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	return FTDI_X_CBx(Communicator, 2, bValue);
}

BOOL FTDI_X_CB3(PGENERIC_COMMUNICATOR Communicator, BYTE bValue)
{
	return FTDI_X_CBx(Communicator, 3, bValue);
}

BOOL FTDI_X_CBUS_Config(PGENERIC_COMMUNICATOR Communicator, const BYTE NeededField, const BYTE Configs[4])
{
	BOOL ret = FALSE;
	FT_STATUS status;
	FT_EEPROM_X_SERIES ft_eeprom_x_series;
	char Manufacturer[64] = { 0 }, ManufacturerId[64] = { 0 }, Description[64] = { 0 }, SerialNumber[64] = { 0 };
	BYTE i, *pValue;
	
	ft_eeprom_x_series.common.deviceType = FT_DEVICE_X_SERIES;
	status = FT_EEPROM_Read(Communicator->hCom, &ft_eeprom_x_series, sizeof(ft_eeprom_x_series), Manufacturer, ManufacturerId, Description, SerialNumber);
	if (FT_SUCCESS(status))
	{
		kprintf(L"| Manufacturer: %S (%S)\n| Description : %S\n| SerialNumber: %S\n\n| CBUS configuration :\n", Manufacturer, ManufacturerId, Description, SerialNumber);

		ret = TRUE;
		for (i = 0; i < 4; i++)
		{
			if(CBUS_BITN(i) & NeededField)
			{
				pValue = &ft_eeprom_x_series.Cbus0 + i;
				kprintf(L"  CBUS%hhu: 0x%02hhx (%S) - ", i, *pValue, FT_X_SERIES_CBUS_to_NAME(*pValue));
				if (*pValue == Configs[i])
				{
					kprintf(L"OK\n");
				}
				else
				{
					kprintf(L"KO, needed: 0x%02hhx (%S) -- will try to adjust it\n", Configs[i], FT_X_SERIES_CBUS_to_NAME(Configs[i]));
					*pValue = Configs[i];
					ret = FALSE;
				}
			}
		}

		if (!ret)
		{
			kprintf(L"| EEPROM program  : ");
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
								kprintf(L"| CBUS configuration :\n");
								ret = TRUE;
								for (i = 0; i < 4; i++)
								{
									if (CBUS_BITN(i) & NeededField)
									{
										pValue = &ft_eeprom_x_series.Cbus0 + i;
										kprintf(L"  CBUS%hhu: 0x%02hhx (%S) - ", i, *pValue, FT_X_SERIES_CBUS_to_NAME(*pValue));
										if (*pValue == Configs[i])
										{
											kprintf(L"OK\n");
										}
										else
										{
											kprintf(L"KO\n");
											ret = FALSE;
										}
									}
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

	if (ret)
	{
		Communicator->Misc = 0;
		for (i = 0; i < 4; i++)
		{
			if ((CBUS_BITN(i) & NeededField) && (Configs[i] == FT_X_SERIES_CBUS_IOMODE))
			{
				Communicator->Misc |= CBUS_BIT(i, 0);
			}
		}
	}

	return ret;
}

const GENERIC_COM COM_FTDI = {
	.Name = L"ftdi",
	.Open = FTDI_Open,
	.Config = NULL,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FTDI_DTR_IO_RESET,
	.IoTest = FTDI_RTS_IO_TEST,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

const BYTE FT234XD_Configs[4] = { FT_X_SERIES_CBUS_IOMODE };
BOOL FT234XD_CBUS0_Config(PGENERIC_COMMUNICATOR Communicator)
{
	return FTDI_X_CBUS_Config(Communicator, CBUS_BITN(0), FT234XD_Configs);
}

const GENERIC_COM COM_FT234XD = {
	.Name = L"ft234xd",
	.Open = FTDI_Open,
	.Config = FT234XD_CBUS0_Config,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FTDI_X_CB0,
	.IoTest = FTDI_RTS_IO_TEST,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

const BYTE ST25TB_KIWI_FT230XD_Configs[4] = { FT_X_SERIES_CBUS_IOMODE, FT_X_SERIES_CBUS_TXLED, FT_X_SERIES_CBUS_RXLED, FT_X_SERIES_CBUS_IOMODE };
BOOL ST25TB_KIWI_FT230XD_CBUS_Config(PGENERIC_COMMUNICATOR Communicator)
{
	return FTDI_X_CBUS_Config(Communicator, CBUS_BITN(0) | CBUS_BITN(1) | CBUS_BITN(2) | CBUS_BITN(3), ST25TB_KIWI_FT230XD_Configs);
}

const GENERIC_COM COM_ST25TB_KIWI = {
	.Name = L"st25tb_kiwi",
	.Open = FTDI_Open,
	.Config = ST25TB_KIWI_FT230XD_CBUS_Config,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FTDI_X_CB0,
	.IoTest = FTDI_X_CB3,
	.Send = FTDI_Send,
	.Recv = FTDI_Recv,
	.Close = FTDI_Close,
};

const BYTE UMFT230XB_Configs[4] = { FT_X_SERIES_CBUS_IOMODE , FT_X_SERIES_CBUS_IOMODE };
BOOL UMFT230XB_CBUS_Config(PGENERIC_COMMUNICATOR Communicator)
{
	return FTDI_X_CBUS_Config(Communicator, CBUS_BITN(0) | CBUS_BITN(1), UMFT230XB_Configs);
}

const GENERIC_COM COM_UMFT230XB = {
	.Name = L"umft230xb",
	.Open = FTDI_Open,
	.Config = UMFT230XB_CBUS_Config,
	.Setup = FTDI_Setup,
	.SetBaudrate = FTDI_SetBaudrate,
	.IoReset = FTDI_X_CB0,
	.IoTest = FTDI_X_CB1,
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