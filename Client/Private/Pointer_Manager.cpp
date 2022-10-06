#include "stdafx.h"
#include "..\Public\Pointer_Manager.h"
#include "Player.h"
#include "Camera_Player.h"

IMPLEMENT_SINGLETON(CPointer_Manager)

CPointer_Manager::CPointer_Manager()
{
}

void CPointer_Manager::Add_Player(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
	Safe_AddRef(m_pPlayer);
}

void CPointer_Manager::Add_CameraPlayer(CCamera_Player * pCameraPlayer)
{
	m_pCameraPlayer = pCameraPlayer;
	Safe_AddRef(m_pCameraPlayer);
}

_float3 CPointer_Manager::Get_CameraPlayerPos()
{
	return m_pCameraPlayer->Get_Pos();
}

void CPointer_Manager::Free()
{
	Safe_Release(m_pPlayer);
	Safe_Release(m_pCameraPlayer);
}
