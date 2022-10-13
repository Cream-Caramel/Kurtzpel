#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Player.h"
BEGIN(Client)
class CPlayer;
class CSkillFrame;
class CKeyQ;
class CKeyE;
class CKeyR;
class CKeyF;
class CKeyShift;
class CKeyTab;
class CExGauge;
class CUI_Manager final : public CBase
{
	DECLARE_SINGLETON(CUI_Manager)
public:
	CUI_Manager();
	virtual ~CUI_Manager() = default;

public:
	void Add_Player(CPlayer* pPlayer); //�÷��̾� �������߰�
	void Add_Boss(CGameObject* pBoss);
	void Add_ExGauge(CExGauge* pGauge);
	_bool Get_UseSkill(); // �÷��̾��� ���°� ��ų��밡���ѻ������� �Ǵ�
	_float Get_PlayerHp();
	_float Get_PlayerMaxHp();
	_float Get_PlayerMp();
	_float Get_BossHp();
	_float Get_BossMaxHp();
	_float Get_BossMp();
	_bool Get_Die();
	_bool Get_Respwan();
	void Set_PlayerHp(_float iHp);
	void Set_PlayerMp(_float iMp);
	void Set_BossHp(_float iHp);
	void Set_BossMp(_float iMp);
	void Add_SkillFrame(CSkillFrame* pSkillFrame); // ��ų������ �������߰�
	_bool Get_CoolTime(int iIndex); // ��ų�������� ��Ÿ������ Ȯ��
	CPlayer::STATE Get_PlayerState();
	void Set_ExGaugeTex(_uint iTexIndex);
	_uint Get_ExGaugeTex();
	void Set_CoolTime(int iIndex);
	void AddKeyR(CKeyR* pKeyR);
	void AddKeyE(CKeyE* pKeyE);
	void AddKeyF(CKeyF* pKeyF);
	void AddKeyQ(CKeyQ* pKeyQ);
	void AddKeyShift(CKeyShift* pKeyShift);
	void AddKeyTab(CKeyTab* pKeyTab);
	void Set_KeyDown(int iIndex);
	void Reset_ExGaugeTex();

private:
	CPlayer* m_pPlayer = nullptr;
	CExGauge* m_pExGauge = nullptr;
	CKeyR* m_pKeyR = nullptr;
	CKeyE* m_pKeyE = nullptr;
	CKeyQ* m_pKeyQ = nullptr;
	CKeyF* m_pKeyF = nullptr;
	CKeyShift* m_pKeyShift = nullptr;
	CKeyTab* m_pKeyTab = nullptr;	
	vector<CSkillFrame*> m_SkillFrame;
	CGameObject* m_pBoss = nullptr;

public:
	virtual void Free() override;
};

END

