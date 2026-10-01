#pragma once

#include <Windows.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

struct LoaderPaths
{
    fs::path mods_directory;
    fs::path log_path;
    fs::path bin_directory;
    fs::path module_path;
};

std::optional<fs::path> GetModulePath(HMODULE module);

const std::optional<LoaderPaths>& GetPaths(HMODULE module);

std::string WideToUtf8(std::wstring_view text);

std::string PathToUtf8(const fs::path& path);

bool IsDll(const fs::path& path);

bool FileContainsBytes(const fs::path& path, std::string_view bytes);
