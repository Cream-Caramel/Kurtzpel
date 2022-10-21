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

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
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

	// 3. 파일 소멸
	CloseHandle(hFile);
}

void CCamera_Manager::Free()
{
	Safe_Release(m_pCamPlayer);
}
