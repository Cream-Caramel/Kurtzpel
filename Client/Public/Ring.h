#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CRing final : public CMesh
{
public:
	enum RINGCOLOR {RING_ORANGE, RING_BLUE, RING_GREEN, RING_RED, RING_END};
public:
	typedef struct tagRingInfo
	{
		_float3 vSize;
		_float3 vSpeed;
		_float fLifeTime;
		_float4 vWorldPos;
		RINGCOLOR eColor;
	}RINGINFO;

private:
	CRing(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CRing(const CRing& rhs);
	virtual ~CRing() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fLifeTimeAcc = 0.f;
	_bool m_bEnd = false;
	CModel* m_pModel;
	RINGINFO m_RingInfo;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END