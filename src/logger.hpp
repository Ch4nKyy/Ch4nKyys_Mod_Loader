#pragma once
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class Logger
{
public:
    explicit Logger(const fs::path& path) : stream_{path, std::ios::binary | std::ios::trunc} {}

    template <class... Args> void Write(std::format_string<Args...> format, Args&&... args)
    {
        if (!stream_)
        {
            return;
        }

        SYSTEMTIME time{};
        GetLocalTime(&time);

        stream_ << std::format("[{:04}-{:02}-{:02} "
                               "{:02}:{:02}:{:02}] ",
                               time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute,
                               time.wSecond)
                << std::format(format, std::forward<Args>(args)...) << '\n';

        stream_.flush();
    }

private:
    std::ofstream stream_;
};
