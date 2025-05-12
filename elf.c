/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#include "elf.h"

const E_IDENT ELF_TI_MSP430 = {
	.ei_magic = ELF_MAGIC,
	.ei_class = ELFCLASS32,
	.ei_data = ELFDATA2LSB,
	.ei_version = EV_CURRENT,
	.ei_osabi = ELFOSABI_NONE,
	.ei_abiversion = ELFABIVERSION_UNSPECIFIED,
};

const ELF32_PROGRAM_HEADER* ELF_Validate_TI_MSP430(const ELF32* pELF32, DWORD cbData)
{
	const ELF32_PROGRAM_HEADER* pProgramHeader = NULL;

	if (cbData >= sizeof(ELF32))
	{
		if (RtlEqualMemory(&ELF_TI_MSP430, &pELF32->e_ident, FIELD_OFFSET(E_IDENT, ei_pad)))
		{
			if ((pELF32->e_type == ET_EXEC) && (pELF32->e_machine == EM_MSP430) && (sizeof(ELF32_PROGRAM_HEADER) == pELF32->e_phentsize))
			{
				if (pELF32->e_phoff && pELF32->e_phnum)
				{
					pProgramHeader = (const ELF32_PROGRAM_HEADER*)((PBYTE)pELF32 + pELF32->e_phoff);
				}
			}
		}
	}

	return pProgramHeader;
}