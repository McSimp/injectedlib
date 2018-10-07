#pragma once

#ifdef _DEBUG
#define SPDLOG_TRACE_ON
#endif
#define SPDLOG_DEBUG_ON

#include <spdlog/spdlog.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <spdlog/fmt/bundled/printf.h>
#include "../../thirdparty/minhook/include/MinHook.h"
#include "Console.h"
#include "Util.h"
#include "ModuleFuncResolver.h"
#include "ExternalFuncs.h"
#include "Hooking.h"

namespace ilib {

void Initialize(const std::string& logFilePath, bool createConsoleWindow);
void Shutdown();

}