/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#include "utils.h"
#include "bsl.h"
#include "elf.h"

int wmain(int argc, wchar_t* argv[])
{
	LPCWSTR szElfFilename, szPortName;
	PELF32 pELF32;
	DWORD cbFile, cbData;
	const ELF32_PROGRAM_HEADER* pProgramHeader;
	WORD i, DataCRC, ChipCRC;
	HANDLE hCom;
	PBYTE pbData;

	kprintf(
		L" USB \xc4\xbf    \xda\xc4 MSP430              msp430_kibsl 0.1\n"
		L" 3.3V \xc3----\xb4 VCC                  | MSP430FR2476 - st25tb_kiemul\n"
		L" DTR# \xc3\xc4\xc4\xc4\xc4\xb4 RST#/NMI/SBWTDIO     | MSP430FR2673 - st25tb_kameleon\n"
		L" RTS# \xc3\xc4\xc4\xc4\xc4\xb4 TEST/SBWTCK          \\ tested on USB Adapter: FT232H\n"
		L"   TX \xc3\xc4\xc4\xc4\xc4\xb4 RX\n"
		L"   RX \xc3\xc4\xc4\xc4\xc4\xb4 TX   /*** Benjamin DELPY `gentilkiwi` ( benjamin@gentilkiwi.com )\n"
		L"  GND \xc3\xc4\xc4\xc4\xc4\xb4 GND       > https://blog.gentilkiwi.com\n"
		L"   \xc4\xc4\xc4\xd9    \xc0\xc4\xc4\xc4    ***/\n\n"
	);

	if (GET_CLI_ARG(L"elf", &szElfFilename))
	{
		if (GET_CLI_ARG(L"port", &szPortName))
		{
			if (kull_m_file_readData(szElfFilename, (PBYTE*)&pELF32, &cbFile))
			{
				pProgramHeader = ELF_Validate_TI_MSP430(pELF32, cbFile);
				if (pProgramHeader)
				{
					hCom = BSL_InitComPort(szPortName);
					if (hCom && (hCom != INVALID_HANDLE_VALUE))
					{
						BSL_Invocation(hCom);
						if (BSL_Baudrate(hCom, BSL_BAUDRATE_115200))
						{
							if (BSL_Mass_Erase(hCom))
							{
								if (BSL_Password(hCom, NULL))
								{
									for (i = 0; i < pELF32->e_phnum; i++)
									{
										kprintf(L"%3hu | ", i);

										if ((pProgramHeader[i].p_type == PT_LOAD) && pProgramHeader[i].p_vaddr && pProgramHeader[i].p_filesz)
										{
											pbData = (PBYTE)pELF32 + pProgramHeader[i].p_offset;
											cbData = pProgramHeader[i].p_filesz;
											DataCRC = BSL_CalcCRC16(pbData, cbData);

											kprintf(L"0x%x - 0x%x (%6u) ", pProgramHeader[i].p_vaddr, pProgramHeader[i].p_vaddr + pProgramHeader[i].p_filesz - 1, pProgramHeader[i].p_filesz);
											if (BSL_Rx_Data_Block_HELPER(hCom, pProgramHeader[i].p_vaddr, pbData, cbData))
											{
												kprintf(L"OK - CRC16 ");
												if (BSL_CRC_Check(hCom, pProgramHeader[i].p_vaddr, (UINT16)cbData, &ChipCRC))
												{
													if (DataCRC == ChipCRC)
													{
														kprintf(L"OK (%04hx)\n", DataCRC);
													}
													else
													{
														kprintf(L"!! KO !! Data %04hx / Chip %04hx\n", DataCRC, ChipCRC);
													}
												}
											}
										}
										else
										{
											kprintf(L"<ignored>\n");
										}
									}
									kprintf(L"Set PC to entry point @ 0x%x ...: %s\n", pELF32->e_entry, BSL_Load_PC(hCom, pELF32->e_entry) ? L"OK" : L"KO");
								}
							}
						}
						CloseHandle(hCom);
					}
				}
				else PRINT_ERROR(L"Invalid ELF file for this program\n");

				LocalFree(pELF32);
			}
		}
		else PRINT_ERROR(L"Argument /port:COM# needed\n");
	}
	else PRINT_ERROR(L"Argument /elf:filename.out needed\n");

	return EXIT_SUCCESS;
}