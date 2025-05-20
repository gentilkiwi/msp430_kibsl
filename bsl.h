/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#pragma once
#include <windows.h>
#include "com/generic.h"

//#define BSL_VERBOSE_OUTPUT

typedef enum _BSL_BAUDRATE {
	BSL_BAUDRATE_9600 = 0x02,
	BSL_BAUDRATE_19200 = 0x03,
	BSL_BAUDRATE_38400 = 0x04,
	BSL_BAUDRATE_57600 = 0x05,
	BSL_BAUDRATE_115200 = 0x06,
} BSL_BAUDRATE, * PBSL_BAUDRATE;

WORD BSL_CalcCRC16(const BYTE* pcbData, DWORD cbData);

void BSL_Invocation(GENERIC_COMMUNICATOR* pCommunicator);
void BSL_Reset(GENERIC_COMMUNICATOR* pCommunicator);

BOOL BSL_Mass_Erase(GENERIC_COMMUNICATOR* pCommunicator);
BOOL BSL_Password(GENERIC_COMMUNICATOR* pCommunicator, const BYTE Password[32]);
BOOL BSL_Version(GENERIC_COMMUNICATOR* pCommunicator, BYTE* pBSLVendor, BYTE* pCommandInterpreter, BYTE* pAPI, BYTE* pPeripheralInterface);
BOOL BSL_Baudrate(GENERIC_COMMUNICATOR* pCommunicator, BSL_BAUDRATE Baudrate);
BOOL BSL_Rx_Data_Block(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr, const BYTE* pbData, WORD cbData);
BOOL BSL_Load_PC(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr);
BOOL BSL_CRC_Check(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr, WORD Length, WORD* pCRC);

BOOL BSL_Rx_Data_Block_HELPER(GENERIC_COMMUNICATOR* pCommunicator, ULONG_PTR Addr, const BYTE* pbData, DWORD cbData);