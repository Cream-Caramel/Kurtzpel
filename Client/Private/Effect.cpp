#include "stdafx.h"
#include "..\Public\Effect.h"
#include "GameInstance.h"

CEffect::CEffect(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CEffect::CEffect(const CEffect & rhs)
	: CGameObject(rhs)
{
}

HRESULT CEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffect::Initialize(void * pArg)
{
	m_MeshInfo = ((MESHINFO*)pArg);

	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bDead = false;
	m_MeshInfo->fPos.w = 1.f;
	Set_Pos(m_MeshInfo->fPos);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&m_MeshInfo->fScale));
	Rotation(_float3{ 1.f,0.f,0.f }, m_MeshInfo->fAngle.x, _float3{ 0.f,1.f,0.f }, m_MeshInfo->fAngle.y, _float3{ 0.f,0.f,1.f }, m_MeshInfo->fAngle.z);

	return S_OK;
}

void CEffect::Tick(_float fTimeDelta)
{
}

void CEffect::LateTick(_float fTimeDelta)
{

}

HRESULT CEffect::Render()
{
	return S_OK;
}

HRESULT CEffect::SetUp_State(_fmatrix StateMatrix)
{
	m_pParentTransformCom->Set_State(CTransform::STATE_RIGHT, StateMatrix.r[0]);
	m_pParentTransformCom->Set_State(CTransform::STATE_UP, StateMatrix.r[1]);
	m_pParentTransformCom->Set_State(CTransform::STATE_LOOK, StateMatrix.r[2]);
	m_pParentTransformCom->Set_State(CTransform::STATE_POSITION, StateMatrix.r[3]);

	m_pParentTransformCom->Set_Scale(XMVectorSet(1.f, 1.f, 1.f, 1.f));

	return S_OK;
}

void CEffect::Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3)
{
	m_pTransformCom->RotationThree(vAxis, fRadian, vAxis2, fRadian2, vAxis3, fRadian3);
}

_float3 CEffect::Get_Pos()
{
	_float3 Pos;
	XMStoreFloat3(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	return Pos;
}

void CEffect::Set_Pos(_float4 Pos)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&Pos));
}

HRESULT CEffect::Ready_Components()
{

	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(TransformDesc));

	TransformDesc.fSpeedPerSec = 25.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(90.0f);

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

	return S_OK;
}

CEffect * CEffect::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CEffect*		pInstance = new CEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CEffect"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CEffect::Clone(void * pArg)
{
	CEffect*		pInstance = new CEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CEffect"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEffect::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pParentTransformCom);
}
