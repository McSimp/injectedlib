#pragma once

namespace ilib {

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

template<class Base>
class HookedFunction : public Base
{
private:
    bool m_hooked = false;
    typename Base::FuncPtrType m_hookedFunc;

public:
    using Base::Base;

    void Hook(typename Base::FuncPtrType detourFunc)
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

    ~HookedFunction()
    {
        if (m_hooked)
        {
            MH_RemoveHook(m_hookedFunc);
        }
    }
};

}
