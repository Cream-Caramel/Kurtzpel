#include "stdafx.h"
#include "..\Public\PlayerHead.h"
#include "GameInstance.h"

CPlayerHead::CPlayerHead(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerHead::CPlayerHead(const CPlayerHead& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerHead::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerHead::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerHead", TEXT("PlayerHead"), (CComponent**)&m_pModel)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ -0.1f,0.f,0.f,1.f });

	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 90.f, _float3{ 0.f,1.f,0.f }, 90.f, _float3{ 0.f,0.f,1.f },0.f);
	m_fOutLinePower = 3.f;
	m_fColiisionTime = 1.2f;
	m_bCollision = false;
	return S_OK;
}

void CPlayerHead::Tick(_float fTimeDelta)
{
	if (!m_bCollision)
	{
		m_fCollisionAcc += 1.f * fTimeDelta;
		if (m_fCollisionAcc >= m_fColiisionTime)
		{
			m_fCollisionAcc = 0.f;
			m_bCollision = true;
		}
	}
}

void CPlayerHead::LateTick(_float fTimeDelta)
{


}

HRESULT CPlayerHead::Render()
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

		if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrixInverse", &XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&WorldMatrix))), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrixInverse", &GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
			return E_FAIL;

		m_pShaderCom->Set_RawValue("g_fOutLinePower", &m_fOutLinePower, sizeof(_float));

		if (FAILED(m_pShaderCom->Begin(MODEL_OUTLINE)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;

		

		if (m_bBlur)
		{
			if (FAILED(m_pShaderCom->Begin(MODEL_NBLUR)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(i)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Begin(MODEL_NDEFAULT)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(i)))
				return E_FAIL;

			if (!m_bCollision)
			{
				m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

				if (FAILED(m_pShaderCom->Begin(MODEL_NHIT)))
					return E_FAIL;

				if (FAILED(m_pModel->Render(i)))
					return E_FAIL;

			}
		}

			

		
	}
	return S_OK;
}

HRESULT CPlayerHead::Render_ShadowDepth()
{
	if (m_pModel != nullptr)
	{
		_uint		iNumMeshes = m_pModel->Get_NumMeshes();
		for (_uint j = 0; j < iNumMeshes; ++j)
		{
			

			_matrix		LightViewMatrix;

			LightViewMatrix = XMMatrixTranspose(GI->Get_LightMatrix());

			_float4x4		WorldMatrix;

			XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()));

			if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Set_RawValue("g_LightViewMatrix", &LightViewMatrix, sizeof(_float4x4))))
				return E_FAIL;

			_matrix Fov60 = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (_float)1280.f / 720.f, 0.2f, 300.f);

			if (FAILED(m_pShaderCom->Set_RawValue("g_LightProjMatrix", &XMMatrixTranspose(Fov60), sizeof(_float4x4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(MODEL_SHADOW)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(j)))
				return E_FAIL;
		}
	}

	return S_OK;
}

CMesh * CPlayerHead::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerHead*		pInstance = new CPlayerHead(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerHead"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerHead::Clone(void * pArg)
{
	CPlayerHead*		pInstance = new CPlayerHead(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerHead"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerHead::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

	

}
