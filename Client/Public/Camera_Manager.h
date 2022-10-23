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
	_bool Get_bPlayerScene() { return m_bPlayerScene; }
	void Start_Scene(const char* SceneName);
	void Set_PlayerScene() { m_bPlayerScene = true; }
	void End_Scene();
	void Load_Scene(char* FileName);
	void Push_CamPosInfo(const char* FileName, vector<POSINFO> PosInfos);
	void Push_CamLookInfo(const char* FileName, vector<LOOKINFO> LookInfos);
private:
	_bool m_bScene = false;
	_bool m_bPlayerScene = false;
	list<pair<const char*,vector<POSINFO>>> m_CamPosInfo;
	list<pair<const char*,vector<LOOKINFO>>> m_CamLookInfo;

private:
	CCamera_Player* m_pCamPlayer = nullptr;

public:
	virtual void Free() override;
};

END