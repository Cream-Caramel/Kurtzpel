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

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return;
	}

	// 2. 파일 쓰기

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

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
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

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
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

	// 3. 파일 소멸
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
