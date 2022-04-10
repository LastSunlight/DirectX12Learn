#pragma once

#include "GameCore.h"

#pragma comment(lib, "runtimeobject.lib")

namespace GameCore
{
	extern HWND g_Hwnd = nullptr;
	UINT32 g_DisplayWidth = 1920;
	UINT32 g_DisplayHeight = 1080;


	HWND GetHwnd()
	{
		return g_Hwnd;
	}

	bool IGameApp::IsDone()
	{
		return false;
	}

	void InitializeApplication(IGameApp& game)
	{
		
	}

	bool UpdateApplication(IGameApp& game)
	{
		return !game.IsDone();
	}

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	int RunApplication(IGameApp& InApp, const wchar_t* InClassName, HINSTANCE InInstance, int InArgc, WCHAR** InArgv)
	{
		if (!XMVerifyCPUSupport())
			return 1;

		Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
		if(FAILED(InitializeWinRT))
		{
			cout << "InitializeWinRT Failed!" << endl;
			return 0;
		}

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
		}
		while (UpdateApplication(InApp));

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

