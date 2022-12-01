#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Player.h"
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
	_float3 Get_PlayerPos();
	CAnimMesh* Get_BossPointer() { return m_pBoss; }
	_float3 Get_BossPos();
	_float3 Get_BossLook();
	_float3 Get_BossRight();
	void Add_CameraPlayer(CCamera_Player* pCameraPlayer);
	CCamera_Player* Get_CameraPlayer() { return m_pCameraPlayer; }
	_float3 Get_CameraPlayerPos();
	LEVEL Get_NowLevel() { return m_eLevel; }
	void Set_Level(LEVEL eLevel) { m_eLevel = eLevel; }
	void Set_NextLevel(LEVEL eLevel) { m_eNextLevel = eLevel; }
	void Change_Level(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	HRESULT Add_Monster(const char* Level);
	HRESULT Add_Boss(CAnimMesh* pBoss);
	HRESULT Delete_Boss();
	_bool Get_BossFinish();
	void Set_PlayerGage(_bool bPlayerGage) { m_bPlayerGage = bPlayerGage; }
	_bool Get_PlayerGage() { return m_bPlayerGage; }
	void Set_PlayerGage2_1(_bool bPlayerGage) { m_bPlayerGage2_1 = bPlayerGage; }
	_bool Get_PlayerGage2_1() { return m_bPlayerGage2_1; }
	void Set_PlayerGage2_2(_bool bPlayerGage) { m_bPlayerGage2_2 = bPlayerGage; }
	_bool Get_PlayerGage2_2() { return m_bPlayerGage2_2; }
	_bool Get_GolemRockOn();
	CPlayer::STATE Get_PlayerState();
	

private:
	LEVEL m_eLevel;
	LEVEL m_eNextLevel;
	CPlayer* m_pPlayer;
	CAnimMesh* m_pBoss = nullptr;
	CCamera_Player* m_pCameraPlayer;
	_bool m_bPlayerGage = false;
	_bool m_bPlayerGage2_1 = false;
	_bool m_bPlayerGage2_2 = false;
public:
	virtual void Free() override;
};
END
