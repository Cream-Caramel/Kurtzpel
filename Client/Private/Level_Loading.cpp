#include "stdafx.h"
#include "..\Public\Level_Loading.h"
#include "GameInstance.h"

#include "Level_Logo.h"
#include "Level_Stage1.h"
#include "Level_Static.h"
#include "Level_Stage2.h"
#include "Level_Stage3.h"
#include "Level_Stage4.h"
#include "Loader.h"


CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevel)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_eNextLevel = eNextLevel;

	m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevel);
	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void CLevel_Loading::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_pLoader->Get_Finished())
	{
		
			CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
			Safe_AddRef(pGameInstance);

			CLevel*		pNewLevel = nullptr;

			switch (m_eNextLevel)
			{
			case LEVEL_STATIC:
				pNewLevel = CLevel_Static::Create(m_pDevice, m_pContext);
				break;
			case LEVEL_LOGO:
				pNewLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
				break;
			case LEVEL_STAGE1:
				pNewLevel = CLevel_Stage1::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE2:
				pNewLevel = CLevel_Stage2::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE3:
				pNewLevel = CLevel_Stage3::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE4:
				pNewLevel = CLevel_Stage4::Create(m_pDevice, m_pContext);
				break;
			}

			if (nullptr == pNewLevel)
				goto except;

			if (FAILED(pGameInstance->Open_Level(m_eNextLevel, pNewLevel)))
				goto except;

		except:
			Safe_Release(pGameInstance);
		
	}
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	SetWindowText(g_hWnd, m_pLoader->Get_LoadingText());

	return S_OK;
}

CLevel_Loading * CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLevel_Loading*		pInstance = new CLevel_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Created : CMainApp"));
		Safe_Release(pInstance);
	}

	return pInstance;

}


void CLevel_Loading::Free()
{
	__super::Free();

	Safe_Release(m_pLoader);

}


