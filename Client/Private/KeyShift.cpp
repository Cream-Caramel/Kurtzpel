#include "stdafx.h"
#include "..\Public\KeyShift.h"
#include "UI_Manager.h"

CKeyShift::CKeyShift(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CKeyShift::CKeyShift(const CKeyShift & rhs)
	: CUI(rhs)
{
}

HRESULT CKeyShift::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKeyShift::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_bDown = false;
	m_fDownAcc = 0.f;
	UM->AddKeyShift(this);

	return S_OK;
}

void CKeyShift::Tick(_float fTimeDelta)
{
	if (m_bDown)
	{
		m_fDownAcc += 1.f * fTimeDelta;
		if (m_fDownAcc >= 0.1f)
		{
			m_bDown = false;
			m_fDownAcc = 0.f;
		}
	}
}

void CKeyShift::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CKeyShift::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (m_bDown)
	{
		if (FAILED(m_pShaderCom->Begin(PASS_KEYDOWN)))
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

CKeyShift * CKeyShift::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CKeyShift*		pInstance = new CKeyShift(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CKeyShift"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CKeyShift::Clone(void * pArg)
{
	CKeyShift*		pInstance = new CKeyShift(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CKeyShift"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKeyShift::Free()
{
	__super::Free();
}
