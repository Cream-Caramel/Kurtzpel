#include "stdafx.h"
#include "..\Public\Rock.h"
#include "GameInstance.h"

CRock::CRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	m_pAnimModel = nullptr;
}

CRock::CRock(const CRock& rhs)
	: CAnimMesh(rhs)
{
}

HRESULT CRock::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CRock::Initialize(void * pArg)
{
	EFFECTINFO* EffectInfo = ((EFFECTINFO*)pArg);

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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	m_bDead = false;
	
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Rock", TEXT("Rock"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Noise"), TEXT("Com_Texture"), (CComponent**)&m_pDissolveTexture)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(EffectInfo->WorldMatrix);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&EffectInfo->vScale));

	return S_OK;
}

void CRock::Tick(_float fTimeDelta)
{
	if (!m_bDissolve)
		m_pAnimModel->Play_Animation(fTimeDelta, m_pAnimModel);
	else
		m_fDissolveAcc += 0.3f * fTimeDelta;
	
}

void CRock::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_pAnimModel->GetAniEnd())
		m_bDissolve = true;

	if (m_fDissolveAcc >= 2.f)
		Set_Dead();

	if (!m_bDead)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	

}

HRESULT CRock::Render()
{
	if (nullptr == m_pAnimModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);

	_uint		iNumMeshes = m_pAnimModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		
		if (!m_bDissolve)
		{
			if (FAILED(m_pAnimModel->Render(m_pShaderCom, i, ANIM_DEFAULT)))
				return E_FAIL;
		}
		else
		{
			m_pDissolveTexture->Set_SRV(m_pShaderCom, "g_DissolveTexture", 0);
			m_pShaderCom->Set_RawValue("g_fDissolveAcc", &m_fDissolveAcc, sizeof(float));
			if (FAILED(m_pAnimModel->Render(m_pShaderCom, i, ANIM_DISSOLVE)))
				return E_FAIL;
		}
		
	}
	return S_OK;
}

CAnimMesh * CRock::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CRock*		pInstance = new CRock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CRock::Clone(void * pArg)
{
	CRock*		pInstance = new CRock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRock::Free()
{
	__super::Free();
	Safe_Release(m_pDissolveTexture);
	Safe_Release(m_pAnimModel);

}
