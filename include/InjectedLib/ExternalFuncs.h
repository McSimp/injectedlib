#pragma once

namespace ilib {

template<typename T>
class ExternalFunction;

template<typename R, typename... Args>
class ExternalFunction<R(Args...)>
{
public:
    typedef typename std::add_pointer<R(Args...)>::type FuncPtrType;

    R operator()(Args... args)
    {
        return m_func(args...);
    }

protected:
    R(*m_func)(Args...) = nullptr;

    void SetFunctionPointer(void* ptr)
    {
        m_func = reinterpret_cast<R(*)(Args...)>(ptr);
    }
};

template<typename T>
class SigScanFunction;

template<typename R, typename... Args>
class SigScanFunction<R(Args...)> : public ExternalFunction<R(Args...)>
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
class OffsetFunction;

template<class R, class... Args>
class OffsetFunction<R(Args...)> : public ExternalFunction<R(Args...)>
{
public:
    OffsetFunction(const char* moduleName, uint64_t offsetFromImageBase)
    {
        auto& resolver = ModuleFuncResolver::GetResolver(moduleName);
        this->SetFunctionPointer(resolver.GetFromRVA(offsetFromImageBase));
    }
};

}
