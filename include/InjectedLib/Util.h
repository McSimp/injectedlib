#pragma once

#include <string>
#include <vector>

namespace ilib {
namespace Util {

std::wstring Widen(const std::string& input);
std::string Narrow(const std::wstring& input);
std::string DataToHex(const char* input, size_t len);
void SuspendAllOtherThreads();
void ResumeAllOtherThreads();

struct ThreadSuspender
{
    ThreadSuspender();
    ~ThreadSuspender();
};

void FindAndReplaceAll(std::string& data, const std::string& search, const std::string& replace);
void* ResolveLibraryExport(const std::string& module, const std::string& exportName);
void FixSlashes(char* pname, char separator);
std::string ConcatStrings(const std::vector<std::string>& strings, const char* delim);
HMODULE WaitForModuleHandle(const std::string& moduleName);
std::string ReadFileToString(std::ifstream& f);
std::vector<std::string> Split(const std::string & s, char delim);

}
}
