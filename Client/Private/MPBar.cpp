#include "stdafx.h"
#include "..\Public\MPBar.h"
#include "Player.h"

CMPBar::CMPBar(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CMPBar::CMPBar(const CMPBar & rhs)
	: CUI(rhs)
{
}

HRESULT CMPBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMPBar::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_fPlayerMp = 100.f;
	m_fShaderMp = 0.f;

	return S_OK;
}

void CMPBar::Tick(_float fTimeDelta)
{
	m_fPlayerMp = (_float)UM->Get_PlayerMp();
	if (m_fPlayerMp < 100.f)
	{
		if (UM->Get_PlayerState() == CPlayer::IDLE || UM->Get_PlayerState() == CPlayer::RUN)
		{
			m_fPlayerMp += 8.f * fTimeDelta;
			UM->Set_PlayerMp(m_fPlayerMp);

			if (m_fPlayerMp >= 100.f)
				m_fPlayerMp = 100.f;
		}
		
	}
}

void CMPBar::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	
	m_fShaderMp = m_fPlayerMp / 100.f;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CMPBar::Render()
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

	if (FAILED(m_pShaderCom->Begin(PASS_MPBAR)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CMPBar * CMPBar::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CMPBar*		pInstance = new CMPBar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CMPBar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CMPBar::Clone(void * pArg)
{
	CMPBar*		pInstance = new CMPBar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMPBar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMPBar::Free()
{
	__super::Free();
}
