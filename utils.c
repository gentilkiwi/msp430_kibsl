/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#include "utils.h"

BOOL kull_m_cli_args_byName(const int argc, const wchar_t* argv[], const wchar_t* name, const wchar_t** theArgs, const wchar_t* defaultValue)
{
	BOOL result = FALSE;
	const wchar_t* pArgName, * pSeparator;
	SIZE_T argLen, nameLen = wcslen(name);
	int i;

	for (i = 0; i < argc; i++)
	{
		if (wcslen(argv[i]) && ((argv[i][0] == L'/') || (argv[i][0] == L'-')))
		{
			pArgName = argv[i] + 1;
			pSeparator = wcschr(argv[i], L':');

			if (!pSeparator)
			{
				pSeparator = wcschr(argv[i], L'=');
			}

			if (pSeparator)
			{
				argLen = pSeparator - pArgName;
			}
			else
			{
				argLen = wcslen(pArgName);
			}

			if ((argLen == nameLen) && !_wcsnicmp(name, pArgName, argLen))
			{
				if (theArgs)
				{
					if (pSeparator)
					{
						*theArgs = pSeparator + 1;
						result = *theArgs[0] != L'\0';
					}
				}
				else
				{
					result = TRUE;
				}
				break;
			}
		}
	}

	if (!result && theArgs)
	{
		if (defaultValue)
		{
			*theArgs = defaultValue;
			result = TRUE;
		}
		else
		{
			*theArgs = NULL;
		}
	}

	return result;
}

PCWCHAR KULL_M_CLI_KPRINTHEX_TYPES[] = {
	L"%02x",		// KPrintHexShort
	L"%02x ",		// KPrintHexSpace
	L"0x%02x, ",	// KPrintHexC
	L"\\x%02x",		// KPrintHexPython
	L"%02X",		// KPrintHexShortCap
};

void kull_m_cli_kprinthex(LPCVOID lpData, DWORD cbData, KPRINT_HEX_TYPES type, DWORD cbWidth, BOOL bWithNewLine)
{
	DWORD i;
	PCWCHAR pType = KULL_M_CLI_KPRINTHEX_TYPES[type];

	if (type == KPrintHexC)
	{
		kprintf(L"\nBYTE data[] = {\n\t");
	}

	for (i = 0; i < cbData; i++)
	{
		kprintf(pType, ((LPCBYTE)lpData)[i]);

		if (cbWidth && !((i + 1) % cbWidth))
		{
			kprintf(L"\n");
			if (type == KPrintHexC)
			{
				kprintf(L"\t");
			}
		}
	}

	if (type == KPrintHexC)
	{
		kprintf(L"\n};\n");
	}

	if (bWithNewLine)
	{
		kprintf(L"\n");
	}
}

void kull_m_cli_DisplayError(PCSTR SourceFunction, PCWSTR SourceError, DWORD dwErrorCode, BOOL bWithMessage, BOOL bWithNewLine)
{
	PWSTR szMessage = NULL;

	if (!dwErrorCode)
	{
		dwErrorCode = GetLastError();
	}

	kprintf(L"ERROR %S ; %s: 0x%08x", SourceFunction, SourceError, dwErrorCode);

	if (bWithMessage)
	{
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&szMessage, 0, NULL))
		{
			if (szMessage)
			{
				kprintf(L" - %s", szMessage);
				LocalFree(szMessage);
			}
		}
	}

	if (bWithNewLine)
	{
		kprintf(L"\n");
	}
}

BOOL kull_m_file_readGeneric(PCWSTR szFileName, PBYTE* ppbData, DWORD* pcbData, DWORD dwFlags)
{
	BOOL status = FALSE;
	DWORD dwBytesReaded;
	LARGE_INTEGER filesize;
	HANDLE hFile = NULL;

	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, dwFlags, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (GetFileSizeEx(hFile, &filesize) && !filesize.HighPart)
		{
			if (!filesize.HighPart)
			{
				*pcbData = filesize.LowPart;
				*ppbData = (PBYTE)LocalAlloc(LPTR, *pcbData);
				if (*ppbData)
				{
					if (ReadFile(hFile, *ppbData, *pcbData, &dwBytesReaded, NULL))
					{
						if (*pcbData == dwBytesReaded)
						{
							status = TRUE;
						}
						else PRINT_ERROR(L"Read %u, needed %u\n", dwBytesReaded, *pcbData);
					}
					else PRINT_ERROR_AUTO(L"ReadFile");

					if (!status)
					{
						LocalFree(*ppbData);
						*ppbData = NULL;
						*pcbData = 0;
					}
				}
			}
			else PRINT_ERROR(L"Too big!\n");
		}
		else PRINT_ERROR_AUTO(L"GetFileSizeEx");

		CloseHandle(hFile);
	}
	else PRINT_ERROR_AUTO(L"CreateFile");

	return status;
}

BOOL kull_m_file_writeGeneric(PCWSTR szFileName, LPCVOID pbData, DWORD cbData, DWORD dwFlags)
{
	BOOL status = FALSE;
	DWORD dwBytesWritten;
	HANDLE hFile = NULL;

	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, dwFlags, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (WriteFile(hFile, pbData, cbData, &dwBytesWritten, NULL))
		{
			if (FlushFileBuffers(hFile))
			{
				status = TRUE;
			}
			else PRINT_ERROR_AUTO(L"FlushFileBuffers");
		}
		else PRINT_ERROR_AUTO(L"WriteFile");

		CloseHandle(hFile);
	}
	else PRINT_ERROR_AUTO(L"CreateFile");

	return status;
}

BOOL kull_m_string_sprintf(PWSTR* outBuffer, PCWSTR format, ...)
{
	BOOL status = FALSE;
	int varBuf;
	va_list args;

	va_start(args, format);

	varBuf = _vscwprintf(format, args);
	if (varBuf > 0)
	{
		varBuf++;
		*outBuffer = LocalAlloc(LPTR, varBuf * sizeof(wchar_t));
		if (*outBuffer)
		{
			varBuf = vswprintf_s(*outBuffer, varBuf, format, args);
			if (varBuf > 0)
			{
				status = TRUE;
			}
			else
			{
				*outBuffer = LocalFree(*outBuffer);
			}
		}
	}

	return status;
}