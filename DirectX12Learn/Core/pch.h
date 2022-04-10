#pragma once

#pragma warning(disable:4201) //警告屏蔽: 匿名结构和联合
#pragma warning(disable:4238) //警告屏蔽: 右值作为左值
#pragma warning(disable:4239) //警告屏蔽: 非const引用只能绑定到左值; 赋值操作符接受对非const对象的引用  
#pragma warning(disable:4324) //警告屏蔽: 结构体内存对齐

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#include <Windows.h>
#include <winrt/base.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <DirectXMath.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <memory>
#include <string>
#include <cwctype>
#include <exception>

#include <iostream>

#include <ppltasks.h>
#include <functional>


using namespace DirectX;