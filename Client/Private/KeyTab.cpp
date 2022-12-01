#include "stdafx.h"
#include "..\Public\KeyTab.h"
#include "UI_Manager.h"

CKeyTab::CKeyTab(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CKeyTab::CKeyTab(const CKeyTab & rhs)
	: CUI(rhs)
{
}

HRESULT CKeyTab::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKeyTab::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_bDown = false;
	m_fDownAcc = 0.f;
	UM->AddKeyTab(this);
	return S_OK;
}

void CKeyTab::Tick(_float fTimeDelta)
{
	if (m_bDown)
	{
		m_fDownAcc += 1.f * fTimeDelta;
		if (m_fDownAcc >= 0.1f)
		{
			m_bDown = false;
			m_fDownAcc = 0.f;
		}
	}
}

void CKeyTab::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CKeyTab::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;
	if (m_bDown)
	{
		if (FAILED(m_pShaderCom->Begin(PASS_KEYDOWN)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Begin(PASS_DEFAULT)))
			return E_FAIL;
	}
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CKeyTab * CKeyTab::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CKeyTab*		pInstance = new CKeyTab(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CKeyTab"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CKeyTab::Clone(void * pArg)
{
	CKeyTab*		pInstance = new CKeyTab(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CKeyTab"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKeyTab::Free()
{
	__super::Free();
}
