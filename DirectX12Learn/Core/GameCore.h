#pragma once
#include "pch.h"

using namespace std;

namespace GameCore
{
	HWND GetHwnd();

	class IGameApp
	{

	public:
		virtual ~IGameApp() = default;
		virtual void Startup() = 0;
		virtual void Cleanup() = 0;

		virtual bool IsDone();

		virtual void Update(float deltaTime) = 0;

		virtual void RenderScene() = 0;

		virtual void RenderUI(class GraphicsContext&) {};

		virtual bool RequiresRaytracingSupport() const { return false; }
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
