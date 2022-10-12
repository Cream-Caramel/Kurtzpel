#include "stdafx.h"
#include "..\Public\SkillIconShift.h"


CSkillIconShift::CSkillIconShift(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CSkillIconShift::CSkillIconShift(const CSkillIconShift & rhs)
	: CUI(rhs)
{
}

HRESULT CSkillIconShift::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkillIconShift::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	return S_OK;
}

void CSkillIconShift::Tick(_float fTimeDelta)
{

}

void CSkillIconShift::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CSkillIconShift::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(PASS_SKILLICON)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CSkillIconShift * CSkillIconShift::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSkillIconShift*		pInstance = new CSkillIconShift(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CSkillIconShift"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CSkillIconShift::Clone(void * pArg)
{
	CSkillIconShift*		pInstance = new CSkillIconShift(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CSkillIconShift"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkillIconShift::Free()
{
	__super::Free();
}
