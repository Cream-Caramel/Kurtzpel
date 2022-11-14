#include "stdafx.h"
#include "..\Public\GolemRock3.h"
#include "GameInstance.h"

CGolemRock3::CGolemRock3(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	m_pAnimModel = nullptr;
}

CGolemRock3::CGolemRock3(const CGolemRock3& rhs)
	: CAnimMesh(rhs)
{
}

HRESULT CGolemRock3::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CGolemRock3::Initialize(void * pArg)
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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"GolemRock3", TEXT("GolemRock3"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Noise"), TEXT("Com_Texture"), (CComponent**)&m_pDissolveTexture)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(EffectInfo->WorldMatrix);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&EffectInfo->vScale));

	return S_OK;
}

void CGolemRock3::Tick(_float fTimeDelta)
{
	if (!m_bDissolve)
		m_pAnimModel->Play_Animation(fTimeDelta, m_pAnimModel);
	else
		m_fDissolveAcc += 0.3f * fTimeDelta;
	
}

void CGolemRock3::LateTick(_float fTimeDelta)
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

HRESULT CGolemRock3::Render()
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

		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(i), TEX_NORMALS, "g_NormalTexture")))
		{
			m_bNormalTex = false;
			if (!m_bDissolve)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, i)))
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
	}
	return S_OK;
}

CAnimMesh * CGolemRock3::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CGolemRock3*		pInstance = new CGolemRock3(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CGolemRock3"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CGolemRock3::Clone(void * pArg)
{
	CGolemRock3*		pInstance = new CGolemRock3(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CGolemRock3"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGolemRock3::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel);

}
