#pragma once

#include "GameCore.h"

#pragma comment(lib, "runtimeobject.lib")

namespace GameCore
{
	HWND g_Hwnd = nullptr;
	UINT32 g_DisplayWidth = 1920;
	UINT32 g_DisplayHeight = 1080;
	wstring g_ClassName = L"";

	HWND GetHwnd()
	{
		return g_Hwnd;
	}

	UINT32 GetDisplayWidth()
	{
		return g_DisplayWidth;
	}

	UINT32 GetDisplayHeight()
	{
		return g_DisplayHeight;
	}

	wstring GetGameClassName()
	{
		return g_ClassName;
	}

	bool IGameApp::IsDone()
	{
		return false;
	}

	void IGameApp::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		com_ptr<IDXGIAdapter1> adapter;

		com_ptr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(WINRT_IID_PPV_ARGS(factory6))))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					WINRT_IID_PPV_ARGS(adapter)));
				++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					//不要选择 software adapter，如果有需求从命令行传入"/warp"
					continue;
				}

				// 这里只是检测是否能够创建ID3D12Device
				if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		if (adapter.get() == nullptr)
		{
			for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, adapter.put())); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					//不要选择 software adapter，如果有需求从命令行传入"/warp"
					continue;
				}

				// 这里只是检测是否能够创建ID3D12Device
				if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.detach();
	}

	void InitializeApplication(IGameApp& game)
	{
		game.Startup();
	}

	bool UpdateApplication(IGameApp& game)
	{
		game.Update(0);
		game.RenderScene();
		return !game.IsDone();
	}

	void TerminateApplication(IGameApp& game)
	{
		game.Cleanup();
	}

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	int RunApplication(IGameApp& InApp, const wchar_t* InClassName, HINSTANCE InInstance, int InArgc, WCHAR** InArgv)
	{
		g_ClassName = InClassName;
		if (!XMVerifyCPUSupport())
			return 1;

		//注册窗口类
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = InInstance;
		wcex.hIcon = LoadIcon(InInstance, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(DKGRAY_BRUSH);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = InClassName;
		wcex.hIconSm = LoadIcon(InInstance, IDI_APPLICATION);

		if (!RegisterClassEx(&wcex))
		{
			MessageBox(0, L"RegisterClass Failed.", 0, 0);
			return 0;
		}

		//创建窗口类
		RECT rc = { 0, 0, (LONG)g_DisplayWidth, (LONG)g_DisplayHeight };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		g_Hwnd = CreateWindow(InClassName, InClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, InInstance, nullptr);

		if (!g_Hwnd)
		{
			MessageBox(0, L"CreateWindow Failed.", 0, 0);
			return 0;
		}

		InitializeApplication(InApp);

		ShowWindow(g_Hwnd, InArgc);

		do
		{
			MSG msg = {};
			bool done = false;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
					done = true;
			}
			if (done)
				break;
		} while (UpdateApplication(InApp));

		TerminateApplication(InApp);

		cout << "Game End!" << endl;
		return 1;
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_SIZE:
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			return 0;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}

