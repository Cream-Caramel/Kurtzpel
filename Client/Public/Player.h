#pragma once
#include "Client_Defines.h"
#include "AnimMesh.h"
#include "Mesh.h"
BEGIN(Engine)
class CHierarchyNode;
END

BEGIN(Client)

class CPlayer final : public CAnimMesh
{
public:
	enum AnimModel {MODEL_PLAYER, MODEL_TOP, MODEL_BOTTOM, MODEL_END};
	enum Parts { PARTS_HEAD, PARTS_HAIRBACK, PARTS_HAIRFRONT, PARTS_HAIRSIDE, PARTS_HAIRTAIL, PARTS_SWORD, PARTS_END };
	enum Socket {SOCKET_HEAD, SOCKET_WEAPONHANDR, SOCKET_END};
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
	int m_iAniIndex;

private:
	CAnimModel* m_pAnimModel[MODEL_END];

	vector<CMesh*>				m_Parts;
	vector<class CHierarchyNode*>		m_Sockets;
	HRESULT Ready_Sockets();
	HRESULT Ready_PlayerParts();
	HRESULT Update_Parts();

public:
	static CAnimMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
