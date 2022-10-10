#include "stdafx.h"
#include "..\Public\UI_Manager.h"
#include "Player.h"
#include "SkillFrame.h"

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

void CUI_Manager::Add_SkillFrame(CSkillFrame * pSkillFrame)
{
	m_SkillFrame.push_back(pSkillFrame);
	Safe_AddRef(pSkillFrame);
}

_bool CUI_Manager::Get_CoolTime(int iIndex)
{
	return m_SkillFrame[iIndex]->Get_CoolTime();
}

void CUI_Manager::Set_CoolTime(int iIndex)
{
	m_SkillFrame[iIndex]->Set_CoolTime();
}

void CUI_Manager::Free()
{
	Safe_Release(m_pPlayer);

	for (auto& iter : m_SkillFrame)
	{
		Safe_Release(iter);
	}
	m_SkillFrame.clear();
}
