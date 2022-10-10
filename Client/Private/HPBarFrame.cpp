#include "stdafx.h"
#include "..\Public\HPBarFrame.h"


CHPBarFrame::CHPBarFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CHPBarFrame::CHPBarFrame(const CHPBarFrame & rhs)
	: CUI(rhs)
{
}

HRESULT CHPBarFrame::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHPBarFrame::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	return S_OK;
}

void CHPBarFrame::Tick(_float fTimeDelta)
{

}

void CHPBarFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CHPBarFrame::Render()
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

CHPBarFrame * CHPBarFrame::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CHPBarFrame*		pInstance = new CHPBarFrame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHPBarFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CHPBarFrame::Clone(void * pArg)
{
	CHPBarFrame*		pInstance = new CHPBarFrame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CHPBarFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHPBarFrame::Free()
{
	__super::Free();
}
