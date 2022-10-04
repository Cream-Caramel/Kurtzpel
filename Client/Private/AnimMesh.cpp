#include "stdafx.h"
#include "..\Public\AnimMesh.h"
#include "AnimModel.h"
#include "GameInstance.h"



CAnimMesh::CAnimMesh(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CAnimMesh::CAnimMesh(const CAnimMesh & rhs)
	: CGameObject(rhs)
{
}

HRESULT CAnimMesh::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimMesh::Initialize(void * pArg)
{
	m_MeshInfo = ((MESHINFO*)pArg);

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_bDead = false;
	m_MeshInfo->fPos.w = 1.f;
	Set_Pos(m_MeshInfo->fPos);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&m_MeshInfo->fScale));
	Rotation(_float3{ 1.f,0.f,0.f }, m_MeshInfo->fAngle.x, _float3{ 0.f,1.f,0.f }, m_MeshInfo->fScale.y, _float3{ 0.f,0.f,1.f }, m_MeshInfo->fAngle.z);

	return S_OK;
}

void CAnimMesh::Tick(_float fTimeDelta)
{
}

void CAnimMesh::LateTick(_float fTimeDelta)
{
}

HRESULT CAnimMesh::Render()
{
	return S_OK;
}

void CAnimMesh::Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3)
{
	m_pTransformCom->RotationThree(vAxis, fRadian, vAxis2, fRadian2, vAxis3, fRadian3);
}

_float3 CAnimMesh::Get_Pos()
{
	_float3 Pos;
	XMStoreFloat3(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	return Pos;
}

void CAnimMesh::Set_Pos(_float4 Pos)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&Pos));
}


HRESULT CAnimMesh::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC Transform;
	Transform.fRotationPerSec = 90.f;
	Transform.fSpeedPerSec = 5.f;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &Transform)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

CAnimMesh * CAnimMesh::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CAnimMesh*		pInstance = new CAnimMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimMesh"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CAnimMesh::Clone(void * pArg)
{
	CAnimMesh*		pInstance = new CAnimMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CAnimMesh"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
}
