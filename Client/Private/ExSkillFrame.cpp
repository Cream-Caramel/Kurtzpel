#include "stdafx.h"
#include "..\Public\ExSkillFrame.h"


CExSkillFrame::CExSkillFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CExSkillFrame::CExSkillFrame(const CExSkillFrame & rhs)
	: CUI(rhs)
{
}

HRESULT CExSkillFrame::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CExSkillFrame::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	
	return S_OK;
}

void CExSkillFrame::Tick(_float fTimeDelta)
{

}

void CExSkillFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CExSkillFrame::Render()
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

CExSkillFrame * CExSkillFrame::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CExSkillFrame*		pInstance = new CExSkillFrame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CExSkillFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CExSkillFrame::Clone(void * pArg)
{
	CExSkillFrame*		pInstance = new CExSkillFrame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CExSkillFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CExSkillFrame::Free()
{
	__super::Free();
}
