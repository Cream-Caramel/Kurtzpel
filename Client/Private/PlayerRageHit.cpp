#include "stdafx.h"
#include "..\Public\PlayerRageHit.h"
#include "GameInstance.h"
#include "Pointer_Manager.h"

CPlayerRageHit::CPlayerRageHit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerRageHit::CPlayerRageHit(const CPlayerRageHit& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerRageHit::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerRageHit::Initialize(void * pArg)
{
	m_PlayerRageHitInfo = (*(PLAYERRAGEHITINFO*)pArg);

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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_EffectModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerHit1", TEXT("PlayerHit1"), (CComponent**)&m_pModel)))
		return E_FAIL;

	m_bDead = false;

	m_fShaderUVAcc = 0.f;
	m_fShaderUVIndexX = 0;
	m_fShaderUVIndexY = 0;

	m_fMaxUVIndexX = 2;
	m_fMaxUVIndexY = 2;
	m_fUVSpeed = 0.04f;
	
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_PlayerRageHitInfo.vWorldPos));
	_vector CamLook = XMLoadFloat4(&GI->Get_CamPosition()) - XMLoadFloat4(&m_PlayerRageHitInfo.vWorldPos);
	//m_pTransformCom->LookAt(CamLook);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&m_PlayerRageHitInfo.vScale));
	m_pTransformCom->TurnAngle(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(m_PlayerRageHitInfo.fRotation));
	//m_pTransformCom->TurnAngle(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(m_PlayerRageHitInfo.fRotation));
	return S_OK;
}

void CPlayerRageHit::Tick(_float fTimeDelta)
{
	if (!m_bEnd)
	{
		m_fShaderUVAcc += 1.f * fTimeDelta;
		if (m_fShaderUVAcc >= m_fUVSpeed)
		{
			m_fShaderUVAcc = 0.f;
			m_fShaderUVIndexX += 1.f;
			if (m_fShaderUVIndexX >= m_fMaxUVIndexX)
			{
				m_fShaderUVIndexX = 0.f;
				m_fShaderUVIndexY += 1.f;
				if (m_fShaderUVIndexY >= m_fMaxUVIndexY)
				{
					m_fShaderUVIndexY = 0.f;
					m_bEnd = true;
				}
			}
		}
	}
	

	else
	{
		m_pTransformCom->Set_Scale(XMLoadFloat3(&m_pTransformCom->Get_Scale()) - _vector{ 0.8f,0.f,0.8f });
		if (m_pTransformCom->Get_Scale().x <= 0.8f)
			Set_Dead();
	}

}

void CPlayerRageHit::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_FRONTALPHA, this);
	

}

HRESULT CPlayerRageHit::Render()
{
	if (nullptr == m_pModel ||
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

	_uint		iNumMeshes = m_pModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		m_pShaderCom->Set_RawValue("g_fMaxUVIndexX", &m_fMaxUVIndexX, sizeof(_float));
		m_pShaderCom->Set_RawValue("g_fMaxUVIndexY", &m_fMaxUVIndexY, sizeof(_float));
		m_pShaderCom->Set_RawValue("g_fUVIndexX", &m_fShaderUVIndexX, sizeof(_float));
		m_pShaderCom->Set_RawValue("g_fUVIndexY", &m_fShaderUVIndexY, sizeof(_float));

		if (FAILED(m_pShaderCom->Begin(EFFECT_NDEFAULT)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

CMesh * CPlayerRageHit::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerRageHit*		pInstance = new CPlayerRageHit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerRageHit"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerRageHit::Clone(void * pArg)
{
	CPlayerRageHit*		pInstance = new CPlayerRageHit(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerRageHit"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerRageHit::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

}
