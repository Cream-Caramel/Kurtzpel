#include "stdafx.h"
#include "..\Public\Player.h"
#include "GameInstance.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	for (auto& iter : m_pAnimModel)
		iter = nullptr;
}

CPlayer::CPlayer(const CPlayer& rhs)
	:CAnimMesh(rhs)
{
}

HRESULT CPlayer::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_pAnimModel[PARTS_PLAYER] = m_pModelCom;
	Safe_AddRef(m_pModelCom);
	_tchar* a = L"Top";
	_tchar* b = L"Bottom";

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Top", TEXT("Top"), (CComponent**)&m_pAnimModel[PARTS_TOP])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Bottom", TEXT("Bottom"), (CComponent**)&m_pAnimModel[PARTS_BOTTOM])))
		return E_FAIL;
		

	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
}

void CPlayer::LateTick(_float fTimeDelta)
{
	for(int i = 0; i < PARTS_END; ++i)
	{
		if(m_pAnimModel[i] != nullptr)
			m_pAnimModel[i]->Play_Animation(fTimeDelta, m_pAnimModel[i]);
	}

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	/*m_pModelCom->SetNextIndex(m_iAniIndex);
	m_pModelCom->SetChangeBool(true);*/
}

HRESULT CPlayer::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;


	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	for (int i = 0; i < PARTS_END; ++i)
	{
		if (m_pAnimModel[i] != nullptr)
		{
			_uint		iNumMeshes = m_pAnimModel[i]->Get_NumMeshes();
			for (_uint j = 0; j < iNumMeshes; ++j)
			{
				if (FAILED(m_pAnimModel[i]->SetUp_OnShader(m_pShaderCom, m_pAnimModel[i]->Get_MaterialIndex(j), TEX_DIFFUSE, "g_DiffuseTexture")))
					return E_FAIL;

				if (FAILED(m_pAnimModel[i]->Render(m_pShaderCom, j)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}

CAnimMesh * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel[PARTS_PLAYER]);
	Safe_Release(m_pAnimModel[PARTS_TOP]);
	Safe_Release(m_pAnimModel[PARTS_BOTTOM]);

}
