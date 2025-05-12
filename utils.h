/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#pragma once
#include <windows.h>
#include <stdio.h>

#define kprintf	wprintf
#define kprintf_level(subject, ...)	kprintf(L"%*s" subject, level << 1, L"", __VA_ARGS__)

#define kprinthex(lpData, cbData) (kull_m_cli_kprinthex(lpData, (DWORD) cbData, KPrintHexShort, 0, TRUE))
#define kprinthex16(lpData, cbData) (kull_m_cli_kprinthex(lpData, (DWORD) cbData, KPrintHexSpace, 16, TRUE))

#define PRINT_ERROR(...) (kprintf(L"ERROR " TEXT(__FUNCTION__) L" ; " __VA_ARGS__))
#define PRINT_ERROR_NUMBER(func, error) (kull_m_cli_DisplayError(__FUNCTION__, func, error, TRUE, TRUE))
#define PRINT_ERROR_AUTO(func) (kull_m_cli_DisplayError(__FUNCTION__, func, 0, TRUE, TRUE))
#define PRINT_ERROR_AUTO_NONL(func) (kull_m_cli_DisplayError(__FUNCTION__, func, 0, TRUE, FALSE))
#define W00T(...) (kprintf(TEXT(__FUNCTION__) L" w00t! ; " __VA_ARGS__))

#define GET_CLI_ARG(name, var) (kull_m_cli_args_byName(argc, argv, name, var, NULL))
#define GET_CLI_ARG_DEF(name, var, def) (kull_m_cli_args_byName(argc, argv, name, var, def))
#define GET_CLI_ARG_PRESENT(name) (kull_m_cli_args_byName(argc, argv, name, NULL, NULL))

typedef enum _KPRINT_HEX_TYPES {
	KPrintHexShort = 0,
	KPrintHexSpace = 1,
	KPrintHexC = 2,
	KPrintHexPython = 3,
	KPrintHexShortCap = 4,
} KPRINT_HEX_TYPES, * PKPRINT_HEX_TYPES;

BOOL kull_m_cli_args_byName(const int argc, const wchar_t* argv[], const wchar_t* name, const wchar_t** theArgs, const wchar_t* defaultValue);
void kull_m_cli_kprinthex(LPCVOID lpData, DWORD cbData, KPRINT_HEX_TYPES type, DWORD cbWidth, BOOL bWithNewLine);
void kull_m_cli_DisplayError(PCSTR SourceFunction, PCWSTR SourceError, DWORD dwErrorCode, BOOL bWithMessage, BOOL bWithNewLine);

#define kull_m_file_isFileExist(lpFileName)	(GetFileAttributes(lpFileName) != INVALID_FILE_ATTRIBUTES)
#define kull_m_file_writeData(szFileName, pbData, cbData)	kull_m_file_writeGeneric(szFileName, pbData, cbData, 0)
#define kull_m_file_readData(szFileName, ppbData, pcbData)	kull_m_file_readGeneric(szFileName, ppbData, pcbData, 0)

BOOL kull_m_file_readGeneric(PCWSTR szFileName, PBYTE* ppbData, DWORD* pcbData, DWORD dwFlags);
BOOL kull_m_file_writeGeneric(PCWSTR szFileName, LPCVOID pbData, DWORD cbData, DWORD dwFlags);

BOOL kull_m_string_sprintf(PWSTR* outBuffer, PCWSTR format, ...);