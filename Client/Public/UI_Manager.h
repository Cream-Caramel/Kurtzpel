#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN(Client)
class CPlayer;
class CUI_Manager final : public CBase
{
	DECLARE_SINGLETON(CUI_Manager)
public:
	CUI_Manager();
	virtual ~CUI_Manager() = default;

public:
	void Add_Player(CPlayer* pPlayer);
	_bool Get_UseSkill();

private:
	CPlayer* m_pPlayer = nullptr;

public:
	virtual void Free() override;
};

END

