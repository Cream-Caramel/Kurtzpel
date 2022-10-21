#include "stdafx.h"
#include "..\Public\Camera_Manager.h"
#include "Camera_Player.h"


IMPLEMENT_SINGLETON(CCamera_Manager)

CCamera_Manager::CCamera_Manager()
{
	Load_Scene("Scene_Stage1");
}

void CCamera_Manager::Add_Camera_Player(CCamera_Player * pPlayerCam)
{
	m_pCamPlayer = pPlayerCam;
	Safe_AddRef(m_pCamPlayer);
}

void CCamera_Manager::Load_Scene(char * FileName)
{
	string FileSave = FileName;

	string temp = "../Data/SceneData/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// ���� ��ο� �̸� ���
		GENERIC_READ,				// ���� ���� ��� (GENERIC_WRITE ���� ����, GENERIC_READ �б� ����)
		NULL,						// �������, ������ �����ִ� ���¿��� �ٸ� ���μ����� ������ �� ����� ���ΰ�, NULL�� ��� �������� �ʴ´�
		NULL,						// ���� �Ӽ�, �⺻��	
		OPEN_EXISTING,				// ���� ���, CREATE_ALWAYS�� ������ ���ٸ� ����, �ִٸ� ���� ����, OPEN_EXISTING ������ ���� ��쿡�� ����
		FILE_ATTRIBUTE_NORMAL,		// ���� �Ӽ�(�б� ����, ���� ��), FILE_ATTRIBUTE_NORMAL �ƹ��� �Ӽ��� ���� �Ϲ� ���� ����
		NULL);						// �������� ������ �Ӽ��� ������ ���ø� ����, �츮�� ������� �ʾƼ� NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return;
	}

	// 2. ���� ����

	DWORD		dwByte = 0;

	int PosCubeSize;
	ReadFile(hFile, &PosCubeSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < PosCubeSize; ++i)
	{
		_float3 Pos;
		ReadFile(hFile, &Pos, sizeof(_float3), &dwByte, nullptr);

		_float CamSpeed;
		ReadFile(hFile, &CamSpeed, sizeof(_float), &dwByte, nullptr);

		_float StopLimit;
		ReadFile(hFile, &StopLimit, sizeof(_float), &dwByte, nullptr);

		_float4 RGB;
		ReadFile(hFile, &RGB, sizeof(_float4), &dwByte, nullptr);

		if (0 == dwByte)	// ���̻� ���� �����Ͱ� ���� ���
		{
			break;
		}
		POSINFO PosInfo;
		PosInfo.vPos = Pos;
		PosInfo.fCamSpeed = CamSpeed;
		PosInfo.fStopLimit = StopLimit;

		if (!strcmp(FileName, "Scene_Stage1"))
		{
			m_Stage1Pos.push_back(PosInfo);
		}
	}

	int LookCubeSize;
	ReadFile(hFile, &LookCubeSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < LookCubeSize; ++i)
	{
		_float3 Pos;
		ReadFile(hFile, &Pos, sizeof(_float3), &dwByte, nullptr);

		_float CamSpeed;
		ReadFile(hFile, &CamSpeed, sizeof(_float), &dwByte, nullptr);

		_float StopLimit;
		ReadFile(hFile, &StopLimit, sizeof(_float), &dwByte, nullptr);

		_float4 RGB;
		ReadFile(hFile, &RGB, sizeof(_float4), &dwByte, nullptr);

		if (0 == dwByte)	// ���̻� ���� �����Ͱ� ���� ���
		{
			break;
		}
		LOOKINFO LookInfo;
		LookInfo.vPos = Pos;
		LookInfo.fCamSpeed = CamSpeed;
		LookInfo.fStopLimit = StopLimit;

		if (!strcmp(FileName, "Scene_Stage1"))
		{
			m_Stage1Look.push_back(LookInfo);
		}
	
	}

	// 3. ���� �Ҹ�
	CloseHandle(hFile);
}

void CCamera_Manager::Free()
{
	Safe_Release(m_pCamPlayer);
}
