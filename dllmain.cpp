#include "MGS2.framework.h"


void InitialiseMod()
{
    OutputDebugString(L"bmn asi getting started");

#ifdef _DEBUG
    //MessageBoxA(NULL, "Attempting to patch game code...", "bmn asi mod thing", MB_OK | MB_ICONINFORMATION);
#endif
    
    MGS2::Run();
}

HMODULE ModuleHandle = nullptr;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ModuleHandle = hModule;
        InitialiseMod();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
