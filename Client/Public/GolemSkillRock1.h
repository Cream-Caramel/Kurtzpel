#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Engine)
class COBB;
END

BEGIN(Client)

class CGolemSkillRock1 final : public CMesh
{
public:
	typedef struct tagGolemSKillRock1Info
	{
		_float3 vScale{ 1.f,1.f,1.f };
		_float4 vPos;
	}GOLEMROCK1INFO;
private:
	CGolemSkillRock1(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CGolemSkillRock1(const CGolemSkillRock1& rhs);
	virtual ~CGolemSkillRock1() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModel;
	_float m_fCloseSpeed;
	_float m_fSpeed = 0.f;
	_float3 m_vTargetPos;
	_float3 m_vTargetLook;
	COBB* m_pOBB = nullptr;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END