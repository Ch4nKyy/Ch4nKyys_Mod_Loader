#include "mod_loader.hpp"
#include "logger.hpp"
#include "proxy.hpp"
#include "utility.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace
{
    using CInitFunction = int(__cdecl*)();
    using InitTermE = int(__cdecl*)(CInitFunction*, CInitFunction*);

    HMODULE g_proxy_module = nullptr;
    InitTermE g_original_initterm_e = nullptr;
    std::once_flag g_load_mods_once;

    void LoadMod(const fs::path& path, Logger& logger)
    {
        HMODULE loaded_module =
            LoadLibraryExW(path.c_str(), nullptr,
                           LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

        if (loaded_module)
        {
            logger.Write("Load succeeded: {}", PathToUtf8(path.filename()));

            using InitFunction = void(__cdecl*)();

            const auto init = reinterpret_cast<InitFunction>(GetProcAddress(loaded_module, "Init"));

            if (init)
            {
                logger.Write("Calling Init().");
                init();
            }
        }
        else
        {
            const DWORD error_code = GetLastError();

            const std::error_code load_error{static_cast<int>(error_code), std::system_category()};

            logger.Write("Load failed: {} ({}: {})", PathToUtf8(path.filename()), error_code,
                         load_error.message());
        }
    }

    void SortModsCaseInsensitiveLexicographical(std::vector<fs::path>& mods)
    {
        std::ranges::sort(mods,
                          [](const fs::path& lhs, const fs::path& rhs)
                          {
                              const std::wstring lhs_name = lhs.filename().wstring();
                              const std::wstring rhs_name = rhs.filename().wstring();

                              return CompareStringOrdinal(
                                         lhs_name.c_str(), static_cast<int>(lhs_name.size()),
                                         rhs_name.c_str(), static_cast<int>(rhs_name.size()),
                                         TRUE) == CSTR_LESS_THAN;
                          });
    }

    void LoadMods()
    {
        const auto paths = GetPaths(g_proxy_module);

        if (!paths)
        {
            return;
        }

        Logger logger{paths->log_path};
        logger.Write("Loading mods from: {}", PathToUtf8(paths->mods_directory));

        std::error_code error;
        fs::directory_iterator iterator{paths->mods_directory, error};

        if (error)
        {
            if (error == std::errc::no_such_file_or_directory)
            {
                logger.Write("NativeMods directory does not exist.");
            }
            else
            {
                logger.Write("Failed to enumerate NativeMods directory: {}", error.message());
            }

            return;
        }

        std::vector<fs::path> mods;
        const fs::directory_iterator end{};

        while (iterator != end)
        {
            const fs::directory_entry& entry = *iterator;
            std::error_code entry_error;

            if (entry.is_regular_file(entry_error) && !entry_error && IsDll(entry.path()))
            {
                mods.push_back(entry.path());
            }

            iterator.increment(error);

            if (error)
            {
                logger.Write("Failed while enumerating NativeMods directory: {}", error.message());

                break;
            }
        }

        SortModsCaseInsensitiveLexicographical(mods);

        if (mods.empty())
        {
            logger.Write("No mods found.");
        }

        for (const fs::path& path : mods)
        {
            LoadMod(path, logger);
        }

        logger.Write("Finished loading mods.");
    }

    int __cdecl InitTermEHook(CInitFunction* first, CInitFunction* last)
    {
        const HMODULE forward_module = GetForwardModule();

        if (forward_module)
        {
            std::call_once(g_load_mods_once, [] { LoadMods(); });
        }

        return g_original_initterm_e ? g_original_initterm_e(first, last) : 0;
    }
}

bool IsNativeModLoaderPresent(HINSTANCE instance)
{
    const auto paths = GetPaths(instance);

    if (!paths)
    {
        // At this point, we cannot say if NativeModLoader is present.
        // Returning true means don't load mods.
        return true;
    }

    const auto nml_path = paths->module_path.parent_path() / L"bink2w64.dll";

    std::error_code ec;
    if (!fs::is_regular_file(nml_path, ec))
    {
        return false;
    }

    if (!FileContainsBytes(nml_path, "NativeModLoader"))
    {
        return false;
    }

    Logger logger{paths->log_path};
    logger.Write("NativeModLoader detected. Will not load mods.");

    return true;
}

bool InstallInitTermEHook(HINSTANCE instance)
{
    g_proxy_module = instance;

    const HMODULE executable = GetModuleHandleW(nullptr);

    if (!executable)
    {
        return false;
    }

    auto* base = reinterpret_cast<std::byte*>(executable);
    const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);

    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return false;
    }

    const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos_header->e_lfanew);

    if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
    {
        return false;
    }

    const IMAGE_DATA_DIRECTORY& import_directory =
        nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (import_directory.VirtualAddress == 0)
    {
        return false;
    }

    auto* descriptor =
        reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + import_directory.VirtualAddress);

    for (; descriptor->Name != 0; ++descriptor)
    {
        const char* module_name = reinterpret_cast<const char*>(base + descriptor->Name);

        if (_stricmp(module_name, "api-ms-win-crt-runtime-l1-1-0.dll") != 0)
        {
            continue;
        }

        if (descriptor->OriginalFirstThunk == 0 || descriptor->FirstThunk == 0)
        {
            return false;
        }

        const auto* names =
            reinterpret_cast<const IMAGE_THUNK_DATA*>(base + descriptor->OriginalFirstThunk);

        auto* functions = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->FirstThunk);

        for (; names->u1.AddressOfData != 0; ++names, ++functions)
        {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal))
            {
                continue;
            }

            const auto* import =
                reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);

            const char* name = reinterpret_cast<const char*>(import->Name);

            if (std::strcmp(name, "_initterm_e") != 0)
            {
                continue;
            }

            auto* slot = &functions->u1.Function;

            const auto original_address = static_cast<std::uintptr_t>(*slot);

            if (original_address == 0)
            {
                return false;
            }

            DWORD old_protection{};

            if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protection))
            {
                return false;
            }

            g_original_initterm_e = reinterpret_cast<InitTermE>(original_address);

            *slot = reinterpret_cast<ULONG_PTR>(&InitTermEHook);

            DWORD ignored{};

            VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);

            return true;
        }

        // We found the correct CRT import descriptor,
        // but it didn't contain _initterm_e.
        return false;
    }

    return false;
}
