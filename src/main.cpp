#include <windows.h>

#include "mod_loader.hpp"

namespace
{
    DWORD WINAPI ShowInitTermEHookError(LPVOID)
    {
        MessageBoxW(nullptr,
                    L"Failed to install the _initterm_e hook.\n\n"
                    L"Mods will not be loaded.",
                    L"Ch4nKyy's Mod Loader", MB_OK | MB_ICONERROR);

        return 0;
    }

    void ShowInitTermEHookErrorAsync()
    {
        const HANDLE thread = CreateThread(nullptr, 0, ShowInitTermEHookError, nullptr, 0, nullptr);

        if (thread)
        {
            CloseHandle(thread);
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH)
    {
        return TRUE;
    }

    if (IsNativeModLoaderPresent(instance))
    {
        return TRUE;
    }

    if (!InstallInitTermEHook(instance))
    {
        ShowInitTermEHookErrorAsync();
    }

    return TRUE;
}