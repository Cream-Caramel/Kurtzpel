#pragma once
#include "Client_Defines.h"
#include "Base.h"

BEGIN(Client)
class CCamera_Player;

class CCamera_Manager final : public CBase
{
	DECLARE_SINGLETON(CCamera_Manager)
public:
	CCamera_Manager();
	virtual ~CCamera_Manager() = default;

public:
	void Add_Camera_Player(CCamera_Player* pPlayerCam);
	_bool Get_bScene() { return m_bScene; }
	void Load_Scene(char* FileName);
private:
	_bool m_bScene = false;
	vector<POSINFO> m_Stage1Pos;
	vector<LOOKINFO> m_Stage1Look;

private:
	CCamera_Player* m_pCamPlayer = nullptr;

public:
	virtual void Free() override;
};

END