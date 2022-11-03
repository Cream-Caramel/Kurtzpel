#pragma once
#include "Base.h"
#include "Client_Defines.h"
BEGIN(Client)
class CPlayer;
class CCamera_Player;
class CAnimMesh;

class CPointer_Manager final : public CBase
{
	DECLARE_SINGLETON(CPointer_Manager)
private:
	CPointer_Manager();
	virtual ~CPointer_Manager() = default;

public:
	void Add_Player(CPlayer* pPlayer);
	CPlayer* Get_PlayerPointer() { return m_pPlayer; }
	CAnimMesh* Get_BossPointer() { return m_pBoss; }
	void Add_CameraPlayer(CCamera_Player* pCameraPlayer);
	CCamera_Player* Get_CameraPlayer() { return m_pCameraPlayer; }
	_float3 Get_CameraPlayerPos();
	LEVEL Get_NowLevel() { return m_eLevel; }
	void Set_Level(LEVEL eLevel) { m_eLevel = eLevel; }
	HRESULT Add_Monster(const char* Level);
	HRESULT Add_Boss(CAnimMesh* pBoss);
	HRESULT Delete_Boss();
	_bool Get_BossFinish();

private:
	LEVEL m_eLevel;
	CPlayer* m_pPlayer;
	CAnimMesh* m_pBoss = nullptr;
	CCamera_Player* m_pCameraPlayer;
public:
	virtual void Free() override;
};
END
