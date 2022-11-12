#include "stdafx.h"
#include "..\Public\Pointer_Manager.h"
#include "Player.h"
#include "Camera_Player.h"
#include "GameInstance.h"
#include "AnimMesh.h"

IMPLEMENT_SINGLETON(CPointer_Manager)

CPointer_Manager::CPointer_Manager()
{
}

void CPointer_Manager::Add_Player(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
	Safe_AddRef(m_pPlayer);
}

_float3 CPointer_Manager::Get_PlayerPos()
{
	return m_pPlayer->Get_Pos();
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

HRESULT CPointer_Manager::Add_Monster(const char * Level)
{

	string FileSave = Level;

	string temp = "../Data/AnimModelObject/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return E_FAIL;
	}

	DWORD		dwByte = 0;

	int ModelObjectSize;
	ReadFile(hFile, &ModelObjectSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ModelObjectSize; ++i)
	{
		char Name[256];
		ReadFile(hFile, Name, sizeof(char) * 256, &dwByte, nullptr);
	}

	while (true)
	{
		_tchar* ModelName = new _tchar[256];
		ReadFile(hFile, ModelName, sizeof(_tchar) * 256, &dwByte, nullptr);

		_float3 fScale;
		ReadFile(hFile, &fScale, sizeof(_float3), &dwByte, nullptr);

		_float4 fPos;
		ReadFile(hFile, &fPos, sizeof(_float4), &dwByte, nullptr);

		_float3 fAngle;
		ReadFile(hFile, &fAngle, sizeof(_float3), &dwByte, nullptr);

		CAnimMesh::MESHINFO* MeshInfo;
		MeshInfo = new CAnimMesh::MESHINFO;
		MeshInfo->sTag = ModelName;
		MeshInfo->fScale = fScale;
		MeshInfo->fPos = fPos;
		MeshInfo->fAngle = fAngle;

		if (0 == dwByte)
		{
			Safe_Delete_Array(ModelName);
			Safe_Delete(MeshInfo);
			break;
		}
		wstring Create = ModelName;

		if (FAILED(GI->Add_GameObjectToLayer(ModelName, LEVEL_STATIC, L"Layer_AnimModelObject", MeshInfo)))
		{
			wstring a = L"Please Load ProtoType";
			wstring b = a + ModelName;
			const _tchar* c = b.c_str();
			MSG_BOX(c);

			Safe_Delete_Array(ModelName);
			Safe_Delete(MeshInfo);
			return E_FAIL;
		}

		Safe_Delete_Array(ModelName);
		Safe_Delete(MeshInfo);
	}

	CloseHandle(hFile);

	return S_OK;
}

HRESULT CPointer_Manager::Add_Boss(CAnimMesh * pBoss)
{
	m_pBoss = pBoss;
	Safe_AddRef(m_pBoss);
	return S_OK;
}

HRESULT CPointer_Manager::Delete_Boss()
{
	Safe_Release(m_pBoss);
	return S_OK;
}

_bool CPointer_Manager::Get_BossFinish()
{
	if (m_pBoss == nullptr)
		return false;
	return m_pBoss->Get_Finish();
}

void CPointer_Manager::Free()
{
	if (m_pBoss != nullptr)
		Safe_Release(m_pBoss);
	Safe_Release(m_pPlayer);
	Safe_Release(m_pCameraPlayer);
}
