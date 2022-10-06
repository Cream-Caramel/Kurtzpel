#pragma once
#include "Base.h"
#include "Client_Defines.h"
BEGIN(Client)
class CPlayer;
class CCamera_Player;

class CPointer_Manager final : public CBase
{
	DECLARE_SINGLETON(CPointer_Manager)
private:
	CPointer_Manager();
	virtual ~CPointer_Manager() = default;

public:
	void Add_Player(CPlayer* pPlayer);
	CPlayer* Get_PlayerPointer() { return m_pPlayer; }

	void Add_CameraPlayer(CCamera_Player* pCameraPlayer);
	CCamera_Player* Get_CameraPlayer() { return m_pCameraPlayer; }
	_float3 Get_CameraPlayerPos();
private:
	CPlayer* m_pPlayer;
	CCamera_Player* m_pCameraPlayer;
public:
	virtual void Free() override;
};
END
