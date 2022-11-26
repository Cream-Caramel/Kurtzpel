#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CPlayerRageHit final : public CMesh
{
public:
	typedef struct tagPlayerRageHitInfo
	{
		_float4 vWorldPos;
		_float fRotation;
		_float3 vScale;
	}PLAYERRAGEHITINFO;

private:
	CPlayerRageHit(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerRageHit(const CPlayerRageHit& rhs);
	virtual ~CPlayerRageHit() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool m_bEnd = false;
	CModel* m_pModel = nullptr;
	PLAYERRAGEHITINFO m_PlayerRageHitInfo;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END