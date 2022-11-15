#include "stdafx.h"
#include "..\Public\PlayerGage2.h"
#include "GameInstance.h"

CPlayerGage2::CPlayerGage2(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerGage2::CPlayerGage2(const CPlayerGage2& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerGage2::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerGage2::Initialize(void * pArg)
{
	m_PlayerGage2Info = (*(PLAYERGAGE2INFO*)pArg);

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

	m_bDead = false;
	
	/* For.Com_Model */

	if (m_PlayerGage2Info.bGage2_1)
	{
		m_pTransformCom->Set_Scale(_vector{ 10.f,2.5f,10.f });
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerGage2_1", TEXT("PlayerGage2_1"), (CComponent**)&m_pModel)))
			return E_FAIL;
	}
	else
	{
		m_pTransformCom->Set_Scale(_vector{ 8.f,2.f,8.f });
		if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerGage2_2", TEXT("PlayerGage2_2"), (CComponent**)&m_pModel)))
			return E_FAIL;
	}
	m_fShaderUVAcc = 0.f;
	m_fShaderUVIndexX = 0;
	m_fShaderUVIndexY = 0;

	m_fMaxUVIndexX = m_PlayerGage2Info.fMaxUVIndexX;
	m_fMaxUVIndexY = m_PlayerGage2Info.fMaxUVIndexY;
	m_fUVSpeed = m_PlayerGage2Info.fUVSpeed;
	m_eTurnDir = m_PlayerGage2Info.eTurnDir;
	
	m_pTransformCom->Rotation(_vector{ 0.f,1.f,0.f }, m_PlayerGage2Info.fRotation);
	
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_PlayerGage2Info.vWorldPos));
	m_pTransformCom->Set_TurnSpeed(8.f);
	return S_OK;
}

void CPlayerGage2::Tick(_float fTimeDelta)
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
			}
		}
	}

	if (m_eTurnDir == TURN_FRONT)
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_UP), fTimeDelta);
	else
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_UP) * -1, fTimeDelta);

	if (m_bEnd)
	{
		m_pTransformCom->Set_Scale(XMLoadFloat3(&m_pTransformCom->Get_Scale()) - _vector{ 0.2f,0.01f,0.2f });
		if (m_pTransformCom->Get_Scale().x <= 0.5f)
			Set_Dead();
	}

}

void CPlayerGage2::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	

	if (m_PlayerGage2Info.bGage2_1)
	{
		if (!m_bEnd)
		{
			if (PM->Get_PlayerGage2_1())
				m_bEnd = true;
		}
	}

	else
	{
		if (!m_bEnd)
		{
			if (PM->Get_PlayerGage2_2())
				m_bEnd = true;
		}
	}

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_FRONTALPHA, this);
	

}

HRESULT CPlayerGage2::Render()
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

		m_bNormalTex = false;
		m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
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

CMesh * CPlayerGage2::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerGage2*		pInstance = new CPlayerGage2(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerGage2"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerGage2::Clone(void * pArg)
{
	CPlayerGage2*		pInstance = new CPlayerGage2(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerGage2"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerGage2::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

}
