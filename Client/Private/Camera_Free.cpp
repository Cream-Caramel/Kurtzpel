#include "stdafx.h"
#include "..\Public\Camera_Free.h"
#include "GameInstance.h"


CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera(pDevice, pContext)
{
}

CCamera_Free::CCamera_Free(const CCamera_Free & rhs, CTransform::TRANSFORMDESC * pArg)
	: CCamera(rhs, pArg)
{

}

HRESULT CCamera_Free::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCamera_Free::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Tick(_float fTimeDelta)
{
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	if (pGameInstance->Key_Pressing(DIK_W))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}

	if (pGameInstance->Key_Pressing(DIK_S))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (pGameInstance->Key_Pressing(DIK_A))
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}

	if (pGameInstance->Key_Pressing(DIK_D))
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}

	_long	MouseMove = 0;

	if (MouseMove = pGameInstance->Get_DIMMoveState(DIMM_X))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * fTimeDelta * 0.05f);
	}

	if (MouseMove = pGameInstance->Get_DIMMoveState(DIMM_Y))
	{
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), MouseMove * fTimeDelta * 0.05f);
	}
	
	Safe_Release(pGameInstance);

	__super::Tick(fTimeDelta);



}

void CCamera_Free::LateTick(_float fTimeDelta)
{
	
}

HRESULT CCamera_Free::Render()
{
	return S_OK;
}

_float3 CCamera_Free::Get_Pos()
{
	_float3 vTempPos;
	XMStoreFloat3(&vTempPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	return vTempPos;
}
void CCamera_Free::Set_Pos(_float3 vPos)
{
	_vector temp = XMLoadFloat4(&_float4{ vPos.x,vPos.y,vPos.z,1.f });
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, temp);
}

_bool CCamera_Free::CameraMove(_fvector vTargetPos, _float fSpeed, _float fTimeDelta, _float fLimitDistance)
{
	return m_pTransformCom->CameraMove(vTargetPos,fSpeed,fTimeDelta,fLimitDistance);
}

void CCamera_Free::LookAt(_float3 TargetPos)
{
	_vector temp = XMLoadFloat4(&_float4{ TargetPos.x,TargetPos.y,TargetPos.z,1.f });
	m_pTransformCom->LookAt(temp);
}

CCamera_Free * CCamera_Free::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Free*		pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_Free::Clone(void* pArg)
{
	CCamera::CAMERADESC*		pCameraDesc = (CCamera::CAMERADESC*)pArg;

	CCamera_Free*		pInstance = new CCamera_Free(*this, &pCameraDesc->TransformDesc);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	__super::Free();


}
