#pragma once
#include "Client_Defines.h"
#include "AnimMesh.h"
BEGIN(Client)

class CPlayer final : public CAnimMesh
{
	enum AnimParts {PARTS_PLAYER, PARTS_TOP, PARTS_BOTTOM, PARTS_END };
private:
	CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayer(const CPlayer& rhs);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CAnimModel* m_pAnimModel[PARTS_END];

public:
	static CAnimMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
