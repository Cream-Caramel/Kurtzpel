#include "stdafx.h"
#include "..\Public\Camera_Player.h"
#include "GameInstance.h"
#include "Player.h"
#include "Pointer_Manager.h"


CCamera_Player::CCamera_Player(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera(pDevice, pContext)
{
}

CCamera_Player::CCamera_Player(const CCamera_Player & rhs, CTransform::TRANSFORMDESC * pArg)
	: CCamera(rhs, pArg)
{

}

HRESULT CCamera_Player::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCamera_Player::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pPlayer = PM->Get_PlayerPointer();
	Safe_AddRef(m_pPlayer);

	PM->Add_CameraPlayer(this);

	m_vDistance = { 0.f,1.f,-6.f };

	m_pTransformCom->LookAt(m_pPlayer->Get_PlayerPos() + _vector{ 0.f,2.f,0.f,0.f });

	return S_OK;
}

void CCamera_Player::Tick(_float fTimeDelta)
{

	_long	MouseMove = 0;


	if (MouseMove = GI->Get_DIMMoveState(DIMM_X))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * fTimeDelta * 0.05f);
	}

	if (MouseMove = GI->Get_DIMMoveState(DIMM_Y))
	{
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), MouseMove * fTimeDelta * 0.05f);
	}

}

void CCamera_Player::LateTick(_float fTimeDelta)
{

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pPlayer->Get_PlayerPos());

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVector3TransformCoord(XMLoadFloat3(&m_vDistance), m_pTransformCom->Get_WorldMatrix()));

	if (XMVectorGetY(m_pTransformCom->Get_State(CTransform::STATE_POSITION)) <= 0.f)
	{
		_float4 Pos;
		XMStoreFloat4(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ Pos.x,0.1f,Pos.z,1.f });
	}
		
	__super::Tick(fTimeDelta);

}

HRESULT CCamera_Player::Render()
{
	return S_OK;
}

_float3 CCamera_Player::Get_Pos()
{
	_float3 vTempPos;
	XMStoreFloat3(&vTempPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	return vTempPos;
}
void CCamera_Player::Set_Pos(_float3 vPos)
{
	_vector temp = XMLoadFloat4(&_float4{ vPos.x,vPos.y,vPos.z,1.f });
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, temp);
}

_bool CCamera_Player::Move(_fvector vTargetPos, _float fSpeed, _float fTimeDelta, _float fLimitDistance)
{
	return m_pTransformCom->Move(vTargetPos, fSpeed, fTimeDelta, fLimitDistance);
}

void CCamera_Player::LookAt(_float3 TargetPos)
{
	_vector temp = XMLoadFloat4(&_float4{ TargetPos.x,TargetPos.y,TargetPos.z,1.f });
	m_pTransformCom->LookAt(temp);
}

CCamera_Player * CCamera_Player::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Player*		pInstance = new CCamera_Player(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Player"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_Player::Clone(void* pArg)
{
	CCamera::CAMERADESC*		pCameraDesc = (CCamera::CAMERADESC*)pArg;

	CCamera_Player*		pInstance = new CCamera_Player(*this, &pCameraDesc->TransformDesc);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Player::Free()
{
	__super::Free();
	Safe_Release(m_pPlayer);

}
