#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	enum STATE { STATE_RIGHT, STATE_UP, STATE_LOOK, STATE_POSITION, STATE_END };
	enum AXIS {AXIS_X, AXIS_Y, AXIS_Z, AXIS_END};
	typedef struct tagTransformDesc
	{
		_float			fSpeedPerSec;
		_float			fRotationPerSec;
	}TRANSFORMDESC;


private:
	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransform(const CTransform& rhs);
	virtual ~CTransform() = default;

public:
	_vector Get_State(STATE eState) {
		return XMLoadFloat4((_float4*)&m_WorldMatrix.m[eState][0]);
	}

	_matrix Get_WorldMatrix() const {
		return XMLoadFloat4x4(&m_WorldMatrix);
	}

	_float4x4 Get_WorldFloat4x4() const {
		return m_WorldMatrix;
	}

	_float4x4 Get_WorldFloat4x4_TP() const {
		_float4x4	WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(Get_WorldMatrix()));
		return WorldMatrix;
	}

	_matrix Get_WorldMatrixInverse() const {
		return XMMatrixInverse(nullptr, Get_WorldMatrix());
	}

	void Set_State(STATE eState, _fvector vState);

	void Set_WorldMatrix(_matrix matrix);

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

public:
	void RotationThree(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3);
	void Go_Straight(_float fTimeDelta);
	void Go_Backward(_float fTimeDelta);
	void Go_Left(_float fTimeDelta);
	void Go_Right(_float fTimeDelta);
	void Move_Up(_float fTimeDelta, _float fSpeed);
	void Move_Down(_float fTimeDelta, _float fSpeed);
	void Move_Right(_float fTimeDelta, _float fSpeed);
	void Move_Left(_float fTimeDelta, _float fSpeed);
	void Move_RU(_float fTimeDelta, _float fSpeed);
	void Move_LU(_float fTimeDelta, _float fSpeed);
	void Move_RD(_float fTimeDelta, _float fSpeed);
	void Move_LD(_float fTimeDelta, _float fSpeed);

	void Go_Dir(_fvector vDir, _float fSpeed, _float fTimeDelta);
	void Go_Dir(_fvector vDir, _float fSpeed, class CNavigation* pNavigation, _float fTimeDelta);
	_bool Go_NoSlide(_fvector vDir, _float fSpeed, class CNavigation* pNavigation, _float fTimeDelta);

	void Set_Scale(_fvector vScaleInfo);
	_float3 Get_Scale();
	void Set_ScaleAxis(AXIS eAxis, _float fScale);
	_float Get_ScaleAxis(AXIS eAxis);

	void Turn(_fvector vAxis, _float fTimeDelta);
	void Turn(_fvector StartLook, _fvector EndLook, _float _fRatio);
	void TurnY(_fvector StartLook, _fvector EndLook, _float _fRatio);
	void Rotation(_fvector vAxis, _float fRadian);

	void LookAt(_fvector vAt);
	void LookAt_ForLandObject(_fvector vAt);
	_bool CameraMove(_fvector vTargetPos, _float fSpeed, _float fTimeDelta, _float fLimitDistance = 0.1f);

	_float Get_Speed() { return m_TransformDesc.fSpeedPerSec; }
	void Set_Speed(_float fSpeed) { m_TransformDesc.fSpeedPerSec = fSpeed; }

	_float Get_TurnSpeed() { return m_TransformDesc.fRotationPerSec; }
	void Set_TurnSpeed(_float fSpeed) { m_TransformDesc.fRotationPerSec = fSpeed; }

	void Jump(_float fTimeDelta);

	_bool Get_Jump() { return m_bJump; }
	_float Get_JumpPower() { return m_fJumpPower; }
	_float Get_Gravity() { return m_fGravity; }
	void Set_Jump(_bool bJump) { m_bJump = bJump; }
	void Set_JumpPower(_float fJumpPower) { m_fJumpPower = fJumpPower; }
	void Set_Gravity(_float fGravity) { m_fGravity = fGravity; }
	_bool Get_JumpEnd(_fvector vPos, CNavigation* pNavigation);
	void Set_JumpEndPos(CNavigation* pNavigation);
	void Set_GravityPower(_float fGravityPower) { m_fGravityPower = fGravityPower; }
	_float Get_GravityPower() { return m_fGravityPower; }
private:
	_float4x4				m_WorldMatrix;
	TRANSFORMDESC			m_TransformDesc;
	_float m_fGravity = 0.f; //중력 
	_bool m_bJump = false; //점프중인지
	_float m_fJumpPower = 5.f; //뛰어오르는 힘
	_float m_fGravityPower = 0.f;

public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END