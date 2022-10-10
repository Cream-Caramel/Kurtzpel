#include "stdafx.h"
#include "..\Public\UI_Manager.h"
#include "Player.h"

IMPLEMENT_SINGLETON(CUI_Manager)

CUI_Manager::CUI_Manager()
{
}

void CUI_Manager::Add_Player(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
	Safe_AddRef(m_pPlayer);
}

_bool CUI_Manager::Get_UseSkill()
{
	 return m_pPlayer->Get_UseSkill(); 
}

void CUI_Manager::Free()
{
	Safe_Release(m_pPlayer);
}
