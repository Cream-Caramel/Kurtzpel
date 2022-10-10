#include "stdafx.h"
#include "..\Public\SkillIconR.h"


CSkillIconR::CSkillIconR(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CSkillIconR::CSkillIconR(const CSkillIconR & rhs)
	: CUI(rhs)
{
}

HRESULT CSkillIconR::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkillIconR::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	return S_OK;
}

void CSkillIconR::Tick(_float fTimeDelta)
{

}

void CSkillIconR::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CSkillIconR::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CSkillIconR * CSkillIconR::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSkillIconR*		pInstance = new CSkillIconR(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CSkillIconR"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CSkillIconR::Clone(void * pArg)
{
	CSkillIconR*		pInstance = new CSkillIconR(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CSkillIconR"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkillIconR::Free()
{
	__super::Free();
}
