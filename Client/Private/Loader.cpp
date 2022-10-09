#include "stdafx.h"
#include "..\Public\Loader.h"

#include "GameInstance.h"
#include "Camera_Free.h"
#include "Terrain.h"
#include "Model.h"
#include "AnimModel.h"
#include "Release_Manager.h"
#include "UI.h"
#include "Texture.h"
#include "Player.h"
#include "Camera_Player.h"
#include "PlayerHead.h"
#include "PlayerHairBack.h"
#include "PlayerHairFront.h"
#include "PlayerHairSide.h"
#include "PlayerHairTail.h"
#include "PlayerSword.h"
#include "SkillFrame.h"

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

_uint APIENTRY LoadingMain(void* pArg)
{
	CoInitializeEx(nullptr, 0);

	CLoader*		pLoader = (CLoader*)pArg;

	EnterCriticalSection(&pLoader->Get_CS());

	switch (pLoader->Get_NextLevelID())
	{
	case LEVEL_STATIC:
		pLoader->Loading_ForStatic();
	case LEVEL_LOGO:
		pLoader->Loading_ForLogoLevel();
		break;
	case LEVEL_STAGE1:
		pLoader->Loading_ForStage1();
		break;
	}
	
	LeaveCriticalSection(&pLoader->Get_CS());

	return 0;
}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_CriticalSection);

	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);

	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading_ForStatic()
{
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	lstrcpy(m_szLoadingText, TEXT("객체원형 로딩중"));

	/* 개ㅑㄱ체원형 로드한다. */
	/* For.Prototype_GameObject_Terrain*/
	if (FAILED(pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Camera_Free */
	if (FAILED(pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Camera_Player"),
		CCamera_Player::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerHead"),
		CPlayerHead::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerHairBack"),
		CPlayerHairBack::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerHairFront"),
		CPlayerHairFront::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerHairSide"),
		CPlayerHairSide::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerHairTail"),
		CPlayerHairTail::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerSword"),
		CPlayerSword::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("UI 로딩중"));

	/* For.Prototype_Component_Texture_Terrain */
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STAGE1, TEXT("Prototype_Component_Texture_Terrain"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Grass_%d.dds"), 2))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillFrame"),
		CSkillFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("모델 로딩중 "));

	LoadAnimModel("Level_Static");
	LoadModel("Level_Static");
	LoadTexture("Level_Static");
	

	/* For.Prototype_Component_VIBuffer_Terrain */
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Height.bmp")))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_Terrain */
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Terrain"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX_DECLARATION::Elements, VTXNORTEX_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_ColorCube*/
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Cube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cube.hlsl"), VTXCUBE_DECLARATION::Elements, VTXCUBE_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_Model */
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Model"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMODEL_DECLARATION::Elements, VTXMODEL_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_AnimModel */
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_AnimModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel.hlsl"), VTXANIMMODEL_DECLARATION::Elements, VTXANIMMODEL_DECLARATION::iNumElements))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다.  "));

	Safe_Release(pGameInstance);

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_ForLogoLevel()
{
	return S_OK;
}

HRESULT CLoader::Loading_ForStage1()
{
	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::LoadAnimModel(char * DatName)
{
	string FileSave = DatName;

	string temp = "../Data/AnimModelData/";

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
		return E_FAIL;
	}


	DWORD		dwByte = 0;


	while (true)
	{

		_tchar* ProtoName = new _tchar[256];
		ReadFile(hFile, ProtoName, sizeof(_tchar) * 256, &dwByte, nullptr);
		char* ModelName = new char[256];
		ReadFile(hFile, ModelName, sizeof(char) * 256, &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete_Array(ProtoName);
			Safe_Delete_Array(ModelName);
			break;
		}
		
		if (FAILED(GI->Add_Prototype(LEVEL_STATIC, ProtoName,
			CAnimModel::Create(m_pDevice, m_pContext, ModelName))))
		{
			Safe_Delete(ProtoName);
			
			return E_FAIL;
		}
		RM->Pushtchar(ProtoName);
		Safe_Delete(ModelName);
	}
	// 3. 파일 소멸
	CloseHandle(hFile);
	return S_OK;
	
}

HRESULT CLoader::LoadModel(char * DatName)
{
	string FileSave = DatName;

	string temp = "../Data/ModelData/";

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
		return E_FAIL;
	}

	DWORD		dwByte = 0;

	while (true)
	{
		_tchar* ProtoName = new _tchar[256];
		ReadFile(hFile, ProtoName, sizeof(_tchar) * 256, &dwByte, nullptr);
		char* ModelName = new char[256];
		ReadFile(hFile, ModelName, sizeof(char) * 256, &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete_Array(ProtoName);
			Safe_Delete_Array(ModelName);
			break;
		}

		if (FAILED(GI->Add_Prototype(LEVEL_STATIC, ProtoName,
			CModel::Create(m_pDevice, m_pContext, ModelName))))
		{
			Safe_Delete(ProtoName);
			return E_FAIL;
		}
		RM->Pushtchar(ProtoName);
		Safe_Delete(ModelName);
	}
	// 3. 파일 소멸
	CloseHandle(hFile);
	return S_OK;
}

HRESULT CLoader::LoadTexture(char * DatName)
{
	string FileSave = DatName;

	string temp = "../Data/TextureData/";

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
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return E_FAIL;
	}

	DWORD		dwByte = 0;


	while (true)
	{
		
		_tchar Path[256];
		ReadFile(hFile, &Path, sizeof(_tchar) * 256, &dwByte, nullptr);
		_tchar* pPath = new _tchar[256];
		lstrcpyW(pPath, Path);
		_tchar Name[256];
		ReadFile(hFile, &Name, sizeof(_tchar) * 256, &dwByte, nullptr);
		_tchar* pName = new _tchar[256];
		lstrcpyW(pName, Name);
		int iTexNums;
		ReadFile(hFile, &iTexNums, sizeof(int), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete(pPath);
			Safe_Delete(pName);
			break;
		}
		if (FAILED(GI->Add_Prototype(LEVEL_STATIC, pName,
			CTexture::Create(m_pDevice, m_pContext, pPath, iTexNums))))
			return E_FAIL;
		
		RM->Pushtchar(pPath);
		RM->Pushtchar(pName);
	}

	// 3. 파일 소멸
	CloseHandle(hFile);

	return S_OK;
}

CLoader * CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLoader*		pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Created : CLoader"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	WaitForSingleObject(m_hThread, INFINITE);

	DeleteCriticalSection(&m_CriticalSection);

	CloseHandle(m_hThread);

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);


}
