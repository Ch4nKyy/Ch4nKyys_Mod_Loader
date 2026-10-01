#include "proxy.hpp"
#include <format>
#include <string>

namespace
{
    INIT_ONCE g_forward_module_once = INIT_ONCE_STATIC_INIT;
    HMODULE g_forward_module = nullptr;

    BOOL CALLBACK InitForwardModule(PINIT_ONCE, PVOID, PVOID*)
    {
        // We cannot simply load the systems version.dll, because our proxy version.dll is already
        // loaded and LoadLibrary would return our proxy instead of the real one, even if we use
        // LOAD_LIBRARY_SEARCH_SYSTEM32.
        //
        // We can either hardcode the systems version.dll path or we can use kernelbase.dll
        // instead to access the necessary functions, because all of the exported functions are
        // also available in kernelbase.dll.
        g_forward_module = LoadLibraryExW(L"kernelbase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (!g_forward_module)
        {
            const DWORD error = GetLastError();

            const std::wstring message = std::format(L"Failed to load kernelbase.dll.\n\n"
                                                     L"Win32 error: {}\n\n"
                                                     L"Mod loading has been disabled.",
                                                     error);
            MessageBoxW(nullptr, message.c_str(), L"Ch4nKyy's Mod Loader", MB_OK | MB_ICONERROR);

            return FALSE;
        }

        return g_forward_module != nullptr;
    }

    template <class T> T Resolve(const char* name)
    {
        const HMODULE module = GetForwardModule();

        if (!module)
        {
            SetLastError(ERROR_MOD_NOT_FOUND);
            return nullptr;
        }

        const FARPROC proc = GetProcAddress(module, name);

        if (!proc)
        {
            SetLastError(ERROR_PROC_NOT_FOUND);
            return nullptr;
        }

        return reinterpret_cast<T>(proc);
    }
}

HMODULE GetForwardModule()
{
    if (!InitOnceExecuteOnce(&g_forward_module_once, InitForwardModule, nullptr, nullptr))
    {
        return nullptr;
    }

    return g_forward_module;
}

extern "C"
{
    BOOL WINAPI Proxy_GetFileVersionInfoA(LPCSTR filename, DWORD handle, DWORD length, LPVOID data)
    {
        using Fn = decltype(&::GetFileVersionInfoA);

        const auto fn = Resolve<Fn>("GetFileVersionInfoA");

        return fn ? fn(filename, handle, length, data) : FALSE;
    }

    BOOL WINAPI Proxy_GetFileVersionInfoByHandle(DWORD flags, HANDLE file, LPVOID* data,
                                                 PDWORD length)
    {
        using Fn = BOOL(WINAPI*)(DWORD, HANDLE, LPVOID*, PDWORD);

        const auto fn = Resolve<Fn>("GetFileVersionInfoByHandle");

        return fn ? fn(flags, file, data, length) : FALSE;
    }

    BOOL WINAPI Proxy_GetFileVersionInfoExA(DWORD flags, LPCSTR filename, DWORD handle,
                                            DWORD length, LPVOID data)
    {
        using Fn = decltype(&::GetFileVersionInfoExA);

        const auto fn = Resolve<Fn>("GetFileVersionInfoExA");

        return fn ? fn(flags, filename, handle, length, data) : FALSE;
    }

    BOOL WINAPI Proxy_GetFileVersionInfoExW(DWORD flags, LPCWSTR filename, DWORD handle,
                                            DWORD length, LPVOID data)
    {
        using Fn = decltype(&::GetFileVersionInfoExW);

        const auto fn = Resolve<Fn>("GetFileVersionInfoExW");

        return fn ? fn(flags, filename, handle, length, data) : FALSE;
    }

    DWORD WINAPI Proxy_GetFileVersionInfoSizeA(LPCSTR filename, LPDWORD handle)
    {
        using Fn = decltype(&::GetFileVersionInfoSizeA);

        const auto fn = Resolve<Fn>("GetFileVersionInfoSizeA");

        return fn ? fn(filename, handle) : 0;
    }

    DWORD WINAPI Proxy_GetFileVersionInfoSizeExA(DWORD flags, LPCSTR filename, LPDWORD handle)
    {
        using Fn = decltype(&::GetFileVersionInfoSizeExA);

        const auto fn = Resolve<Fn>("GetFileVersionInfoSizeExA");

        return fn ? fn(flags, filename, handle) : 0;
    }

    DWORD WINAPI Proxy_GetFileVersionInfoSizeExW(DWORD flags, LPCWSTR filename, LPDWORD handle)
    {
        using Fn = decltype(&::GetFileVersionInfoSizeExW);

        const auto fn = Resolve<Fn>("GetFileVersionInfoSizeExW");

        return fn ? fn(flags, filename, handle) : 0;
    }

    DWORD WINAPI Proxy_GetFileVersionInfoSizeW(LPCWSTR filename, LPDWORD handle)
    {
        using Fn = decltype(&::GetFileVersionInfoSizeW);

        const auto fn = Resolve<Fn>("GetFileVersionInfoSizeW");

        return fn ? fn(filename, handle) : 0;
    }

    BOOL WINAPI Proxy_GetFileVersionInfoW(LPCWSTR filename, DWORD handle, DWORD length, LPVOID data)
    {
        using Fn = decltype(&::GetFileVersionInfoW);

        const auto fn = Resolve<Fn>("GetFileVersionInfoW");

        return fn ? fn(filename, handle, length, data) : FALSE;
    }

    DWORD WINAPI Proxy_VerFindFileA(DWORD flags, LPCSTR file_name, LPCSTR win_dir, LPCSTR app_dir,
                                    LPSTR current_dir, PUINT current_dir_length,
                                    LPSTR destination_dir, PUINT destination_dir_length)
    {
        using Fn = decltype(&::VerFindFileA);

        const auto fn = Resolve<Fn>("VerFindFileA");

        return fn ? fn(flags, file_name, win_dir, app_dir, current_dir, current_dir_length,
                       destination_dir, destination_dir_length)
                  : 0;
    }

    DWORD WINAPI Proxy_VerFindFileW(DWORD flags, LPCWSTR file_name, LPCWSTR win_dir,
                                    LPCWSTR app_dir, LPWSTR current_dir, PUINT current_dir_length,
                                    LPWSTR destination_dir, PUINT destination_dir_length)
    {
        using Fn = decltype(&::VerFindFileW);

        const auto fn = Resolve<Fn>("VerFindFileW");

        return fn ? fn(flags, file_name, win_dir, app_dir, current_dir, current_dir_length,
                       destination_dir, destination_dir_length)
                  : 0;
    }

    DWORD WINAPI Proxy_VerInstallFileA(DWORD flags, LPCSTR source_file_name,
                                       LPCSTR destination_file_name, LPCSTR source_dir,
                                       LPCSTR destination_dir, LPCSTR current_dir, LPSTR temp_file,
                                       PUINT temp_file_length)
    {
        using Fn = decltype(&::VerInstallFileA);

        const auto fn = Resolve<Fn>("VerInstallFileA");

        return fn ? fn(flags, source_file_name, destination_file_name, source_dir, destination_dir,
                       current_dir, temp_file, temp_file_length)
                  : 0;
    }

    DWORD WINAPI Proxy_VerInstallFileW(DWORD flags, LPCWSTR source_file_name,
                                       LPCWSTR destination_file_name, LPCWSTR source_dir,
                                       LPCWSTR destination_dir, LPCWSTR current_dir,
                                       LPWSTR temp_file, PUINT temp_file_length)
    {
        using Fn = decltype(&::VerInstallFileW);

        const auto fn = Resolve<Fn>("VerInstallFileW");

        return fn ? fn(flags, source_file_name, destination_file_name, source_dir, destination_dir,
                       current_dir, temp_file, temp_file_length)
                  : 0;
    }

    DWORD WINAPI Proxy_VerLanguageNameA(DWORD language, LPSTR buffer, DWORD buffer_length)
    {
        using Fn = decltype(&::VerLanguageNameA);

        const auto fn = Resolve<Fn>("VerLanguageNameA");

        return fn ? fn(language, buffer, buffer_length) : 0;
    }

    DWORD WINAPI Proxy_VerLanguageNameW(DWORD language, LPWSTR buffer, DWORD buffer_length)
    {
        using Fn = decltype(&::VerLanguageNameW);

        const auto fn = Resolve<Fn>("VerLanguageNameW");

        return fn ? fn(language, buffer, buffer_length) : 0;
    }

    BOOL WINAPI Proxy_VerQueryValueA(LPCVOID block, LPCSTR sub_block, LPVOID* buffer, PUINT length)
    {
        using Fn = decltype(&::VerQueryValueA);

        const auto fn = Resolve<Fn>("VerQueryValueA");

        return fn ? fn(block, sub_block, buffer, length) : FALSE;
    }

    BOOL WINAPI Proxy_VerQueryValueW(LPCVOID block, LPCWSTR sub_block, LPVOID* buffer, PUINT length)
    {
        using Fn = decltype(&::VerQueryValueW);

        const auto fn = Resolve<Fn>("VerQueryValueW");

        return fn ? fn(block, sub_block, buffer, length) : FALSE;
    }
}
