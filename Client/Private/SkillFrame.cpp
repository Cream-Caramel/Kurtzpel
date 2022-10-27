#include "stdafx.h"
#include "..\Public\SkillFrame.h"


CSkillFrame::CSkillFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CSkillFrame::CSkillFrame(const CSkillFrame & rhs)
	:CUI(rhs)
{
}

HRESULT CSkillFrame::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkillFrame::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	Setting();
	UM->Add_SkillFrame(this);
	return S_OK;
}

void CSkillFrame::Tick(_float fTimeDelta)
{
	if (m_bCoolTime)
	{
		m_fCoolTime += 1.f * fTimeDelta;
		if (m_fCoolTime >= m_fMaxCoolTime)
		{
			m_bCoolTimeOn = true;
			m_bCoolTime = false;
			m_fCoolTime = 0.f;
		}
	}

	if (m_bCoolTimeOn)
	{
		if (!m_bCoolTimeOn2)
		{
			m_fCoolTimeOn += 1.f * fTimeDelta;
			if (m_fCoolTimeOn >= m_fMaxCoolTimeOn)
			{
				m_bCoolTimeOn2 = true;
			}
		}
		else
		{
			m_fCoolTimeOn -= 1.f * fTimeDelta;
			if (m_fCoolTimeOn <= 0.f)
			{
				m_fCoolTimeOn = 0.f;
				m_bCoolTimeOn = false;
				m_bCoolTimeOn2 = false;
			}
		}
	}
}

void CSkillFrame::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	ShaderCoolTime = m_fCoolTime / m_fMaxCoolTime;
	ShaderCoolTimeOn = m_fCoolTimeOn / m_fMaxCoolTimeOn;
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CSkillFrame::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_fCoolTime", &ShaderCoolTime, sizeof(float));
	m_pShaderCom->Set_RawValue("g_fCoolTimeOn", &ShaderCoolTimeOn, sizeof(float));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	
	if (m_bCoolTime)
	{
		if (FAILED(m_pShaderCom->Begin(PASS_COOLTIME)))
			return E_FAIL;
	}

	else if (m_bCoolTimeOn)
	{
		if (FAILED(m_pShaderCom->Begin(PASS_COOLTIMEON)))
			return E_FAIL;
	}

	else if (!UM->Get_UseSkill())
	{
		if (FAILED(m_pShaderCom->Begin(PASS_IMPOSSIBLE)))
			return E_FAIL;
	}

	else
	{
		if (FAILED(m_pShaderCom->Begin(PASS_DEFAULT)))
			return E_FAIL;
	}

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CSkillFrame::Setting()
{
	if (m_UIInfo->UIPosX == 230)
	{
		m_fCoolTime = 0.f;
		m_fMaxCoolTime = 8.f;		
		m_iIndex = 0;
	}
	else if (m_UIInfo->UIPosX == 290)
	{
		m_fCoolTime = 0.f;
		m_fMaxCoolTime = 2.f;	
		m_iIndex = 1;
	}
	else if (m_UIInfo->UIPosX == 350)
	{
		m_fCoolTime = 0.f;
		m_fMaxCoolTime = 10.f;	
		m_iIndex = 2;
	}
	else if (m_UIInfo->UIPosX == 320)
	{
		m_fCoolTime = 0.f;
		m_fMaxCoolTime = 10.f;	
		m_iIndex = 3;
	}
	else if (m_UIInfo->UIPosX == 260)
	{
		m_fCoolTime = 0.f;
		m_fMaxCoolTime = 2.f;	
		m_iIndex = 4;
	}

	m_fCoolTimeOn = 0.f;
	m_fMaxCoolTimeOn = 0.3f;
	m_bCoolTime = false;
	m_bCoolTimeOn = false;
	m_bCoolTimeOn2 = false;
}

CSkillFrame * CSkillFrame::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSkillFrame*		pInstance = new CSkillFrame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CSkillFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CSkillFrame::Clone(void * pArg)
{
	CSkillFrame*		pInstance = new CSkillFrame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CSkillFrame"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkillFrame::Free()
{
	__super::Free();
}
