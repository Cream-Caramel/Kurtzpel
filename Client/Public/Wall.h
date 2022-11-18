#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CWall final : public CMesh
{
public:
	enum WALLCOLOR { WALL_ORANGE, WALL_BLUE, WALL_GREEN, WALL_RED, WALL_END };

public:
	typedef struct tagWallInfo
	{
		_float fUVSpeed = 0.f;
		_float fMaxUVIndexX = 0.f;
		_float fMaxUVIndexY = 0.f;
		_float3 vSize;
		_float4 vWorldPos;
		_float3 vSpeed;
		_float fEndSpeed;
		_float fLifeTime;
		WALLCOLOR eColor;
	}WALLINFO;

private:
	CWall(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CWall(const CWall& rhs);
	virtual ~CWall() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModel;
	WALLINFO m_WallInfo;
	_float m_fLifeTimeAcc = 0.f;
public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END