#include "stdafx.h"
#include "..\Public\Level_Logo.h"

#include "GameInstance.h"
#include "Level_Static.h"
#include "Level_Loading.h"


CLevel_Static::CLevel_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Static::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	

	return S_OK;
}

void CLevel_Static::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);


	if (FAILED(pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_LOGO))))
		return;
	
	Safe_Release(pGameInstance);
}

HRESULT CLevel_Static::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}


CLevel_Static * CLevel_Static::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Static*		pInstance = new CLevel_Static(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevel_Static"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Static::Free()
{
	__super::Free();




}


