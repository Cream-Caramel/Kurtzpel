#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CPlayerGage2 final : public CMesh
{
public:
	typedef struct tagPlayerGage2Info
	{
		_float fUVSpeed = 0.f;
		_float fMaxUVIndexX = 0.f;
		_float fMaxUVIndexY = 0.f;
		_bool bGage2_1;
		_float4 vWorldPos;
		TURNDIR eTurnDir;
		_float fRotation;
	}PLAYERGAGE2INFO;

private:
	CPlayerGage2(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerGage2(const CPlayerGage2& rhs);
	virtual ~CPlayerGage2() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool m_bEnd = false;
	CModel* m_pModel;
	PLAYERGAGE2INFO m_PlayerGage2Info;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END