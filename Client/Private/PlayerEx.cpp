#include "stdafx.h"
#include "..\Public\PlayerEx.h"
#include "GameInstance.h"

CPlayerEx::CPlayerEx(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerEx::CPlayerEx(const CPlayerEx& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerEx::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerEx::Initialize(void * pArg)
{
	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(TransformDesc));

	TransformDesc.fSpeedPerSec = 25.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(1.0f);

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Model"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Noise"), TEXT("Com_Texture"), (CComponent**)&m_pDissolveTexture)))
		return E_FAIL;

	m_bDead = false;
	
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerEx", TEXT("PlayerEx"), (CComponent**)&m_pModel)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ 0.05f,0.f,0.f,1.f });
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 90.f, _float3{ 0.f,1.f,0.f }, 180.f, _float3{ 0.f,0.f,1.f }, 0.f);

	return S_OK;
}

void CPlayerEx::Tick(_float fTimeDelta)
{
	
}

void CPlayerEx::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	
	m_fDissolveAcc += 0.3f * fTimeDelta;
	if (m_fDissolveAcc >= 0.67f)
	{
		m_bStartDissolve = false;
		m_bEndDissolve = true;
		m_fDissolveAcc = 0.f;
	}
	
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);

}

HRESULT CPlayerEx::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	_float4x4		WorldMatrix;

	XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()));

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);

	_uint		iNumMeshes = m_pModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
	
		if (m_bStartDissolve)
		{
			if (FAILED(m_pShaderCom->Begin(MODEL_NSTARTDISSOLVE)))
				return E_FAIL;

			m_pDissolveTexture->Set_SRV(m_pShaderCom, "g_DissolveTexture", 0);
			m_pShaderCom->Set_RawValue("g_fDissolveAcc", &m_fDissolveAcc, sizeof(float));

			if (FAILED(m_pModel->Render(i)))
				return E_FAIL;
		}

		else if (m_bEndDissolve)
		{
			if (FAILED(m_pShaderCom->Begin(MODEL_NENDDISSOLVE)))
				return E_FAIL;

			m_pDissolveTexture->Set_SRV(m_pShaderCom, "g_DissolveTexture", 0);
			m_pShaderCom->Set_RawValue("g_fDissolveAcc", &m_fDissolveAcc, sizeof(float));		
		}
		
		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

void CPlayerEx::StartDissolve()
{
	m_bStartDissolve = true;
	m_bEndDissolve = false;
	m_fDissolveAcc = 0.f;
}

void CPlayerEx::EndDissolve()
{
	m_bEndDissolve = true;
	m_bStartDissolve = false;
	m_fDissolveAcc = 0.f;
}

CMesh * CPlayerEx::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerEx*		pInstance = new CPlayerEx(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerEx"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerEx::Clone(void * pArg)
{
	CPlayerEx*		pInstance = new CPlayerEx(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerEx"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerEx::Free()
{
	__super::Free();
	Safe_Release(m_pDissolveTexture);
	Safe_Release(m_pModel);

}
