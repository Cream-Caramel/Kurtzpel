#include "stdafx.h"
#include "..\Public\BossBarLine.h"
#include "UI_Manager.h"

CBossBarLine::CBossBarLine(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CBossBarLine::CBossBarLine(const CBossBarLine & rhs)
	: CUI(rhs)
{
}

HRESULT CBossBarLine::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossBarLine::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	return S_OK;
}

void CBossBarLine::Tick(_float fTimeDelta)
{
	if (!m_bDie)
	{
		if (UM->Get_BossDie())
			m_bDie = true;
	}
	else
	{
		m_fDieAcc += 1.f * fTimeDelta;
		if (m_fDieAcc >= 4.2f)
		{
			Set_Dead();
		}
	}
}

void CBossBarLine::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CBossBarLine::Render()
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

CBossBarLine * CBossBarLine::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBossBarLine*		pInstance = new CBossBarLine(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBossBarLine"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBossBarLine::Clone(void * pArg)
{
	CBossBarLine*		pInstance = new CBossBarLine(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBossBarLine"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossBarLine::Free()
{
	__super::Free();
}
