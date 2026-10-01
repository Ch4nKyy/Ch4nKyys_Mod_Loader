#include "utility.hpp"
#include <cwctype>
#include <fstream>

std::optional<fs::path> GetModulePath(HMODULE module)
{
    std::vector<wchar_t> buffer(512);

    for (;;)
    {
        if (buffer.size() > std::numeric_limits<DWORD>::max())
        {
            return std::nullopt;
        }

        const DWORD length =
            GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));

        if (length == 0)
        {
            return std::nullopt;
        }

        if (length < buffer.size())
        {
            return fs::path{std::wstring{buffer.data(), length}};
        }

        buffer.resize(buffer.size() * 2);
    }
}

const std::optional<LoaderPaths>& GetPaths(HMODULE module)
{
    // Cache the paths so that we only build them once.
    static const std::optional<LoaderPaths> paths = [module]() -> std::optional<LoaderPaths>
    {
        const auto module_path = GetModulePath(module);

        if (!module_path || !module_path->has_parent_path())
        {
            MessageBoxW(nullptr,
                        L"Failed to determine the module path.\n\n"
                        L"Mods will not be loaded.",
                        L"Ch4nKyy's Mod Loader", MB_OK | MB_ICONERROR);

            return std::nullopt;
        }

        const fs::path directory = module_path->parent_path();

        return LoaderPaths{.mods_directory = directory / L"NativeMods",
                           .log_path = directory / L"Ch4nKyysModLoader.log",
                           .bin_directory = directory,
                           .module_path = module_path.value()};
    }();

    return paths;
}

std::string WideToUtf8(std::wstring_view text)
{
    if (text.empty())
    {
        return {};
    }

    if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        return {};
    }

    const int source_length = static_cast<int>(text.size());

    const int required_size =
        WideCharToMultiByte(CP_UTF8, 0, text.data(), source_length, nullptr, 0, nullptr, nullptr);

    if (required_size <= 0)
    {
        return {};
    }

    std::string result(static_cast<std::size_t>(required_size), '\0');

    const int converted_size = WideCharToMultiByte(CP_UTF8, 0, text.data(), source_length,
                                                   result.data(), required_size, nullptr, nullptr);

    if (converted_size <= 0)
    {
        return {};
    }

    return result;
}

std::string PathToUtf8(const fs::path& path) { return WideToUtf8(path.native()); }

bool IsDll(const fs::path& path)
{
    std::wstring extension = path.extension().wstring();

    std::ranges::transform(extension, extension.begin(), [](wchar_t character)
                           { return static_cast<wchar_t>(std::towlower(character)); });

    return extension == L".dll";
}

bool FileContainsBytes(const fs::path& path, std::string_view bytes)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        return false;
    }

    const std::vector<char> data{std::istreambuf_iterator<char>(file),
                                 std::istreambuf_iterator<char>()};

    return std::search(data.begin(), data.end(), bytes.begin(), bytes.end()) != data.end();
}
