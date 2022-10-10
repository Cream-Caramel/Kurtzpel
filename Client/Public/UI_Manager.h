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
	void Add_Player(CPlayer* pPlayer); //플레이어 포인터추가
	_bool Get_UseSkill(); // 플레이어의 상태가 스킬사용가능한상태인지 판단
	void Add_SkillFrame(CSkillFrame* pSkillFrame); // 스킬프레임 포인터추가
	_bool Get_CoolTime(int iIndex); // 스킬프레임이 쿨타임인지 확인
	void Set_CoolTime(int iIndex);


private:
	CPlayer* m_pPlayer = nullptr;
	vector<CSkillFrame*> m_SkillFrame;

public:
	virtual void Free() override;
};

END

