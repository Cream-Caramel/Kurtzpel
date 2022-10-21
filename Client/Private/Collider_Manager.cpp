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
	for (auto& Pair1 : m_Pairs[eGroup1])
	{
		for (auto& Pair2 : m_Pairs[eGroup2])
		{
			if (Pair1.second->Collision(Pair2.second))
			{
				Pair1.first->Collision(Pair2.first, Pair2.second->Get_Tag().c_str());
				Pair2.first->Collision(Pair1.first, Pair1.second->Get_Tag().c_str());
			}
		}
	}

	return S_OK;
}


HRESULT CCollider_Manager::Add_OBBObject(COLLIDER_GROUP eGroup, CGameObject* pObject, COBB* pOBB)
{
	if (pObject == nullptr)
		return E_FAIL;
	
	m_Pairs[eGroup].push_back(make_pair(pObject,pOBB));
	Safe_AddRef(pObject);
	Safe_AddRef(pOBB);
	return S_OK;
}

HRESULT CCollider_Manager::Collider_Render()
{
	if (m_bColliderRender)
	{
		for (int i = 0; i < COLLIDER_END; ++i)
		{
			for (auto& iter : m_Pairs[i])
			{
				iter.second->Render();
			}
		}
	}
	return S_OK;
}

void CCollider_Manager::End_Collision()
{
	for (int i = 0; i < COLLIDER_END; ++i)
	{
		for (auto& iter : m_Pairs[i])
		{
			Safe_Release(iter.first);
			Safe_Release(iter.second);
		}
		m_Pairs[i].clear();
	}
}


void CCollider_Manager::Free()
{
	End_Collision();
}
