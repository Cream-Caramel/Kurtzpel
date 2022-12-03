#include "stdafx.h"
#include "..\Public\FadeInOut.h"
#include "UI_Manager.h"
#include "Pointer_Manager.h"
CFadeInOut::CFadeInOut(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CFadeInOut::CFadeInOut(const CFadeInOut & rhs)
	: CUI(rhs)
{
}

HRESULT CFadeInOut::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFadeInOut::Initialize(void * pArg)
{	
	m_bDead = false;
	Identity = XMMatrixIdentity();
	Proj = XMMatrixTranspose(XMMatrixOrthographicLH(1280, 720, 0.f, 1.f));
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"FadeInOut", L"FadeInOut", (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scale(XMVectorSet(1280.f, 720.f, 1.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ 0.f,0.f,0.f,1.f });

	UM->Add_FadeInOut(this);

	return S_OK;
}

void CFadeInOut::Tick(_float fTimeDelta)
{
	if (m_bFade)
	{
		if (m_eFadeIn == FADEIN)
		{
			m_fFadeAcc -= 0.5f * fTimeDelta;
			if (m_fFadeAcc <= 0.f)
			{
				m_bFade = false;
				m_fFadeAcc = 0.f;
			}
		}
		else
		{
			m_fFadeAcc += 0.5f * fTimeDelta;
			GI->VolumeDown(SD_BGM, m_fFadeAcc);
			if (m_fFadeAcc >= 1.f)
			{
				m_fFadeAcc = 1.f;
				m_bFade = false;
				
				PM->Change_Level(m_pDevice, m_pContext);
			}
		}

	}
}

void CFadeInOut::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_FRONTUI, this);
}

HRESULT CFadeInOut::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	
	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &Identity, sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &Proj, sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_fFadeAcc", &m_fFadeAcc, sizeof(_float));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(PASS_FADE)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}


HRESULT CFadeInOut::Ready_Components()
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

CFadeInOut * CFadeInOut::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CFadeInOut*		pInstance = new CFadeInOut(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CFadeInOut"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CFadeInOut::Clone(void * pArg)
{
	CFadeInOut*		pInstance = new CFadeInOut(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CFadeInOut"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFadeInOut::Free()
{
	__super::Free();
}
