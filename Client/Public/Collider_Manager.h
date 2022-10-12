#pragma once

#include "Base.h"
#include "Client_Defines.h"

BEGIN(Engine)
class CGameObject;
END

BEGIN(Client)

class CCollider_Manager : public CBase
{
	DECLARE_SINGLETON(CCollider_Manager)

public:
	enum COLLIDER_GROUP {COLLIDER_PLAYER, COLLIDER_MONSTER, COLLIDER_END};

public:
	CCollider_Manager();
	virtual ~CCollider_Manager() = default;

public:
	HRESULT Check_Collision(COLLIDER_GROUP eGroup1, COLLIDER_GROUP eGroup2);
	HRESULT Check_OBB(CGameObject* pObject1, CGameObject* pObject2);

	HRESULT Add_OBBObject(COLLIDER_GROUP eGroup, CGameObject* pObject);
	void End_Collision();

private:
	list<CGameObject*> m_OBBObjects[COLLIDER_END];

	virtual void Free() override;
};

END