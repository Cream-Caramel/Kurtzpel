#include "stdafx.h"
#include "..\Public\BossMPBarFrame.h"


CBossMPBarFrame::CBossMPBarFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CBossMPBarFrame::CBossMPBarFrame(const CBossMPBarFrame & rhs)
	: CUI(rhs)
{
}

HRESULT CBossMPBarFrame::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossMPBarFrame::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	return S_OK;
}

void CBossMPBarFrame::Tick(_float fTimeDelta)
{

}

void CBossMPBarFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CBossMPBarFrame::Render()
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

CBossMPBarFrame * CBossMPBarFrame::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBossMPBarFrame*		pInstance = new CBossMPBarFrame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBossMPBarFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBossMPBarFrame::Clone(void * pArg)
{
	CBossMPBarFrame*		pInstance = new CBossMPBarFrame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBossMPBarFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossMPBarFrame::Free()
{
	__super::Free();
}
