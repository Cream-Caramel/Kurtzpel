#include "stdafx.h"
#include "..\Public\UI_Manager.h"
#include "UIInclude.h"


IMPLEMENT_SINGLETON(CUI_Manager)

CUI_Manager::CUI_Manager()
{
}

void CUI_Manager::Add_Player(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
	Safe_AddRef(m_pPlayer);
}

void CUI_Manager::Add_Boss(CAnimMesh * pBoss)
{
	m_pBoss = pBoss;
	Safe_AddRef(m_pBoss);
}

void CUI_Manager::Add_ExGauge(CExGauge * pGauge)
{
	m_pExGauge = pGauge;
	Safe_AddRef(m_pExGauge);
}

void CUI_Manager::Add_Count(CCount * pCount)
{
	m_pCount = pCount;
	Safe_AddRef(m_pCount);
}

_uint CUI_Manager::Get_Count()
{
	return m_pCount->Get_Count();
}

void CUI_Manager::Set_Count(_uint iCount)
{
	m_pCount->Set_Count(iCount);
}

_bool CUI_Manager::Get_UseSkill()
{
	 return m_pPlayer->Get_UseSkill(); 
}

_float CUI_Manager::Get_PlayerHp()
{
	return m_pPlayer->Get_Hp();
}

_float CUI_Manager::Get_PlayerMaxHp()
{
	return m_pPlayer->Get_MaxHp();
}

_float CUI_Manager::Get_PlayerMp()
{
	return m_pPlayer->Get_Mp();
}

_float CUI_Manager::Get_BossHp()
{
	return m_pBoss->Get_Hp();
}

_float CUI_Manager::Get_BossMaxHp()
{
	return m_pBoss->Get_MaxHp();
}

_float CUI_Manager::Get_BossMp()
{
	return m_pBoss->Get_Mp();
}

_bool CUI_Manager::Get_Die()
{
	return m_pPlayer->Get_Die();
}

_bool CUI_Manager::Get_Respwan()
{
	return m_pPlayer->Get_Respawn();
}

_bool CUI_Manager::Get_BossDie()
{
	return m_pBoss->Get_Die();
}

void CUI_Manager::Set_PlayerHp(_float iHp)
{
	m_pPlayer->Set_Hp(iHp);
}

void CUI_Manager::Set_PlayerMp(_float iMp)
{
	m_pPlayer->Set_Mp(iMp);
}

void CUI_Manager::Set_BossHp(_float iHp)
{
	m_pBoss->Set_Hp(iHp);
}

void CUI_Manager::Set_BossMp(_float iMp)
{
	m_pBoss->Set_Mp(iMp);
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

CPlayer::STATE CUI_Manager::Get_PlayerState()
{
	return m_pPlayer->Get_State();
}

void CUI_Manager::Set_ExGaugeTex(_uint iTexIndex)
{
	m_pExGauge->Set_TextureNum(iTexIndex);
}

_uint CUI_Manager::Get_ExGaugeTex()
{
	return m_pExGauge->Get_TextureNum();
}

void CUI_Manager::Set_CoolTime(int iIndex)
{
	m_SkillFrame[iIndex]->Set_CoolTime();
}

void CUI_Manager::AddKeyR(CKeyR * pKeyR)
{
	m_pKeyR = pKeyR;
	Safe_AddRef(m_pKeyR);
}

void CUI_Manager::AddKeyE(CKeyE * pKeyE)
{
	m_pKeyE = pKeyE;
	Safe_AddRef(m_pKeyE);
}

void CUI_Manager::AddKeyF(CKeyF * pKeyF)
{
	m_pKeyF = pKeyF;
	Safe_AddRef(m_pKeyF);
}

void CUI_Manager::AddKeyQ(CKeyQ * pKeyQ)
{
	m_pKeyQ = pKeyQ;
	Safe_AddRef(m_pKeyQ);
}

void CUI_Manager::AddKeyShift(CKeyShift * pKeyShift)
{
	m_pKeyShift = pKeyShift;
	Safe_AddRef(m_pKeyShift);
}

void CUI_Manager::AddKeyTab(CKeyTab * pKeyTab)
{
	m_pKeyTab = pKeyTab;
	Safe_AddRef(m_pKeyTab);
}

void CUI_Manager::Set_KeyDown(int iIndex)
{
	switch (iIndex)
	{
	case 0:
		m_pKeyQ->Set_Down();
		break;
	case 1:
		m_pKeyE->Set_Down();
		break;
	case 2:
		m_pKeyF->Set_Down();
		break;
	case 3:
		m_pKeyR->Set_Down();
		break;
	case 4:
		m_pKeyTab->Set_Down();
		break;
	case 5:
		m_pKeyShift->Set_Down();
		break;

	default:
		break;
	}
}

void CUI_Manager::Reset_ExGaugeTex()
{
	m_pExGauge->Reset_TextureNum();
}


void CUI_Manager::Free()
{
	Safe_Release(m_pPlayer);
	Safe_Release(m_pExGauge);
	Safe_Release(m_pKeyTab);
	Safe_Release(m_pKeyE);
	Safe_Release(m_pKeyF);
	Safe_Release(m_pKeyQ);
	Safe_Release(m_pKeyR);
	Safe_Release(m_pKeyShift);
	Safe_Release(m_pBoss);
	Safe_Release(m_pCount);
	for (auto& iter : m_SkillFrame)
	{
		Safe_Release(iter);
	}
	m_SkillFrame.clear();
}
