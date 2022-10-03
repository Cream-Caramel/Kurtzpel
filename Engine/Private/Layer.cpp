#include "..\Public\Layer.h"
#include "GameObject.h"

CLayer::CLayer()
{
}

CComponent * CLayer::Get_ComponentPtr(const _tchar * pComponentTag, _uint iLayerIndex)
{
	auto	iter = m_GameObjects.begin();

	for (_uint i = 0; i < iLayerIndex; ++i)
		++iter;

	return (*iter)->Get_ComponentPtr(pComponentTag);	
}

HRESULT CLayer::Add_GameObject(CGameObject * pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_GameObjects.push_back(pGameObject);

	return S_OK;
}

HRESULT CLayer::Initialize()
{

	return S_OK;
}

void CLayer::Tick(_float fTimeDelta)
{
	for (list<class CGameObject*>::iterator iter = m_GameObjects.begin(); iter != m_GameObjects.end();)
	{
		if (nullptr != *iter)
			(*iter)->Tick(fTimeDelta);
		
		if ((*iter)->Get_Dead())
		{
			Safe_Release(*iter);
			iter = m_GameObjects.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void CLayer::LateTick(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->LateTick(fTimeDelta);
	}
}

CLayer * CLayer::Create()
{
	CLayer*			pInstance = new CLayer();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLayer::Free()
{
	for (auto& pGameObject : m_GameObjects)
		Safe_Release(pGameObject);

	m_GameObjects.clear();
}


