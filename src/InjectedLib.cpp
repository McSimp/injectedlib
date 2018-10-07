#include "stdafx.h"

namespace ilib {

std::unique_ptr<Console> g_console;

void SetupLogger(const std::string& filename)
{
    // Create sinks to file and console
    std::vector<spdlog::sink_ptr> sinks;

    if (g_console)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
    }

    // The file sink could fail so capture the error if so
    std::unique_ptr<std::string> fileError;
    try
    {
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true));
    }
    catch (spdlog::spdlog_ex& ex)
    {
        fileError = std::make_unique<std::string>(ex.what());
    }

    // Create logger from sinks
    auto logger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
    logger->set_pattern("[%T] [thread %t] [%l] %^%v%$");
#ifdef _DEBUG
    logger->set_level(spdlog::level::trace);
#else
    logger->set_level(spdlog::level::debug);
#endif

    if (fileError)
    {
        logger->warn("Failed to initialise file sink, log file will be unavailable ({})", *fileError);
    }

    spdlog::register_logger(logger);
}

void Initialize(const std::string& logFilePath, bool createConsoleWindow)
{
    if (createConsoleWindow)
    {
        g_console = std::make_unique<Console>();
    }
    
    SetupLogger(logFilePath);
}

void Shutdown()
{
    g_console.reset();
}

}