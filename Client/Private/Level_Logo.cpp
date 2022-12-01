#include "stdafx.h"
#include "..\Public\Level_Logo.h"
#include "FadeInOut.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "UI_Manager.h"
#include "Pointer_Manager.h"
CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Logo::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(GI->Add_GameObjectToLayer(L"LogoScene", LEVEL_LOGO, L"Layer_UI")))
		return E_FAIL;

	if (FAILED(GI->Add_GameObjectToLayer(L"PressKey", LEVEL_LOGO, L"Layer_UI")))
		return E_FAIL;

	if (FAILED(GI->Add_GameObjectToLayer(L"FadeInOut", LEVEL_STATIC, L"Layer_UI")))
		return E_FAIL;

	UM->On_Fade();

	UM->Set_Fade(CFadeInOut::FADEIN);
	

	GI->PlayBGM(L"LogoBGM.wav", 0.4f);

	return S_OK;
}

void CLevel_Logo::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (GI->Key_Down(DIK_SPACE))
	{
		UM->On_Fade();
		UM->Set_Fade(CFadeInOut::FADEOUT);
		PM->Set_NextLevel(LEVEL_STAGE1);
	}
}

HRESULT CLevel_Logo::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Logo::Ready_Layer_BackGround(const _tchar * pLayerTag)
{
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	/*if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_BackGround"), LEVEL_GAMEPLAY, pLayerTag)))
		return E_FAIL;
*/
	Safe_Release(pGameInstance);

	return S_OK;
}

CLevel_Logo * CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Logo*		pInstance = new CLevel_Logo(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevel_Logo"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Logo::Free()
{
	__super::Free();




}


