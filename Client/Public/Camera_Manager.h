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
	_bool Get_bFov() { return m_bFov; }
	_bool Get_bFovDir() { return m_bFovDIr; }
	void Start_Scene(const char* SceneName);
	void Set_PlayerScene(_bool bPlayerScene) { m_bPlayerScene = bPlayerScene; }
	void End_Scene();
	void Load_Scene(char* FileName);
	void Push_CamPosInfo(const char* FileName, vector<POSINFO> PosInfos);
	void Push_CamLookInfo(const char* FileName, vector<LOOKINFO> LookInfos);
	void Start_Shake(_float fShakeTime, _float fShakePower, _float fShakeSpeed);
	void Start_Fov(_float fFov, _float fFovSpeed);
	void Fix_Fov(_float fFov, _float fFovSpeed);
	void EndFov();
	void Set_Fov(_bool bFov) { m_bFov = bFov; }
	void Set_FovDir(_bool bFovDir) { m_bFovDIr = bFovDir; }
	void Set_FovSpeed(_float fFovSpeed);
	void Set_StopScene(_bool bStopScene);
	_float Get_Fov();
private:
	_bool m_bScene = false;
	_bool m_bPlayerScene = false;
	_bool m_bFov = false;
	_bool m_bFovDIr = false;
	list<pair<const char*,vector<POSINFO>>> m_CamPosInfo;
	list<pair<const char*,vector<LOOKINFO>>> m_CamLookInfo;

private:
	CCamera_Player* m_pCamPlayer = nullptr;

public:
	virtual void Free() override;
};

END