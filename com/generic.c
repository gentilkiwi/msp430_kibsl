#include "generic.h"

const GENERIC_COM * Coms[] = {
	&COM_SERIAL,
	&COM_FTDI,
	&COM_FT234XD,
};

BOOL Generic_Com_From_Args(PGENERIC_COMMUNICATOR Communicator, int argc, wchar_t* argv[])
{
	BOOL ret = FALSE;
	LPCWSTR szDriver;
	size_t i;

	Communicator->Com = NULL;
	GET_CLI_ARG_DEF(L"driver", &szDriver, COM_SERIAL.Name);

	for (i = 0; i < ARRAYSIZE(Coms); i++)
	{
		if (_wcsicmp(szDriver, Coms[i]->Name) == 0)
		{
			Communicator->Com = Coms[i];
			break;
		}
	}

	if (Communicator->Com)
	{
		kprintf(L"Selected driver: %s\n", Communicator->Com->Name);
		ret = TRUE;
	}
	else
	{
		kprintf(L"Unable to find a driver for: \'%s\', available drivers:\n", szDriver);
		for (i = 0; i < ARRAYSIZE(Coms); i++)
		{
			kprintf(L" - %s\n", Coms[i]->Name);
		}
	}

	return ret;
}