#include "stdafx.h"
#include "..\Public\Release_Manager.h"

IMPLEMENT_SINGLETON(CRelease_Manager)

CRelease_Manager::CRelease_Manager()
{
}

void CRelease_Manager::Free()
{
	for (auto& iter : m_tchars)
	{
		Safe_Delete(iter);
	}
	m_tchars.clear();
}
