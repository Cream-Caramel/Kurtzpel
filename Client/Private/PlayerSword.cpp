#include "stdafx.h"
#include "..\Public\PlayerSword.h"
#include "GameInstance.h"
#include "OBB.h"

CPlayerSword::CPlayerSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerSword::CPlayerSword(const CPlayerSword& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerSword::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerSword::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerSword", TEXT("PlayerSword"), (CComponent**)&m_pModel)))
		return E_FAIL;


	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ 0.05f,0.f,0.f,1.f });
	m_pTransformCom->Set_Scale(_vector{ 0.8f,0.8f,0.8f });
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 90.f, _float3{ 0.f,1.f,0.f }, 180.f, _float3{ 0.f,0.f,1.f }, 0.f);

	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(0.3f, 2.2f, 0.3f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(45.f), 0.f);
	ColliderDesc.sTag = "Player_Sword";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Sword"), (CComponent**)&m_pOBB, &ColliderDesc)))
		return E_FAIL;

	m_pOBBs.push_back(m_pOBB);
	Safe_AddRef(m_pOBB);

	return S_OK;
}

void CPlayerSword::Tick(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_0))
		m_bCollider = !m_bCollider;
}

void CPlayerSword::LateTick(_float fTimeDelta)
{
	_float4x4		WorldMatrix;

	//XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose());
	m_pOBB->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
}

HRESULT CPlayerSword::Render()
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
		/*if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
		return E_FAIL;*/

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	if(m_bCollider)
		m_pOBB->Render();
	return S_OK;
}

CPlayerSword * CPlayerSword::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerSword*		pInstance = new CPlayerSword(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerSword"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerSword::Clone(void * pArg)
{
	CPlayerSword*		pInstance = new CPlayerSword(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerSword"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerSword::Free()
{
	__super::Free();
	Safe_Release(m_pModel);
	Safe_Release(m_pOBB);


}
