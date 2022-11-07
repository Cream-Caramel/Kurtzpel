#pragma once
#include "Client_Defines.h"
#include "AnimMesh.h"
BEGIN(Client)

class CRock final : public CAnimMesh
{

private:
	CRock(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CRock(const CRock& rhs);
	virtual ~CRock() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CAnimModel* m_pAnimModel;


public:
	static CAnimMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END