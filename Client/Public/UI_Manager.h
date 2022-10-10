#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN(Client)
class CPlayer;
class CSkillFrame;
class CUI_Manager final : public CBase
{
	DECLARE_SINGLETON(CUI_Manager)
public:
	CUI_Manager();
	virtual ~CUI_Manager() = default;

public:
	void Add_Player(CPlayer* pPlayer); //�÷��̾� �������߰�
	_bool Get_UseSkill(); // �÷��̾��� ���°� ��ų��밡���ѻ������� �Ǵ�
	void Add_SkillFrame(CSkillFrame* pSkillFrame); // ��ų������ �������߰�
	_bool Get_CoolTime(int iIndex); // ��ų�������� ��Ÿ������ Ȯ��
	void Set_CoolTime(int iIndex);


private:
	CPlayer* m_pPlayer = nullptr;
	vector<CSkillFrame*> m_SkillFrame;

public:
	virtual void Free() override;
};

END

