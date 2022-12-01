#include "stdafx.h"
#include "..\Public\PressKey.h"


CPressKey::CPressKey(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CPressKey::CPressKey(const CPressKey & rhs)
	: CUI(rhs)
{
}

HRESULT CPressKey::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPressKey::Initialize(void * pArg)
{	
	m_bDead = false;
	Identity = XMMatrixIdentity();
	Proj = XMMatrixTranspose(XMMatrixOrthographicLH(1280, 720, 0.f, 1.f));
	if (FAILED(__super::Add_Component(LEVEL_LOGO, L"PressKey", L"PressKey", (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scale(XMVectorSet(250.f, 100.f, 1.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ 10.f,-150.f,0.f,1.f });

	return S_OK;
}

void CPressKey::Tick(_float fTimeDelta)
{
	if (m_bUp)
	{
		m_fPressAcc += 1.f * fTimeDelta;
		if (m_fPressAcc >= 1.f)
		{
			m_bUp = false;
			m_fPressAcc = 1.f;
		}
	}
	else
	{
		m_fPressAcc -= 1.f * fTimeDelta;
		if (m_fPressAcc <= 0.f)
		{
			m_bUp = true;
			m_fPressAcc = 0.f;
		}
	}
}

void CPressKey::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CPressKey::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	
	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &Identity, sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &Proj, sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_fPressAcc", &m_fPressAcc, sizeof(_float));
	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(PASS_PRESS)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CPressKey::Ready_Components()
{
	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_UI"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

CPressKey * CPressKey::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPressKey*		pInstance = new CPressKey(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPressKey"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPressKey::Clone(void * pArg)
{
	CPressKey*		pInstance = new CPressKey(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPressKey"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPressKey::Free()
{
	__super::Free();
}
