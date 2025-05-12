/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#pragma once
#include <windows.h>

//#define BSL_VERBOSE_OUTPUT

typedef enum _BSL_BAUDRATE {
	BSL_BAUDRATE_9600 = 0x02,
	BSL_BAUDRATE_19200 = 0x03,
	BSL_BAUDRATE_38400 = 0x04,
	BSL_BAUDRATE_57600 = 0x05,
	BSL_BAUDRATE_115200 = 0x06,
} BSL_BAUDRATE, * PBSL_BAUDRATE;

WORD BSL_CalcCRC16(const BYTE* pcbData, DWORD cbData);

HANDLE BSL_InitComPort(LPCWSTR PortName);

void BSL_Invocation(HANDLE hCom);
void BSL_Reset(HANDLE hCom);

BOOL BSL_Mass_Erase(HANDLE hCom);
BOOL BSL_Password(HANDLE hCom, const BYTE Password[32]);
BOOL BSL_Version(HANDLE hCom, BYTE* pBSLVendor, BYTE* pCommandInterpreter, BYTE* pAPI, BYTE* pPeripheralInterface);
BOOL BSL_Baudrate(HANDLE hCom, BSL_BAUDRATE Baudrate);
BOOL BSL_Rx_Data_Block(HANDLE hCom, ULONG_PTR Addr, const BYTE* pbData, WORD cbData);
BOOL BSL_Load_PC(HANDLE hCom, ULONG_PTR Addr);
BOOL BSL_CRC_Check(HANDLE hCom, ULONG_PTR Addr, WORD Length, WORD* pCRC);

BOOL BSL_Rx_Data_Block_HELPER(HANDLE hCom, ULONG_PTR Addr, const BYTE* pbData, DWORD cbData);