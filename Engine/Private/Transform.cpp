#include "..\Public\Transform.h"

CTransform::CTransform(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{

}

CTransform::CTransform(const CTransform & rhs)
	: CComponent(rhs)
	, m_WorldMatrix(rhs.m_WorldMatrix)
{

}

void CTransform::Set_State(STATE eState, _fvector vState)
{
	_matrix	WorldMatrix = XMLoadFloat4x4(&m_WorldMatrix);

	WorldMatrix.r[eState] = vState;

	XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
}

HRESULT CTransform::Initialize_Prototype()
{
	/* vector -> float : XMStore*/
	/* float -> vector : XMLoad */

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());	

	return S_OK;
}

HRESULT CTransform::Initialize(void * pArg)
{
	if (nullptr != pArg)
		memcpy(&m_TransformDesc, pArg, sizeof(TRANSFORMDESC));

	return S_OK;
}

void CTransform::RotationThree(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3)
{
	_float3		vScale = Get_Scale();
	_vector vRight = { 1.f,0.f,0.f };
	vRight = vRight * vScale.x;

	_vector vUp = { 0.f,1.f,0.f };
	vUp = vUp * vScale.y;

	_vector vLook = { 0.f,0.f,1.f };
	vLook = vLook * vScale.z;

	_vector Axis = XMLoadFloat3(&vAxis);
	_matrix		RotationMatrix = XMMatrixRotationAxis(Axis, XMConvertToRadians(fRadian));
	
	Set_State(CTransform::STATE_RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
	Set_State(CTransform::STATE_UP, XMVector3TransformNormal(vUp, RotationMatrix));
	Set_State(CTransform::STATE_LOOK, XMVector3TransformNormal(vLook, RotationMatrix));


	_vector		vRight2 = Get_State(CTransform::STATE_RIGHT);
	_vector		vUp2 = Get_State(CTransform::STATE_UP);
	_vector		vLook2 = Get_State(CTransform::STATE_LOOK);

	_vector Axis2 = XMLoadFloat3(&vAxis2);
	_matrix		RotationMatrix2 = XMMatrixRotationAxis(Axis2, XMConvertToRadians(fRadian2));

	Set_State(CTransform::STATE_RIGHT, XMVector3TransformNormal(vRight2, RotationMatrix2));
	Set_State(CTransform::STATE_UP, XMVector3TransformNormal(vUp2, RotationMatrix2));
	Set_State(CTransform::STATE_LOOK, XMVector3TransformNormal(vLook2, RotationMatrix2));

	_vector		vRight3 = Get_State(CTransform::STATE_RIGHT);
	_vector		vUp3 = Get_State(CTransform::STATE_UP);
	_vector		vLook3 = Get_State(CTransform::STATE_LOOK);

	_vector Axis3 = XMLoadFloat3(&vAxis3);
	_matrix		RotationMatrix3 = XMMatrixRotationAxis(Axis3, XMConvertToRadians(fRadian3));

	Set_State(CTransform::STATE_RIGHT, XMVector3TransformNormal(vRight3, RotationMatrix3));
	Set_State(CTransform::STATE_UP, XMVector3TransformNormal(vUp3, RotationMatrix3));
	Set_State(CTransform::STATE_LOOK, XMVector3TransformNormal(vLook3, RotationMatrix3));
	
}

void CTransform::Go_Straight(_float fTimeDelta)
{
	_vector		vPosition = Get_State(CTransform::STATE_POSITION);
	_vector		vLook = Get_State(CTransform::STATE_LOOK);

	vPosition += XMVector3Normalize(vLook) * m_TransformDesc.fSpeedPerSec * fTimeDelta;

	Set_State(CTransform::STATE_POSITION, vPosition);
}

void CTransform::Go_Backward(_float fTimeDelta)
{
	_vector		vPosition = Get_State(CTransform::STATE_POSITION);
	_vector		vLook = Get_State(CTransform::STATE_LOOK);

	vPosition -= XMVector3Normalize(vLook) * m_TransformDesc.fSpeedPerSec * fTimeDelta;

	Set_State(CTransform::STATE_POSITION, vPosition);
}

void CTransform::Go_Left(_float fTimeDelta)
{
	_vector		vPosition = Get_State(CTransform::STATE_POSITION);
	_vector		vRight = Get_State(CTransform::STATE_RIGHT);

	vPosition -= XMVector3Normalize(vRight) * m_TransformDesc.fSpeedPerSec * fTimeDelta;

	Set_State(CTransform::STATE_POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta)
{
	_vector		vPosition = Get_State(CTransform::STATE_POSITION);
	_vector		vRight = Get_State(CTransform::STATE_RIGHT);

	vPosition += XMVector3Normalize(vRight) * m_TransformDesc.fSpeedPerSec * fTimeDelta;

	Set_State(CTransform::STATE_POSITION, vPosition);
}

void CTransform::Go_Dir(_fvector vDir, _float fSpeed, _float fTimeDelta)
{
	_vector vPosition = Get_State(CTransform::STATE_POSITION);
	_vector vDirN = XMVector3Normalize(vDir);

	vPosition += vDir * fSpeed * fTimeDelta;

	Set_State(CTransform::STATE_POSITION, vPosition);
}

void CTransform::Set_Scale(_fvector vScaleInfo)
{
	Set_State(CTransform::STATE_RIGHT, 
		XMVector3Normalize(Get_State(CTransform::STATE_RIGHT)) * XMVectorGetX(vScaleInfo));
	Set_State(CTransform::STATE_UP, 
		XMVector3Normalize(Get_State(CTransform::STATE_UP)) * XMVectorGetY(vScaleInfo));
	Set_State(CTransform::STATE_LOOK, 
		XMVector3Normalize(Get_State(CTransform::STATE_LOOK)) * XMVectorGetZ(vScaleInfo));	
}

_float3 CTransform::Get_Scale()
{
	return _float3(
		XMVectorGetX(XMVector3Length(Get_State(CTransform::STATE_RIGHT))),
		XMVectorGetX(XMVector3Length(Get_State(CTransform::STATE_UP))),
		XMVectorGetX(XMVector3Length(Get_State(CTransform::STATE_LOOK))));
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_matrix		RotationMatrix = XMMatrixRotationAxis(vAxis, m_TransformDesc.fRotationPerSec * fTimeDelta);
	
	Set_State(CTransform::STATE_RIGHT, XMVector3TransformNormal(Get_State(CTransform::STATE_RIGHT), RotationMatrix));
	Set_State(CTransform::STATE_UP, XMVector3TransformNormal(Get_State(CTransform::STATE_UP), RotationMatrix));
	Set_State(CTransform::STATE_LOOK, XMVector3TransformNormal(Get_State(CTransform::STATE_LOOK), RotationMatrix));
}

void CTransform::Turn(_fvector StartLook, _fvector EndLook, _float _fRatio)
{
	_vector vLook, vRight;
	vLook = (XMVector3Normalize(StartLook) * (1.f - _fRatio)) + (XMVector3Normalize(EndLook)*_fRatio);

	vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
	vLook = XMVector3Normalize(XMVector3Cross(vRight, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	_float3      vScale = Get_Scale();

	Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);

}

void CTransform::Rotation(_fvector vAxis, _float fRadian)
{
	// Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(60.0f));

	_matrix		RotationMatrix = XMMatrixRotationAxis(vAxis, XMConvertToRadians(fRadian));
	_float3		Scale = Get_Scale();

	Set_State(CTransform::STATE_RIGHT, XMVector3TransformNormal(XMVectorSet(1.f, 0.f, 0.f, 0.f) * Scale.x, RotationMatrix));
	Set_State(CTransform::STATE_UP, XMVector3TransformNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f) * Scale.y, RotationMatrix));
	Set_State(CTransform::STATE_LOOK, XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f) * Scale.z, RotationMatrix));
}

void CTransform::LookAt(_fvector vAt)
{
	_vector		vLook = vAt - Get_State(CTransform::STATE_POSITION);

	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);

	_vector		vUp = XMVector3Cross(vLook, vRight);

	_float3		vScale = Get_Scale();

	Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScale.y);
	Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void CTransform::LookAt_ForLandObject(_fvector vAt)
{
	_vector		vLook = vAt - Get_State(CTransform::STATE_POSITION);

	_float3		vScale = Get_Scale();

	_vector		vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook)) * vScale.x;	

	vLook = XMVector3Normalize(XMVector3Cross(vRight, Get_State(CTransform::STATE_UP))) * vScale.z;

	Set_State(CTransform::STATE_RIGHT, vRight);
	Set_State(CTransform::STATE_LOOK, vLook);

}

_bool  CTransform::Move(_fvector vTargetPos, _float fSpeed, _float fTimeDelta, _float fLimitDistance)
{
	_vector		vPosition = Get_State(CTransform::STATE_POSITION);
	_vector		vDirection = vTargetPos - vPosition;

	_float		fDistance = XMVectorGetX(XMVector3Length(vDirection));

	if (fDistance > fLimitDistance)
		vPosition += XMVector3Normalize(vDirection) * fSpeed * fTimeDelta;
	else
		return true;

	Set_State(CTransform::STATE_POSITION, vPosition);
	return false;
}

CTransform * CTransform::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTransform*			pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTransform"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent * CTransform::Clone(void * pArg)
{
	CTransform*			pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTransform"));
		Safe_Release(pInstance);
	}
	return pInstance;
}


void CTransform::Free()
{
	__super::Free();
}
