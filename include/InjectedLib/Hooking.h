#pragma once

#include <functional>

namespace ilib {

template<typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}


template<typename Base>
class HookedFunction : public Base
{
private:
    bool m_hooked = false;
    typename Base::FuncType* m_hookedFunc;

public:
    using Base::Base;

    void Hook(typename Base::FuncType* detourFunc)
    {
        if (m_hooked)
        {
            return;
        }

        auto logger = spdlog::get("logger");

        m_hookedFunc = this->m_stdFunc.target<typename Base::FuncType>();
        typename Base::FuncType* originalFunc = this->m_stdFunc.target<typename Base::FuncType>();

        MH_STATUS status = MH_CreateHookEx(m_hookedFunc, detourFunc, &originalFunc);
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

        // Update m_stdFunc to point to the original function
        this->m_stdFunc = originalFunc;

        m_hooked = true;
        SPDLOG_DEBUG(logger, "Hooked function at {} - trampoline location: {}", (void*)m_hookedFunc, (void*)originalFunc);
    }

    ~HookedFunction()
    {
        if (m_hooked)
        {
            MH_RemoveHook(m_hookedFunc);
            this->m_stdFunc = m_hookedFunc;
            m_hooked = false;
        }
    }
};

}
