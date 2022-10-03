#pragma once

namespace Client
{
	static const unsigned int g_iWinSizeX = 1280;
	static const unsigned int g_iWinSizeY = 720;

	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_GAMEPLAY, LEVEL_END };

#define IG CImGui_Manager::Get_Instance()
#define GI CGameInstance::Get_Instance()
}

extern HINSTANCE g_hInst;
extern HWND g_hWnd;

using namespace Client;


