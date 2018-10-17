#pragma once

#include <functional>

namespace ilib {

enum CallingConvention
{
    _stdcall_, _cdecl_, _thiscall_, _fastcall_
};

template<typename T, CallingConvention Call>
class FuncPtr;

template<typename R, typename... Args>
class FuncPtr<R(*)(Args...), _cdecl_>
{
protected:
    static const CallingConvention CallingConvention = _cdecl_;
    typedef R(__cdecl *FuncPtrType)(Args...);
    FuncPtrType m_func;
};

template<typename R, typename... Args>
class FuncPtr<R(*)(Args...), _stdcall_>
{
protected:
    static const CallingConvention CallingConvention = _stdcall_;
    typedef R(__stdcall *FuncPtrType)(Args...);
    FuncPtrType m_func;
};

template<typename R, typename... Args>
class FuncPtr<R(*)(Args...), _fastcall_>
{
protected:
    static const CallingConvention CallingConvention = _fastcall_;
    typedef R(__stdcall *FuncPtrType)(Args...);
    FuncPtrType m_func;
};

template<typename R, typename Arg1, typename... Args>
class FuncPtr<R(*)(Arg1, Args...), _thiscall_>
{
protected:
    static const CallingConvention CallingConvention = _thiscall_;
    typedef R(__thiscall *FuncPtrType)(Arg1, Args...);
    typedef R(__fastcall *FastCallPtrType)(Arg1, void*, Args...);
    FuncPtrType m_func;
};

template<typename T, CallingConvention Call>
class BaseExternalFunction;

template<typename R, CallingConvention Call, typename... Args>
class BaseExternalFunction<R(*)(Args...), Call> : public FuncPtr<R(*)(Args...), Call>
{
public:
    auto operator()(Args... args) const
    {
        return this->m_func(std::forward<Args>(args)...);
    }

protected:
    void SetFunctionPointer(void* ptr)
    {
        this->m_func = reinterpret_cast<decltype(this->m_func)>(ptr);
    }
};

template<typename T, CallingConvention Call>
class ExternalFunction;

template<typename R, CallingConvention Call, class C, typename... Args>
class ExternalFunction<R(C::*)(Args...), Call> : public BaseExternalFunction<R(*)(Args...), Call>
{
};

template<typename R, CallingConvention Call, typename... Args>
class ExternalFunction<R(*)(Args...), Call> : public BaseExternalFunction<R(*)(Args...), Call>
{
};

template<typename T, CallingConvention Call = _cdecl_>
class SigScanFunction : public ExternalFunction<T, Call>
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

template<typename T, CallingConvention Call = _cdecl_>
class OffsetFunction : public ExternalFunction<T, Call>
{
public:
    OffsetFunction(const char* moduleName, uint64_t offsetFromImageBase)
    {
        auto& resolver = ModuleFuncResolver::GetResolver(moduleName);
        this->SetFunctionPointer(resolver.GetFromRVA(offsetFromImageBase));
    }
};

}
