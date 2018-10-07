#pragma once
#include <map>

namespace ilib {

class ResolveException : public std::runtime_error
{
public:
    ResolveException(const std::string& errorStr)
        : std::runtime_error(errorStr) {}
};

class ModuleFuncResolver
{
private:
    char* m_moduleBase;
    DWORD m_moduleLen;
    HMODULE m_moduleHandle;

public:
    ModuleFuncResolver(const std::string& moduleName);
    ModuleFuncResolver(HMODULE moduleName);

    void* SigScan(const char* sig, const char* mask, size_t sigLength);
    void* SigScan(const char* sig, const char* mask);
    void* GetFromRVA(uint64_t rva);

    static ModuleFuncResolver& GetResolver(const std::string& moduleName);

private:
    static std::map<std::string, ModuleFuncResolver> Resolvers;
};

}
