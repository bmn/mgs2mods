// dllmain.cpp : Defines the entry point for the DLL application.
//#include "framework.h"
#include "Hooking.Patterns.h"
#include "SimpleIni.h"
#include <filesystem>

#if defined ASI_UNMETAL
#include "UnMetal.framework.h"
#endif
#if defined ASI_MGS2
#include "MGS2.framework.h"
#endif



void OverrideIni(const char* category, CSimpleIniA &oldIni) {
    if (oldIni.GetSectionSize(category) != -1) {
        return;
    }

    char* filename = (char*)malloc(strlen(category) + 10);
    strcpy(filename, "MGS2.");
    strcat(filename, category);
    strcat(filename, ".ini\x00");
    if (std::filesystem::exists(filename))
        SI_Error iniError = oldIni.LoadFile(filename);
}


bool PatchMemory(const char* find, int offset, const char* replace)
{
    auto pattern = hook::pattern(find);
    if (!pattern.count_hint(1).empty())
    {
        void* pointer = pattern.get(0).m_pointer;
        void* offsetPointer = (char*)pointer + offset;

        return PatchMemory(offsetPointer, replace);
    }
    return false;
}

bool PatchMemory(void* pointer, const char* replace, int length) {
    void* replacement = (char*)replace;

    if (length <= 0) {
        length = 1;
    }

    DWORD oldProtection;
    if (!VirtualProtect(pointer, length, PAGE_EXECUTE_READWRITE, &oldProtection))
        return false;
    memcpy(pointer, replacement, length);
    VirtualProtect(pointer, length, oldProtection, &oldProtection);
    return true;
}

bool PatchMemory(void* pointer, const char* replace) {
    return PatchMemory(pointer, replace, strlen(replace));
}

void InitialiseMod()
{
    OutputDebugString(L"bmn asi getting started");

#ifdef _DEBUG
    MessageBoxA(NULL, "Attempting to patch game code...", "bmn asi mod thing", MB_OK | MB_ICONINFORMATION);
#endif
    
    CSimpleIniA ini;
    SI_Error rc = ini.LoadFile("MGS2.ini");
    if (!ini.GetBoolValue("MGS2", "Enabled", true))
        return;


#if defined ASI_UNMETAL
    UnMetal::RunUnFocused();
#endif
#if defined ASI_MGS2
    MGS2::Actions::Run(ini);
    MGS2::RunAffinity(ini);
    MGS2::Timer::Run(ini);
    MGS2::Turbo::Run(ini);
    MGS2::TurboDisplay::Run(ini);
    MGS2::InputDisplay::Run(ini);
    MGS2::RunDInputBg(ini);
    MGS2::RunNoQuitPrompt(ini);
    MGS2::RunConsole(ini);
    MGS2::Caution::Run(ini);
    MGS2::RunPS2Controls(ini);
    MGS2::TextChange::Run(ini);
    MGS2::RunUnlockRadar(ini);
    MGS2::RunDrebinMode(ini);
    MGS2::FirstPerson::Run(ini);
    MGS2::SaveLocation::Run(ini);
    MGS2::SaveGame::Run(ini);
    MGS2::SoftReset::Run(ini);
    MGS2::NewGameInfo::Run(ini);
    MGS2::Options::Run(ini);
    MGS2::Wet::Run(ini);
    MGS2::Performance::Run(ini);
    MGS2::DelayedLoad::Run(ini);
#endif

    
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitialiseMod();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

