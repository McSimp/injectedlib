#include "stdafx.h"

namespace ilib {

Console::Console()
    : m_winConsole(), m_conOut("CONOUT$", "w", stdout), m_conIn("CONIN$", "r", stdin)
{
    RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
    SetConsoleCtrlHandler(NULL, TRUE);
}

Console::~Console()
{
    SetConsoleCtrlHandler(NULL, FALSE);
}

FileStreamWrapper::FileStreamWrapper(const char* filename, const char* mode, FILE* oldStream) : m_file(nullptr)
{
    errno_t err = freopen_s(&m_file, filename, mode, oldStream);
    if (err != 0)
    {
        throw std::runtime_error("Failed to reopen"); // TODO: better exception
    }
}

FileStreamWrapper::~FileStreamWrapper()
{
    if (m_file != nullptr)
    {
        fclose(m_file);
        m_file = nullptr;
    }
}

WindowsConsole::WindowsConsole()
{
    if (!AllocConsole())
    {
        throw std::runtime_error("Failed to AllocConsole()"); // TODO: better exception
    }
}

WindowsConsole::~WindowsConsole()
{
    FreeConsole();
}

}
