#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <tchar.h>
#include "BatsModule.h"

namespace BWAPI { Game* Broodwar; }

#pragma warning (push)
#pragma warning (disable:4100)
BOOL APIENTRY DllMain( HANDLE hModule, 
	DWORD  reasonForCall, 
	LPVOID pReserved
	)
{
	switch (reasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		BWAPI::BWAPI_init();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}


	return TRUE;
}
#pragma warning (pop)

extern "C" __declspec(dllexport) BWAPI::AIModule* newAIModule(BWAPI::Game* game)
{
	BWAPI::Broodwar=game;
	return new bats::BatsModule();
}