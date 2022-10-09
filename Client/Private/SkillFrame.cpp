#include "stdafx.h"
#include "..\Public\SkillFrame.h"


CSkillFrame::CSkillFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CSkillFrame::CSkillFrame(const CSkillFrame & rhs)
	:CUI(rhs)
{
}

HRESULT CSkillFrame::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkillFrame::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_fCoolTime = 0.f;
	m_fMaxCoolTime = 10.f;
	m_bCoolTime = true;
	m_bUse = false;
	return S_OK;
}

void CSkillFrame::Tick(_float fTimeDelta)
{
	m_fCoolTime += 1.f * fTimeDelta;
	if (m_fCoolTime >= m_fMaxCoolTime)
	{
		m_fCoolTime = 0.f;
	}
}

void CSkillFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CSkillFrame::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	_float a = m_fCoolTime / m_fMaxCoolTime;
	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_fPercent", &a, sizeof(float));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (m_bUse)
	{
		if (FAILED(m_pShaderCom->Begin(2)))
			return E_FAIL;
	}

	else if (m_bCoolTime)
	{
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
	}

	

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CSkillFrame * CSkillFrame::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSkillFrame*		pInstance = new CSkillFrame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CSkillFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CSkillFrame::Clone(void * pArg)
{
	CSkillFrame*		pInstance = new CSkillFrame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CSkillFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkillFrame::Free()
{
	__super::Free();
}
