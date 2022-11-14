#pragma once
#include "Client_Defines.h"
#include "AnimMesh.h"
BEGIN(Client)

class CGolemRock3 final : public CAnimMesh
{

private:
	CGolemRock3(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CGolemRock3(const CGolemRock3& rhs);
	virtual ~CGolemRock3() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CAnimModel* m_pAnimModel;
	CTexture* m_pDissolveTexture = nullptr;
	_bool m_bDissolve = false;
	_float m_fDissolveAcc = 0.f;


public:
	static CAnimMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END