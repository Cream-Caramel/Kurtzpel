#include "stdafx.h"
#include "..\Public\BossHPBarFrame.h"
#include "Player.h"

CBossHPBarFrame::CBossHPBarFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CBossHPBarFrame::CBossHPBarFrame(const CBossHPBarFrame & rhs)
	: CUI(rhs)
{
}

HRESULT CBossHPBarFrame::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossHPBarFrame::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_fNowBossHp = UM->Get_BossHp();
	m_fPreBossHp = UM->Get_BossHp();
	return S_OK;
}

void CBossHPBarFrame::Tick(_float fTimeDelta)
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

 	m_fNowBossHp = UM->Get_BossHp();

	if (m_fPreBossHp > m_fNowBossHp)
	{
		m_fPreBossHp -= 20.f * fTimeDelta;
		if (m_fPreBossHp <= m_fNowBossHp)
		{
			m_fPreBossHp = m_fNowBossHp;
		}
	}

	else
	{
		m_fPreBossHp = m_fNowBossHp;
		if (UM->Get_BossMp() >= 100.f)
		{
			m_fNowBossHp += 3.f * fTimeDelta;
			m_fPreBossHp = m_fNowBossHp;
			if (m_fNowBossHp >= UM->Get_BossMaxHp())
			{
				m_fNowBossHp = UM->Get_BossMaxHp();
				m_fPreBossHp = m_fNowBossHp;
			}

			UM->Set_BossHp(m_fNowBossHp);
		}
	}
	

	
}

void CBossHPBarFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;


	m_fShaderNowHp = m_fNowBossHp / UM->Get_BossMaxHp();
	m_fShaderPreHp = m_fPreBossHp / UM->Get_BossMaxHp();
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CBossHPBarFrame::Render()
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
		if (FAILED(m_pShaderCom->Begin(PASS_BOSSHPBAR)))
			return E_FAIL;
	
	}
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CBossHPBarFrame * CBossHPBarFrame::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBossHPBarFrame*		pInstance = new CBossHPBarFrame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBossHPBarFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBossHPBarFrame::Clone(void * pArg)
{
	CBossHPBarFrame*		pInstance = new CBossHPBarFrame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBossHPBarFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossHPBarFrame::Free()
{
	__super::Free();
}
