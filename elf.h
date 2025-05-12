/*	Benjamin DELPY `gentilkiwi`
	https://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : https://creativecommons.org/licenses/by/4.0/
*/
#pragma once
#include <windows.h>

#pragma pack(push, 1) 

#define EV_CURRENT  1
#define ELFCLASS32	1
#define ELFDATA2LSB	1
#define ELFOSABI_NONE	0
#define ELFABIVERSION_UNSPECIFIED 0

#define ELF_MAGIC   'FLE\x7f'

typedef struct _E_IDENT {
    DWORD ei_magic;
    BYTE ei_class;
    BYTE ei_data;
    BYTE ei_version;
    BYTE ei_osabi;
    BYTE ei_abiversion;
    BYTE ei_pad[7];
} E_IDENT, * PE_IDENT;

#define ET_EXEC	    0x0002
#define EM_MSP430   0x0069

typedef struct _ELF32 {
    E_IDENT e_ident;
    WORD e_type;
    WORD e_machine;
    DWORD e_version;
    DWORD e_entry;
    DWORD e_phoff;
    DWORD e_shoff;
    DWORD e_flags;
    WORD e_ehsize;
    WORD e_phentsize;
    WORD e_phnum;
    WORD e_shentsize;
    WORD e_shnum;
    WORD e_shstrndx;
} ELF32, * PELF32;

#define PT_LOAD     0x00000001

typedef struct _ELF32_PROGRAM_HEADER {
    DWORD p_type;
    DWORD p_offset;
    DWORD p_vaddr;
    DWORD p_paddr;
    DWORD p_filesz;
    DWORD p_memsz;
    DWORD p_flags;
    DWORD p_align;
} ELF32_PROGRAM_HEADER, * PELF32_PROGRAM_HEADER;

#pragma pack(pop)

const ELF32_PROGRAM_HEADER* ELF_Validate_TI_MSP430(const ELF32* pELF32, DWORD cbData);