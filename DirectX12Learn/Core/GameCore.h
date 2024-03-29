﻿#pragma once
#include "pch.h"

using namespace std;

namespace GameCore
{
	HWND GetHwnd();
	UINT32 GetDisplayWidth();
	UINT32 GetDisplayHeight();
	wstring GetGameClassName();

	class IGameApp
	{

	public:
		virtual ~IGameApp() = default;
		virtual void Startup() = 0;
		virtual void Cleanup() = 0;

		virtual void Update(float deltaTime) = 0;
		virtual void RenderScene() = 0;

		virtual bool IsDone();
		virtual void SetWindowTitle() {};
	public:
		void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = true);
	};
}


namespace  GameCore
{
	int RunApplication(IGameApp& InApp, const wchar_t* InClassName, HINSTANCE InInstance, int InArgc, WCHAR** InArgv);
}

#define CREATE_APPLICATION( app_class ) \
	int wmain(int InArgc, WCHAR* InArgv[]) \
	{ \
	    HINSTANCE hInst = GetModuleHandle(0); \
		return GameCore::RunApplication( app_class(), L#app_class, hInst, InArgc, InArgv ); \
	}
