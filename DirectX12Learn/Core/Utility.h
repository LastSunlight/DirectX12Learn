#pragma once
#include <stdexcept>
#include "pch.h"

#define WINRT_IID_PPV_ARGS(ppType) __uuidof(ppType), ppType.put_void()

#define MEM_KiB(kib)		   (kib * 1024)
#define MEM_MiB(mib)	MEM_KiB(mib * 1024)
#define MEM_GiB(gib)	MEM_MiB(gib * 1024)

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}