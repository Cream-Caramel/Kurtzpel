#include "stdafx.h"
#include "..\Public\Collider_Manager.h"
#include "GameObject.h"
#include "OBB.h"
IMPLEMENT_SINGLETON(CCollider_Manager)

CCollider_Manager::CCollider_Manager()
{
}

HRESULT CCollider_Manager::Check_Collision(COLLIDER_GROUP eGroup1, COLLIDER_GROUP eGroup2)
{
	for (auto& Object1 : m_OBBObjects[eGroup1])
	{
		for (auto& Object2 : m_OBBObjects[eGroup2])
		{

		}
	}

	return S_OK;
}

HRESULT CCollider_Manager::Check_OBB(CGameObject * pObject1, CGameObject * pObject2)
{
	for (auto& OBB1 : pObject1->Get_OBBs())
	{
		for (auto& OBB2 : pObject2->Get_OBBs())
		{
			if (OBB1->Collision(OBB2))
			{
				pObject1->Collision(pObject2, OBB2->Get_Tag().c_str());
				pObject2->Collision(pObject1, OBB1->Get_Tag().c_str());
			}
		}
	}
	return S_OK;
}

HRESULT CCollider_Manager::Add_OBBObject(COLLIDER_GROUP eGroup, CGameObject* pObject)
{
	if (pObject == nullptr)
		return E_FAIL;

	m_OBBObjects[eGroup].push_back(pObject);

	return S_OK;
}

void CCollider_Manager::End_Collision()
{
	for (int i = 0; i < COLLIDER_END; ++i)
	{
		for (auto& iter : m_OBBObjects[i])
		{
			Safe_Release(iter);
		}
		m_OBBObjects[i].clear();
	}
}


void CCollider_Manager::Free()
{
	End_Collision();
}
