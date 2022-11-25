#include "stdafx.h"
#include "..\Public\PlayerRageSword.h"
#include "GameInstance.h"

CPlayerRageSword::CPlayerRageSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerRageSword::CPlayerRageSword(const CPlayerRageSword& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerRageSword::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerRageSword::Initialize(void * pArg)
{
	m_RageSowrdInfo = (*(RAGESOWRD*)pArg);

	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(TransformDesc));

	TransformDesc.fSpeedPerSec = 25.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(1.0f);

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Model"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Noise"), TEXT("Com_Texture"), (CComponent**)&m_pDissolveTexture)))
		return E_FAIL;

	m_bDead = false;
	
	/* For.Com_Model */
	switch (m_RageSowrdInfo.iSowrdNum)
	{
	case 1:
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Sword1", TEXT("Sword1"), (CComponent**)&m_pModel)))
			return E_FAIL;
		break;
	case 2:
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Sword2", TEXT("Sword2"), (CComponent**)&m_pModel)))
			return E_FAIL;
		break;
	case 3:
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Sword3", TEXT("Sword3"), (CComponent**)&m_pModel)))
			return E_FAIL;
		break;
	case 4:
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Sword4", TEXT("Sword4"), (CComponent**)&m_pModel)))
			return E_FAIL;
		break;
	case 5:
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Sword5", TEXT("Sword5"), (CComponent**)&m_pModel)))
			return E_FAIL;
		break;
	case 6:
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Sword6", TEXT("Sword6"), (CComponent**)&m_pModel)))
			return E_FAIL;
		break;
	
	}
	m_pTransformCom->Set_Scale(_vector{ 1.f,1.f,1.f });
	//m_pTransformCom->Rotation(_vector{ 1.f,0.f,0.f }, 180.f);

	return S_OK;
}

void CPlayerRageSword::Tick(_float fTimeDelta)
{
	
}

void CPlayerRageSword::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_bStartDissolve)
	{
		m_fDissolveAcc += 0.3f * fTimeDelta;
		if (m_fDissolveAcc >= 0.67f)
		{
			m_bStartDissolve = false;
			m_fDissolveAcc = 0.f;
		}
	}
	else if (m_bEndDissolve)
	{
		m_fDissolveAcc += 0.3f * fTimeDelta;
		if (m_fDissolveAcc >= 0.67f)
		{
			m_bEndDissolve = false;
			m_fDissolveAcc = 0.f;
		}
	}

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);

}

HRESULT CPlayerRageSword::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
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
		else
		{
			if (FAILED(m_pShaderCom->Begin(MODEL_DEFAULT)))
				return E_FAIL;
		}
		
		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

void CPlayerRageSword::StartDissolve()
{
	m_bStartDissolve = true;
	m_bEndDissolve = false;
	m_fDissolveAcc = 0.f;
}

void CPlayerRageSword::EndDissolve()
{
	m_bEndDissolve = true;
	m_bStartDissolve = false;
	m_fDissolveAcc = 0.f;
}

void CPlayerRageSword::Set_On(_float4 vPos)
{
	m_bStartDissolve = true;
	m_bEndDissolve = false;
	m_fDissolveAcc = 0.f;
	vPos.w = 1.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&vPos));
}

void CPlayerRageSword::Set_Off()
{
	m_bEndDissolve = true;
	m_fDissolveAcc = 0.f;
}

CMesh * CPlayerRageSword::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerRageSword*		pInstance = new CPlayerRageSword(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerRageSword"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerRageSword::Clone(void * pArg)
{
	CPlayerRageSword*		pInstance = new CPlayerRageSword(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerRageSword"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerRageSword::Free()
{
	__super::Free();
	Safe_Release(m_pDissolveTexture);
	Safe_Release(m_pModel);

}
