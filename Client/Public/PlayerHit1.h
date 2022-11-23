#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CPlayerHit1 final : public CMesh
{
public:
	typedef struct tagPlayerHit1Info
	{
		_matrix vWorldMatrix;
		_float fRotation;
		_float3 vScale;
	}PLAYERHIT1INFO;

private:
	CPlayerHit1(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerHit1(const CPlayerHit1& rhs);
	virtual ~CPlayerHit1() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool m_bEnd = false;
	CModel* m_pModel = nullptr;
	PLAYERHIT1INFO m_PlayerHit1Info;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END