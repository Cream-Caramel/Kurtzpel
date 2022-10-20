#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	enum STATE { STATE_RIGHT, STATE_UP, STATE_LOOK, STATE_POSITION, STATE_END };

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

	void Go_Dir(_fvector vDir, _float fSpeed, class CNavigation* pNavigation, _float fTimeDelta);

	void Set_Scale(_fvector vScaleInfo);
	_float3 Get_Scale();

	void Turn(_fvector vAxis, _float fTimeDelta);
	void Turn(_fvector StartLook, _fvector EndLook, _float _fRatio);
	void Rotation(_fvector vAxis, _float fRadian);

	void LookAt(_fvector vAt);
	void LookAt_ForLandObject(_fvector vAt);
	_bool Move(_fvector vTargetPos, _float fSpeed, _float fTimeDelta, _float fLimitDistance = 0.1f);

	_float Get_Speed() { return m_TransformDesc.fSpeedPerSec; }
	void Set_Speed(_float fSpeed) { m_TransformDesc.fSpeedPerSec = fSpeed; }
private:
	_float4x4				m_WorldMatrix;
	TRANSFORMDESC			m_TransformDesc;

public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END