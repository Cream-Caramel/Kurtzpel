#include "stdafx.h"
#include "..\Public\HPBarFrame.h"
#include "Player.h"

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
	m_fNowPlayerHp = 100.f;
	m_fPrePlayerHp = 100.f;
	return S_OK;
}

void CHPBarFrame::Tick(_float fTimeDelta)
{

	if (m_fPrePlayerHp > m_fNowPlayerHp)
	{
		m_fPrePlayerHp -= 10.f * fTimeDelta;
		if (m_fPrePlayerHp <= m_fNowPlayerHp)
		{
			m_fPrePlayerHp = m_fNowPlayerHp;
		}
	}

	else
	{
		if (m_fNowPlayerHp < 100.f)
		{
			m_fNowPlayerHp += 3.f * fTimeDelta;
			m_fPrePlayerHp += 3.f * fTimeDelta;
			if (m_fNowPlayerHp >= 100.f)
			{
				m_fNowPlayerHp = 100.f;
				m_fPrePlayerHp = 100.f;
			}

			UM->Set_PlayerHp(m_fNowPlayerHp);
		}
	}
}

void CHPBarFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_fNowPlayerHp != (_float)UM->Get_PlayerHp())
		m_fNowPlayerHp = UM->Get_PlayerHp();


	m_fShaderNowHp = m_fNowPlayerHp / 100.f;
	m_fShaderPreHp = m_fPrePlayerHp / 100.f;
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
	m_pShaderCom->Set_RawValue("g_fNowPlayerHp", &m_fShaderNowHp, sizeof(float));
	m_pShaderCom->Set_RawValue("g_fPrePlayerHp", &m_fShaderPreHp, sizeof(float));
	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (m_iUIIndex == 1)
	{
		if (FAILED(m_pShaderCom->Begin(PASS_HPBARFRAME)))
			return E_FAIL;
	}

	else if (m_iUIIndex == 2)
	{
		if (FAILED(m_pShaderCom->Begin(PASS_HPBAR)))
			return E_FAIL;
	
	}
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
