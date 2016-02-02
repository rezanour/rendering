#pragma once

#define NOMINMAX
#include <Windows.h>
#include <d3d11_2.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <DirectXMath.h>
using namespace DirectX;

#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include <memory>

class NonCopyable
{
protected:
    NonCopyable() {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator= (const NonCopyable&) = delete;
};

#define CHECKHR(x) \
{ \
    HRESULT hr##__LINE__ = (x); \
    if (FAILED(hr##__LINE__)) \
    { \
        assert(false); \
        return hr##__LINE__; \
    } \
}
