#include "stdafx.h"
#include "..\Public\Camera_Manager.h"
#include "Camera_Player.h"


IMPLEMENT_SINGLETON(CCamera_Manager)

CCamera_Manager::CCamera_Manager()
{
	Load_Scene("PlayerDoubleSlash");
	Load_Scene("Scene_Stage1");
	Load_Scene("Scene_Stage2");
	Load_Scene("Scene_Stage2Boss");
	Load_Scene("Scene_Stage3");
	Load_Scene("Scene_Stage3Boss");
	Load_Scene("Scene_Stage4");
	Load_Scene("Scene_Stage4Boss");

}

void CCamera_Manager::Add_Camera_Player(CCamera_Player * pPlayerCam)
{
	m_pCamPlayer = pPlayerCam;
	Safe_AddRef(m_pCamPlayer);
}

void CCamera_Manager::Start_Scene(const char * SceneName)
{
	m_bScene = true;

	for (auto& iter : m_CamPosInfo)
	{
		if (!strcmp(SceneName, iter.first))
			m_pCamPlayer->Set_ScenePosInfo(iter.second);
	}

	for (auto& iter : m_CamLookInfo)
	{
		if (!strcmp(SceneName, iter.first))
			m_pCamPlayer->Set_SceneLookInfo(iter.second);
	}
}

void CCamera_Manager::End_Scene()
{
	m_bScene = false;
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

	vector<POSINFO> PosInfos;

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

		PosInfos.push_back(PosInfo);
		
	}

	Push_CamPosInfo(FileName, PosInfos);


	int LookCubeSize;
	ReadFile(hFile, &LookCubeSize, sizeof(int), &dwByte, nullptr);

	vector<LOOKINFO> LookInfos;

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

		LookInfos.push_back(LookInfo);
	
	}

	Push_CamLookInfo(FileName, LookInfos);

	// 3. ���� �Ҹ�
	CloseHandle(hFile);
}

void CCamera_Manager::Push_CamPosInfo(const char* FileName, vector<POSINFO> PosInfos)
{
	if (!strcmp(FileName, "PlayerDoubleSlash"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}

	if (!strcmp(FileName, "Scene_Stage1"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}
	if (!strcmp(FileName, "Scene_Stage2"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}
	if (!strcmp(FileName, "Scene_Stage2Boss"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}
	if (!strcmp(FileName, "Scene_Stage3"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}
	if (!strcmp(FileName, "Scene_Stage3Boss"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}
	if (!strcmp(FileName, "Scene_Stage4"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}
	if (!strcmp(FileName, "Scene_Stage4Boss"))
	{
		m_CamPosInfo.push_back(make_pair(FileName, PosInfos));
	}

}

void CCamera_Manager::Push_CamLookInfo(const char * FileName, vector<LOOKINFO> LookInfos)
{
	if (!strcmp(FileName, "PlayerDoubleSlash"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}

	if (!strcmp(FileName, "Scene_Stage1"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}
	if (!strcmp(FileName, "Scene_Stage2"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}
	if (!strcmp(FileName, "Scene_Stage2Boss"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}
	if (!strcmp(FileName, "Scene_Stage3"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}
	if (!strcmp(FileName, "Scene_Stage3Boss"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}
	if (!strcmp(FileName, "Scene_Stage4"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}
	if (!strcmp(FileName, "Scene_Stage4Boss"))
	{
		m_CamLookInfo.push_back(make_pair(FileName, LookInfos));
	}

}

void CCamera_Manager::Start_Shake(_float fShakeTime, _float fShakePower, _float fShakeSpeed)
{
	m_pCamPlayer->Start_Shake(fShakeTime, fShakePower, fShakeSpeed);
}

void CCamera_Manager::Start_Fov(_float fFov, _float fFovSpeed)
{
	m_pCamPlayer->Start_Fov(fFov, fFovSpeed);
	m_bFov = true;
	m_bFovDIr = false;
}

void CCamera_Manager::Fix_Fov(_float fFov, _float fFovSpeed)
{
	m_pCamPlayer->FixFov(fFov, fFovSpeed);
}

void CCamera_Manager::EndFov()
{
	m_bFov = false;
}

void CCamera_Manager::Set_FovSpeed(_float fFovSpeed)
{
	m_pCamPlayer->Set_FovSpeed(fFovSpeed);
}

void CCamera_Manager::Free()
{
	Safe_Release(m_pCamPlayer);
}
