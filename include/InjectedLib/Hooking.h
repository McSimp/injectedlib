#pragma once

#include <functional>

namespace ilib {

template<typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

template<int UniqueVal, class C, typename R, typename Arg1, typename... Args>
struct MemFnCallback
{
    static R(C::*F)(Arg1, Args...);
    static C* Instance;

    typedef R(__fastcall *FastCallPtr)(Arg1, void*, Args...);
    typedef R(__stdcall *StdCallPtr)(Arg1, Args...);
    typedef R(__cdecl *CdeclPtr)(Arg1, Args...);

    auto GetForwarderFunc() const
    {
        return [](Arg1 a, Args... b) -> R {
            return (Instance->*F)(a, b...);
        };
    }

    // TODO: This is used as a fake __thiscall, so won't work if being used for real __fastcall
    operator FastCallPtr() const
    {
        return [](Arg1 a, void*, Args... b) -> R {
            return (Instance->*F)(a, b...);
        };
    }

    operator StdCallPtr() const
    {
        return GetForwarderFunc();
    }

    operator CdeclPtr() const
    {
        return GetForwarderFunc();
    }

    operator void*() const
    {
        return static_cast<CdeclPtr>(GetForwarderFunc());
    }
};

template<int UniqueVal, class C, typename R, typename Arg1, typename... Args>
R(C::*(MemFnCallback<UniqueVal, C, R, Arg1, Args...>::F))(Arg1, Args...);

template<int UniqueVal, class C, typename R, typename Arg1, typename... Args>
C* MemFnCallback<UniqueVal, C, R, Arg1, Args...>::Instance;

template<int UniqueVal, class C, typename R, typename Arg1, typename... Args>
static auto memberCallback(R(C::*f)(Arg1, Args...), C* instance)
{
    using ThisMemFnCallback = MemFnCallback<UniqueVal, C, R, Arg1, Args...>;
    ThisMemFnCallback::F = f;
    ThisMemFnCallback::Instance = instance;
    return ThisMemFnCallback();
}

#define ILIB_MEMBER_CALLBACK(f, instance) ilib::memberCallback<__COUNTER__>(f, instance)

template<typename Base>
class HookedFunction : public Base
{
private:
    bool m_hooked = false;
    typename Base::FuncPtrType m_hookedFunc;

    void HookInternal(void* detourFunc)
    {
        if (m_hooked)
        {
            return;
        }

        auto logger = spdlog::get("logger");

        m_hookedFunc = this->m_func;

        MH_STATUS status = MH_CreateHookEx(m_hookedFunc, detourFunc, &this->m_func);
        if (status != MH_OK)
        {
            logger->critical("MH_CreateHook returned {}", status);
            throw std::runtime_error("Failed to hook function");
        }

        status = MH_EnableHook(m_hookedFunc);
        if (status != MH_OK)
        {
            logger->critical("MH_EnableHook returned {}", status);
            throw std::runtime_error("Failed to enable hook");
        }

        m_hooked = true;
        SPDLOG_DEBUG(logger, "Hooked function at {} - trampoline location: {}", (void*)m_hookedFunc, (void*)this->m_func);
    }

public:
    using Base::Base;

    void Hook(typename Base::FuncPtrType detourFunc)
    {
        HookInternal(detourFunc);
    }

    // For __thiscall functions, we want to be able to add a hook function
    // that's declared as a __fastcall, so we can access ecx without needing
    // some asm hax. For all other types, this overload is not available.
    template<typename B=Base>
    void Hook(typename std::enable_if<B::CallingConvention == _thiscall_, typename B::FastCallPtrType>::type detourFunc)
    {
        HookInternal(detourFunc);
    }

    ~HookedFunction()
    {
        if (m_hooked)
        {
            MH_RemoveHook(m_hookedFunc);
            this->m_func = m_hookedFunc;
            m_hooked = false;
        }
    }
};

}
