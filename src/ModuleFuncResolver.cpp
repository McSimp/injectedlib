#include "stdafx.h"

namespace ilib {
std::map<std::string, ModuleFuncResolver> ModuleFuncResolver::Resolvers;

// Based off CSigScan from https://wiki.alliedmods.net/Signature_Scanning

ModuleFuncResolver::ModuleFuncResolver(const std::string& moduleName) : ModuleFuncResolver(GetModuleHandle(Util::Widen(moduleName).c_str()))
{

}

ModuleFuncResolver::ModuleFuncResolver(HMODULE module)
{
    m_moduleHandle = module;
    if (m_moduleHandle == nullptr)
    {
        throw ResolveException(fmt::sprintf("GetModuleHandle returned NULL (Win32 Error = %d)", GetLastError()));
    }

    MEMORY_BASIC_INFORMATION mem;
    if (!VirtualQuery(m_moduleHandle, &mem, sizeof(mem)))
    {
        throw ResolveException(fmt::sprintf("VirtualQuery returned NULL (Win32 Error = %d)", GetLastError()));
    }

    m_moduleBase = (char*)mem.AllocationBase;
    if (m_moduleBase == nullptr)
    {
        throw ResolveException("mem.AllocationBase was NULL");
    }

    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)((unsigned char*)dos + dos->e_lfanew);

    if (pe->Signature != IMAGE_NT_SIGNATURE)
    {
        throw ResolveException("PE signature is not a valid NT signature");
    }

    m_moduleLen = pe->OptionalHeader.SizeOfImage;
}

void* ModuleFuncResolver::SigScan(const char* sig, const char* mask)
{
    size_t sigLength = strlen(mask);
    return SigScan(sig, mask, sigLength);
}

void* ModuleFuncResolver::SigScan(const char* sig, const char* mask, size_t sigLength)
{
    char* pData = m_moduleBase;
    char* pEnd = m_moduleBase + m_moduleLen;

    while (pData < (pEnd - sigLength))
    {
        size_t i;
        for (i = 0; i < sigLength; i++)
        {
            if (mask[i] != '?' && sig[i] != pData[i])
                break;
        }

        // The for loop finished on its own accord
        if (i == sigLength)
        {
            return (void*)pData;
        }

        pData++;
    }

    throw ResolveException(fmt::sprintf("Signature could not be resolved - %s", Util::DataToHex(sig, sigLength).c_str()));
}

void* ModuleFuncResolver::GetFromRVA(uint64_t rva)
{
    return (void*)(m_moduleBase + rva);
}

ModuleFuncResolver& ModuleFuncResolver::GetResolver(const std::string & moduleName)
{
    if (Resolvers.count(moduleName) == 0)
    {
        auto logger = spdlog::get("logger");
        SPDLOG_DEBUG(logger, "No ModuleScan found for {}", moduleName);

        // Create modulescan
        HMODULE module = GetModuleHandle(Util::Widen(moduleName).c_str());
        if (module == nullptr)
        {
            throw std::runtime_error(fmt::format("Failed to get handle for {}", moduleName));
        }

        SPDLOG_DEBUG(logger, "Module {} already loaded, creating ModuleScan", moduleName);
        Resolvers.emplace(moduleName, module);
    }

    return Resolvers.at(moduleName);
}
}
