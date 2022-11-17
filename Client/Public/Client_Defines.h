#pragma once
#include "Engine_Defines.h"
namespace Client
{
	static const unsigned int g_iWinSizeX = 1280;
	static const unsigned int g_iWinSizeY = 720;

	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_STAGE1, LEVEL_STAGE2, LEVEL_STAGE3, LEVEL_STAGE4, LEVEL_END };
	enum UIPASS { PASS_DEFAULT, PASS_COOLTIME, PASS_IMPOSSIBLE, PASS_COOLTIMEON, PASS_KEYDOWN, PASS_HPBARFRAME, PASS_HPBAR, 
		PASS_BOSSHPBAR, PASS_EXGAUGE, PASS_MPBAR, PASS_BOSSMPBAR, PASS_SKILLICON,PASS_TRAIL, PASS_END };
	enum ANIMMODELPASS {ANIM_DEFAULT, ANIM_NDEFAULT, ANIM_PATTERN, ANIM_NPATTERN, ANIM_HIT, ANIM_NHIT, ANIM_FINISH, ANIM_NFINISH, ANIM_DISSOLVE, ANIM_NDISSOLVE
		, ANIM_GOLEMPATTERN, ANIM_END};
	enum MODELPASS {MODEL_DEFAULT, MODEL_NDEFAULT, MODEL_NONLIGHT, MODEL_ENDDISSOLVE, MODEL_NENDDISSOLVE, MODEL_STARTDISSOLVE, MODEL_NSTARTDISSOLVE, MODEL_END};
	enum INSTANCEPASS {INSTANCEMODEL_DEFAULT, INSTANCEMODEL_NDEFAULT, INSTANCEMODEL_END};
	enum EFFECTPASS {EFFECT_DEFAULT, EFFECT_NDEFAULT, EFFECT_ENDPASS, EFFECT_END};
	enum SOUND {SD_PLAYER1, SD_PLAYER2, SD_PLAYERVOICE, SD_PLAYERSKILL1, SD_PLAYERSKILL2, SD_MONSTER1, SD_MONSTER2, SD_MONSTERVOICE, SD_UI, SD_SYSTEM, SD_BGM, SD_END};

	typedef struct tagPosInfo
	{
		_float3 vPos;
		_float fCamSpeed;
		_float fStopLimit;
	}POSINFO;

	typedef struct tagLookInfo
	{
		_float3 vPos;
		_float fCamSpeed;
		_float fStopLimit;
	}LOOKINFO;

#define GI CGameInstance::Get_Instance()
#define RM CRelease_Manager::Get_Instance()
#define PM CPointer_Manager::Get_Instance()
#define UM CUI_Manager::Get_Instance()
#define CM CCollider_Manager::Get_Instance()
#define CRM CCamera_Manager::Get_Instance()
#define PTM CParticle_Manager::Get_Instance()
#define FONT GI->Render_Fonts(TEXT("Font_Nexon"),

}



extern HINSTANCE g_hInst;
extern HWND g_hWnd;

using namespace Client;


