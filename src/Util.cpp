#include "stdafx.h"

namespace ilib {
namespace Util {

// Taken from https://stackoverflow.com/a/18374698
std::wstring Widen(const std::string& input)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(input);
}

// Taken from https://stackoverflow.com/a/18374698
std::string Narrow(const std::wstring& input)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(input);
}

// This will convert some data like "Hello World" to "48 65 6C 6C 6F 20 57 6F 72 6C 64"
// Taken mostly from https://stackoverflow.com/a/3382894
std::string DataToHex(const char* input, size_t len)
{
    static const char* const lut = "0123456789ABCDEF";

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; i++)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }

    return output;
}

template <DWORD(__stdcall *Func)(HANDLE)>
void PerformThreadOperation()
{
    auto logger = spdlog::get("logger");

    // Take a snapshot of all running threads  
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
    {
        logger->error("Failed to call CreateToolhelp32Snapshot, cannot perform thread operation");
        return;
    }

    // Fill in the size of the structure before using it. 
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(hThreadSnap, &te32))
    {
        logger->error("Failed to call Thread32First, cannot perform thread operation");
        CloseHandle(hThreadSnap);
        return;
    }

    // Now walk the thread list of the system,
    // and display information about each thread
    // associated with the specified process
    do
    {
        if (te32.th32OwnerProcessID == GetCurrentProcessId() && te32.th32ThreadID != GetCurrentThreadId())
        {
            HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            if (thread != NULL)
            {
                Func(thread);
                CloseHandle(thread);
            }
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
}

void SuspendAllOtherThreads()
{
    PerformThreadOperation<SuspendThread>();
}

void ResumeAllOtherThreads()
{
    PerformThreadOperation<ResumeThread>();
}

ThreadSuspender::ThreadSuspender()
{
    Util::SuspendAllOtherThreads();
}

ThreadSuspender::~ThreadSuspender()
{
    Util::ResumeAllOtherThreads();
}

void FindAndReplaceAll(std::string& data, const std::string& search, const std::string& replace)
{
    size_t pos = data.find(search);
    while (pos != std::string::npos)
    {
        data.replace(pos, search.size(), replace);
        pos = data.find(search, pos + replace.size());
    }
}

void* ResolveLibraryExport(const std::string& module, const std::string& exportName)
{
    HMODULE hModule = GetModuleHandle(Util::Widen(module).c_str());
    if (!hModule)
    {
        throw std::runtime_error(fmt::sprintf("GetModuleHandle failed for %s (Error = 0x%X)", module, GetLastError()));
    }

    void* exportPtr = GetProcAddress(hModule, exportName.c_str());
    if (!exportPtr)
    {
        throw std::runtime_error(fmt::sprintf("GetProcAddress failed for %s (Error = 0x%X)", exportName, GetLastError()));
    }

    return exportPtr;
}

void FixSlashes(char* pname, char separator)
{
    while (*pname)
    {
        if (*pname == '\\' || *pname == '/')
        {
            *pname = separator;
        }
        pname++;
    }
}

std::string ConcatStrings(const std::vector<std::string>& strings, const char* delim)
{
    std::ostringstream imploded;
    std::copy(strings.begin(), strings.end(), std::ostream_iterator<std::string>(imploded, delim));
    return imploded.str();
}

HMODULE WaitForModuleHandle(const std::string& moduleName)
{
    HMODULE module = nullptr;
    do
    {
        module = GetModuleHandle(Util::Widen(moduleName).c_str());;
    } while (module == nullptr);
    return module;
}

std::string ReadFileToString(std::ifstream& f)
{
    std::string fileData;
    f.seekg(0, std::ios::end);

    std::streamoff size = f.tellg();
    if (size > std::numeric_limits<size_t>::max())
    {
        throw std::runtime_error("File is too large to read into std::string");
    }

    fileData.reserve(static_cast<size_t>(f.tellg()));
    f.seekg(0, std::ios::beg);

    fileData.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return fileData;
}

// Taken from https://stackoverflow.com/a/9435385
std::vector<std::string> Split(const std::string & s, char delim)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(std::move(item));
    }
    return elems;
}

}
}