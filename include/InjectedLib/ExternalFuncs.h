#pragma once

#include <functional>

namespace ilib {


template<typename T>
class ExternalFunction
{
public:
    typedef T FuncType;

    template<typename... Args>
    auto operator()(Args... args) const
    {
        return m_stdFunc(std::forward<Args>(args)...);
    }

protected:
    void SetFunctionPointer(void* ptr)
    {
        m_stdFunc = reinterpret_cast<typename std::add_pointer<T>::type>(ptr);
    }

    std::function<T> m_stdFunc;
};

template<typename T>
class SigScanFunction : public ExternalFunction<T>
{
public:
    SigScanFunction(const char* moduleName, const char* signature, const char* mask)
    {
        auto& resolver = ModuleFuncResolver::GetResolver(moduleName);
        void* ptr = resolver.SigScan(signature, mask);

        // Skip past any leading 0xCC bytes to get the real function pointer
        unsigned char* funcData = static_cast<unsigned char*>(ptr);
        while (*funcData == 0xCC)
        {
            funcData++;
        }

        this->SetFunctionPointer(funcData);

        SPDLOG_DEBUG(spdlog::get("logger"), "Signature {} in {} found at {}", Util::DataToHex(signature, strlen(mask)), moduleName, static_cast<void*>(funcData));
    }
};

template<typename T>
class OffsetFunction : public ExternalFunction<T>
{
public:
    OffsetFunction(const char* moduleName, uint64_t offsetFromImageBase)
    {
        auto& resolver = ModuleFuncResolver::GetResolver(moduleName);
        this->SetFunctionPointer(resolver.GetFromRVA(offsetFromImageBase));
    }
};

}
