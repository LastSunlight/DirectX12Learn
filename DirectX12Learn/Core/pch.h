#pragma once

#pragma warning(disable:4201) //��������: �����ṹ������
#pragma warning(disable:4238) //��������: ��ֵ��Ϊ��ֵ
#pragma warning(disable:4239) //��������: ��������
#pragma warning(disable:4324) //��������: �ṹ���ڴ����

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#include <Windows.h>
#include <wrl/client.h>
#include <wrl/event.h>

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