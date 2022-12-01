#include "stdafx.h"
#include "..\Public\KeyE.h"
#include "UI_Manager.h"

CKeyE::CKeyE(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CKeyE::CKeyE(const CKeyE & rhs)
	: CUI(rhs)
{
}

HRESULT CKeyE::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKeyE::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_bDown = false;
	m_fDownAcc = 0.f;
	UM->AddKeyE(this);
	return S_OK;
}

void CKeyE::Tick(_float fTimeDelta)
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

void CKeyE::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CKeyE::Render()
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

CKeyE * CKeyE::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CKeyE*		pInstance = new CKeyE(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CKeyE"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CKeyE::Clone(void * pArg)
{
	CKeyE*		pInstance = new CKeyE(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CKeyE"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKeyE::Free()
{
	__super::Free();
}
