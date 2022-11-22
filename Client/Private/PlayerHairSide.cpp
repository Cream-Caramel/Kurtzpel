#include "stdafx.h"
#include "..\Public\PlayerHairSide.h"
#include "GameInstance.h"

CPlayerHairSide::CPlayerHairSide(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerHairSide::CPlayerHairSide(const CPlayerHairSide& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerHairSide::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerHairSide::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerHairSide", TEXT("PlayerHairSide"), (CComponent**)&m_pModel)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ -1.6f,0.f,0.f,1.f });

	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 90.f, _float3{ 0.f,1.f,0.f }, 90.f, _float3{ 0.f,0.f,1.f }, 0.f);

	m_fOutLinePower = 3.f;
	m_fColiisionTime = 1.2f;
	m_bCollision = false;
	return S_OK;
}

void CPlayerHairSide::Tick(_float fTimeDelta)
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

void CPlayerHairSide::LateTick(_float fTimeDelta)
{


}

HRESULT CPlayerHairSide::Render()
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
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		_matrix SumMatrix = m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix();
		_float4x4		OutLineWorldMatrix;

		XMStoreFloat4x4(&OutLineWorldMatrix, XMMatrixInverse(nullptr, SumMatrix));

		if (!m_bCollision)
		{
			if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrixInverse", &OutLineWorldMatrix, sizeof(_float4x4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrixInverse", &GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
				return E_FAIL;

			_uint		iNumViewport = 1;

			D3D11_VIEWPORT		ViewportDesc;

			m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

			m_pShaderCom->Set_RawValue("g_fWinSizeX", &ViewportDesc.Width, sizeof(_float));
			m_pShaderCom->Set_RawValue("g_fWinSizeY", &ViewportDesc.Height, sizeof(_float));
			m_pShaderCom->Set_RawValue("g_fOutLinePower", &m_fOutLinePower, sizeof(_float));

			if (FAILED(m_pShaderCom->Begin(MODEL_HIT)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(i)))
				return E_FAIL;

		}

		if (FAILED(m_pShaderCom->Begin(MODEL_DEFAULT)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

CMesh * CPlayerHairSide::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerHairSide*		pInstance = new CPlayerHairSide(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerHairSide"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerHairSide::Clone(void * pArg)
{
	CPlayerHairSide*		pInstance = new CPlayerHairSide(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerHairSide"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerHairSide::Free()
{
	__super::Free();
	Safe_Release(m_pModel);



}
