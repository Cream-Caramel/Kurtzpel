#include "stdafx.h"
#include "..\Public\BossMPBar.h"
#include "Player.h"

CBossMPBar::CBossMPBar(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CBossMPBar::CBossMPBar(const CBossMPBar & rhs)
	: CUI(rhs)
{
}

HRESULT CBossMPBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossMPBar::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_fBossMp = UM->Get_BossMp();
	m_fShaderMp = 0.f;

	return S_OK;
}

void CBossMPBar::Tick(_float fTimeDelta)
{
	m_fBossMp = UM->Get_BossMp();
}

void CBossMPBar::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	
	m_fShaderMp = m_fBossMp / 100.f;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CBossMPBar::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_fPlayerMp", &m_fShaderMp, sizeof(_float));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(PASS_BOSSMPBAR)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CBossMPBar * CBossMPBar::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBossMPBar*		pInstance = new CBossMPBar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBossMPBar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBossMPBar::Clone(void * pArg)
{
	CBossMPBar*		pInstance = new CBossMPBar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBossMPBar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossMPBar::Free()
{
	__super::Free();
}
