#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <tchar.h>

/** @todo change to "BATSModule.h"  */
#include "BTHAIModule.h"
namespace BWAPI { Game* Broodwar; }
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

extern "C" __declspec(dllexport) BWAPI::AIModule* newAIModule(BWAPI::Game* game)
{
	BWAPI::Broodwar=game;
	return new BTHAIModule();
}