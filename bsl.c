/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#include "bsl.h"
#include "utils.h"

#define BSL_CMD_RX_DATA_BLOCK				(0x10)
#define BSL_CMD_RX_PASSWORD					(0x11)
#define BSL_CMD_MASS_ERASE					(0x15)
#define BSL_CMD_CRC_CHECK					(0x16)
#define BSL_CMD_LOAD_PC						(0x17)
#define BSL_CMD_TX_DATA_BLOCK				(0x18)
#define BSL_CMD_TX_BSL_VERSION				(0x19)
#define BSL_CMD_RX_DATA_BLOCK_FAST			(0x1b)
#define BSL_CMD_CHANGE_BAUDRATE				(0x52)

#define BSL_CMD_DATA						(0x3a)
#define BSL_CMD_MESSAGE						(0x3b)

#define BSL_ACK								(0x00)
#define BSL_ACK_INCORRECT_HEADER			(0x51)
#define BSL_ACK_INCORRECT_CHEKSUM			(0x52)
#define BSL_ACK_PACKET_SIZE_ZERO			(0x53)
#define BSL_ACK_PACKET_SIZE_EXCEEDS_BUFFER	(0x54)
#define BSL_ACK_UNKNOWN_ERROR				(0x55)
#define BSL_ACK_UNKNOWN_BAUDRATE			(0x56)
#define BSL_ACK_PACKET_SIZE_ERROR			(0x57)

#define BSL_MSG_SUCCESSFUL					(0x00)
#define BSL_MSG_MEMORY_WRITE_CHECK_FAILED	(0x01)
#define BSL_MSG_LOCKED						(0x04)
#define BSL_MSG_PASSWORD_ERROR				(0x05)
#define BSL_MSG_UNKNOWN_COMMAND				(0x07)


#define BSL_HEADER							(0x80)

#define BSL_CORE_MAX_SZ_COMMAND	(1)
#define BSL_CORE_MAX_SZ_ADDRESS	(3)
#define BSL_CORE_MAX_SZ_DATA	(256)

#define BSL_CORE_MAX_SZ			(BSL_CORE_MAX_SZ_COMMAND + BSL_CORE_MAX_SZ_ADDRESS + BSL_CORE_MAX_SZ_DATA)

#define BSL_MAX_SZ_HEADER		(1)
#define BSL_MAX_SZ_LENGTH		(2)
#define BSL_MAX_SZ_CHECKSUM		(2)

#define BSL_MAX_SZ				(BSL_MAX_SZ_HEADER + BSL_MAX_SZ_LENGTH + BSL_CORE_MAX_SZ + BSL_MAX_SZ_CHECKSUM)

WORD BSL_CalcCRC16(const BYTE* pcbData, DWORD cbData)
{
	DWORD i;
	WORD crc = 0xffff;
	BYTE j, Data;

	for (i = 0; i < cbData; i++)
	{
		Data = pcbData[i];
		for (j = 0; j < 8; j++) {
			if (((crc & 0x8000) >> 8) ^ (Data & 0x80))
				crc = (crc << 1) ^ 0x1021;
			else
				crc = (crc << 1);
			Data <<= 1;
		}
	}

	return crc;
}

void BSL_Invocation(GENERIC_COMMUNICATOR* pCommunicator)
{
	COM_IO_TEST(pCommunicator, 1); // SET_TEST
	Sleep(6);
	COM_IO_TEST(pCommunicator, 0); // CLR_TEST
	Sleep(6);
	COM_IO_TEST(pCommunicator, 1); // SET_TEST
	Sleep(3);
	COM_IO_RESET(pCommunicator, 1); // SET_RST
	Sleep(1);
	COM_IO_TEST(pCommunicator, 0); // CLR_TEST
	Sleep(10);
}

void BSL_Reset(GENERIC_COMMUNICATOR* pCommunicator)
{
	COM_IO_RESET(pCommunicator, 0); // CLR_RST
	Sleep(20);
	COM_IO_RESET(pCommunicator, 1); // SET_RST
}

BYTE BSL_Buffer[BSL_MAX_SZ];
WORD BSL_Buffer_Count;
#define BSL_Buffer_Core	(BSL_Buffer + BSL_MAX_SZ_HEADER + BSL_MAX_SZ_LENGTH)

BOOL BSL_Internal_Send(GENERIC_COMMUNICATOR* pCommunicator)
{
	BOOL ret = FALSE;

	BSL_Buffer[0] = BSL_HEADER;
	*(WORD*)(BSL_Buffer + BSL_MAX_SZ_HEADER) = BSL_Buffer_Count;
	*(WORD*)(BSL_Buffer_Core + BSL_Buffer_Count) = BSL_CalcCRC16(BSL_Buffer_Core, BSL_Buffer_Count);
	BSL_Buffer_Count += BSL_MAX_SZ_HEADER + BSL_MAX_SZ_LENGTH + BSL_MAX_SZ_CHECKSUM;
#if defined(BSL_VERBOSE_OUTPUT)	
	kprintf(L"> ");
	kprinthex(BSL_Buffer, BSL_Buffer_Count);
#endif
	if(COM_SEND(pCommunicator, BSL_Buffer, BSL_Buffer_Count))
	{
		ret = TRUE;
	}

	return ret;
}

BOOL BSL_Internal_Recv(GENERIC_COMMUNICATOR* pCommunicator, BYTE CmdInResponseWanted)
{
	BOOL ret = FALSE;
	WORD Size = 0, CRC_Calculated, CRC_Expected;

	if (COM_RECV(pCommunicator, BSL_Buffer, 1))
	{
		if (BSL_Buffer[0] == BSL_ACK)
		{
#if defined(BSL_VERBOSE_OUTPUT)	
			kprintf(L"< ACK\n");
#endif
			if (CmdInResponseWanted)
			{
				if (COM_RECV(pCommunicator, BSL_Buffer, BSL_MAX_SZ_HEADER + BSL_MAX_SZ_LENGTH + BSL_MAX_SZ_CHECKSUM))
				{
					if (BSL_Buffer[0] == BSL_HEADER)
					{
						Size = *(WORD*)(BSL_Buffer + BSL_MAX_SZ_HEADER);
						if (Size && (Size <= BSL_CORE_MAX_SZ))
						{
							if (COM_RECV(pCommunicator, BSL_Buffer_Core + BSL_MAX_SZ_CHECKSUM, Size))
							{
								BSL_Buffer_Count = BSL_MAX_SZ_HEADER + BSL_MAX_SZ_LENGTH + Size + BSL_MAX_SZ_CHECKSUM;
#if defined(BSL_VERBOSE_OUTPUT)	
								kprintf(L"< ");
								kprinthex(BSL_Buffer, BSL_Buffer_Count);
#endif
								CRC_Expected = *(WORD*)(BSL_Buffer_Core + Size);
								CRC_Calculated = BSL_CalcCRC16(BSL_Buffer_Core, Size);

								if (CRC_Expected == CRC_Calculated)
								{
									if (BSL_Buffer_Core[0] == CmdInResponseWanted)
									{
										BSL_Buffer_Count -= BSL_MAX_SZ_HEADER + BSL_MAX_SZ_LENGTH + BSL_MAX_SZ_CHECKSUM;
#if defined(BSL_VERBOSE_OUTPUT)							
										kprintf(L"<<");
										kprinthex(BSL_Buffer_Core, BSL_Buffer_Count);
#endif
										ret = TRUE;
									}
									else PRINT_ERROR(L"Expected CMD: 0x%02hhx (wanted: 0x%02hhx)\n", CmdInResponseWanted, BSL_Buffer_Core[0]);
								}
								else PRINT_ERROR(L"Expected CRC: 0x%04hx - Calculated CRC: 0x%04hx\n", CRC_Expected, CRC_Calculated);
							}
						}
						else PRINT_ERROR(L"Size: %hu (max is %hu)\n", Size, BSL_CORE_MAX_SZ);
					}
					else PRINT_ERROR(L"Bad BSL_HEADER: 0x%08hhx (wanted 0x%08hhx)\n", BSL_Buffer[0], BSL_HEADER);
				}
			}
			else
			{
				ret = TRUE;
			}
		}
		else PRINT_ERROR(L"ACK: 0x%08hhx\n", BSL_Buffer[0]);
	}

	return ret;
}

BOOL BSL_Internal_SendAndRecv(GENERIC_COMMUNICATOR* pCommunicator, BYTE CmdInResponseWanted)
{
	BOOL ret = FALSE;

	if (BSL_Internal_Send(pCommunicator))
	{
		if (BSL_Internal_Recv(pCommunicator, CmdInResponseWanted))
		{
			if (CmdInResponseWanted == BSL_CMD_MESSAGE)
			{
				if (BSL_Buffer_Count > 1)
				{
					if (BSL_Buffer_Core[1] == BSL_MSG_SUCCESSFUL)
					{
						ret = TRUE;
					}
					else PRINT_ERROR(L"MESSAGE response is: 0x%08hhx (wanted 0x%08hhx)\n", BSL_Buffer_Core[0], BSL_MSG_SUCCESSFUL);
				}
				else PRINT_ERROR(L"MESSAGE BSL_Buffer_Count: %hu (wanted >1)\n", BSL_Buffer_Count);
			}
			else
			{
				ret = TRUE;
			}
		}
	}


	return ret;
}

BOOL BSL_Mass_Erase(GENERIC_COMMUNICATOR* pCommunicator)
{
	BSL_Buffer_Core[0] = BSL_CMD_MASS_ERASE;
	BSL_Buffer_Count = 1;
	return BSL_Internal_SendAndRecv(pCommunicator, BSL_CMD_MESSAGE);
}

const BYTE BSL_Password_DEFAULT[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };
BOOL BSL_Password(GENERIC_COMMUNICATOR* pCommunicator, const BYTE Password[32])
{
	BSL_Buffer_Core[0] = BSL_CMD_RX_PASSWORD;
	memcpy(BSL_Buffer_Core + 1, Password ? Password : BSL_Password_DEFAULT, 32);
	BSL_Buffer_Count = 1 + 32;

	return BSL_Internal_SendAndRecv(pCommunicator, BSL_CMD_MESSAGE);
}

BOOL BSL_Version(GENERIC_COMMUNICATOR* pCommunicator, BYTE* pBSLVendor, BYTE* pCommandInterpreter, BYTE* pAPI, BYTE* pPeripheralInterface)
{
	BOOL ret = FALSE;

	BSL_Buffer_Core[0] = BSL_CMD_TX_BSL_VERSION;
	BSL_Buffer_Count = 1;

	if (BSL_Internal_SendAndRecv(pCommunicator, BSL_CMD_DATA))
	{
		if (BSL_Buffer_Count == (1 + 4))
		{
			if (pBSLVendor)
			{
				*pBSLVendor = BSL_Buffer_Core[1];
			}

			if (pCommandInterpreter)
			{
				*pCommandInterpreter = BSL_Buffer_Core[2];
			}

			if (pAPI)
			{
				*pAPI = BSL_Buffer_Core[3];
			}

			if (pPeripheralInterface)
			{
				*pPeripheralInterface = BSL_Buffer_Core[4];
			}

			ret = TRUE;

		}
		else PRINT_ERROR(L"DATA BSL_Buffer_Count: %hu (wanted 1 + 4)\n", BSL_Buffer_Count);
	}

	return ret;
}

BOOL BSL_Baudrate(GENERIC_COMMUNICATOR* pCommunicator, BSL_BAUDRATE BaudRate)
{
	BOOL ret = FALSE;
	DWORD TargetBaudRate = 0;

	BSL_Buffer_Core[0] = BSL_CMD_CHANGE_BAUDRATE;
	BSL_Buffer_Core[1] = BaudRate;
	BSL_Buffer_Count = 2;

	if (BSL_Internal_SendAndRecv(pCommunicator, 0))
	{
		switch (BaudRate)
		{
		case BSL_BAUDRATE_9600:
			TargetBaudRate = CBR_9600;
			break;
		case BSL_BAUDRATE_19200:
			TargetBaudRate = CBR_19200;
			break;
		case BSL_BAUDRATE_38400:
			TargetBaudRate = CBR_38400;
			break;
		case BSL_BAUDRATE_57600:
			TargetBaudRate = CBR_57600;
			break;
		case BSL_BAUDRATE_115200:
			TargetBaudRate = CBR_115200;
			break;
		}

		if (COM_SETBAUDRATE(pCommunicator, TargetBaudRate))
		{
			ret = TRUE;
		}
	}

	return ret;
}

BOOL BSL_Rx_Data_Block(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr, const BYTE* pbData, WORD cbData)
{
	BOOL ret = FALSE;

	if (cbData <= 256)
	{
		BSL_Buffer_Core[0] = BSL_CMD_RX_DATA_BLOCK;
		*(ULONG_PTR*)(BSL_Buffer_Core + 1) = Addr; // we don't care about last byte
		memcpy(BSL_Buffer_Core + 4, pbData, cbData);
		BSL_Buffer_Count = 1 + 3 + cbData;

		ret = BSL_Internal_SendAndRecv(pCommunicator, BSL_CMD_MESSAGE);
	}
	else PRINT_ERROR("cbData is %hu (max is 256)\n", cbData);

	return ret;
}

BOOL BSL_Load_PC(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr)
{
	BSL_Buffer_Core[0] = BSL_CMD_LOAD_PC;
	*(ULONG_PTR*)(BSL_Buffer_Core + 1) = Addr; // we don't care about last byte
	BSL_Buffer_Count = 1 + 3;

	return BSL_Internal_SendAndRecv(pCommunicator, 0);
}

BOOL BSL_CRC_Check(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr, WORD Length, WORD* pCRC)
{
	BOOL ret = FALSE;

	BSL_Buffer_Core[0] = BSL_CMD_CRC_CHECK;
	*(ULONG_PTR*)(BSL_Buffer_Core + 1) = Addr; // we don't care about last byte
	*(WORD*)(BSL_Buffer_Core + 4) = Length;
	BSL_Buffer_Count = 1 + 3 + 2;

	if (BSL_Internal_SendAndRecv(pCommunicator, BSL_CMD_DATA))
	{
		if (BSL_Buffer_Count == (1 + 2))
		{
			if (pCRC)
			{
				*pCRC = *(WORD*)(BSL_Buffer_Core + 1);
			}
			ret = TRUE;
		}
		else PRINT_ERROR(L"DATA BSL_Buffer_Count: %hu (wanted 1 + 2)\n", BSL_Buffer_Count);
	}

	return ret;
}

BOOL BSL_Rx_Data_Block_HELPER(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr, const BYTE* pbData, DWORD cbData)
{
	BOOL ret = TRUE;
	DWORD i;

	for (i = 0; (i + 256) < cbData; i += 256)
	{
		BSL_Rx_Data_Block(pCommunicator, Addr + i, pbData + i, 256);
		kprintf(L".");
	}

	if (cbData - i)
	{
		BSL_Rx_Data_Block(pCommunicator, Addr + i, pbData + i, (WORD)(cbData - i));
		kprintf(L".");
	}

	return ret;
}