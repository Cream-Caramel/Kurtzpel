#include "stdafx.h"
#include "..\Public\Level_Stage1.h"
#include "GameInstance.h"
#include "Camera_Free.h"
#include "AnimMesh.h"
#include "Mesh.h"
#include "Pointer_Manager.h"
#include "UI.h"
#include "Level_Loading.h"


CLevel_Stage1::CLevel_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Stage1::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;


	if (FAILED(Ready_Load_AnimModel("Level_Stage1")))
		return E_FAIL;

	if (FAILED(Ready_Load_Model("Level_Stage1")))
		return E_FAIL;

	if (FAILED(Ready_UI("Level_Stage1")))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	PM->Get_PlayerPointer()->Create_Navigation("Level_Stage1");

	/*CAnimMesh::MESHINFO MeshInfo;
	MeshInfo.sTag = L"Dragon";
	MeshInfo.fPos = _float4{ 90.f,0.6f,103.f,1.f };
	MeshInfo.fScale = _float3{ 1.f,1.f,1.f };
	MeshInfo.fAngle = _float3{ 1.f,1.f,1.f };
	GI->Add_GameObjectToLayer(L"Dragon", LEVEL_STAGE1, L"Monster", &MeshInfo);*/

	CAnimMesh::MESHINFO MeshInfo;
	MeshInfo.sTag = L"Theo";
	MeshInfo.fPos = _float4{ 90.f,0.6f,103.f,1.f };
	MeshInfo.fScale = _float3{ 1.f,1.f,1.f };
	MeshInfo.fAngle = _float3{ 1.f,1.f,1.f };
	GI->Add_GameObjectToLayer(L"Theo", LEVEL_STAGE1, L"Monster", &MeshInfo);

	//CAnimMesh::MESHINFO MeshInfo;
	//MeshInfo.sTag = L"Golem";
	//MeshInfo.fPos = _float4{ 90.f,0.6f,103.f,1.f };
	//MeshInfo.fScale = _float3{ 1.f,1.f,1.f };
	//MeshInfo.fAngle = _float3{ 1.f,1.f,1.f };
	//GI->Add_GameObjectToLayer(L"Golem", LEVEL_STAGE1, L"Monster", &MeshInfo);

	//GI->StopAll();
	//GI->PlayBGM(L"Stage1.ogg", 0.6f);

	m_bScene = false;

	return S_OK;
}

void CLevel_Stage1::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	

	/*if (!m_bScene)
	{
		CRM->Start_Scene("Scene_Stage1");
		m_bScene = true;
	}*/
	if (GI->Key_Down(DIK_L))
	{
		GI->StopAll();
		GI->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_STAGE4));
	}

	if (GI->Key_Down(DIK_K))
	{
		GI->StopAll();
		GI->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_STAGE3));
	}

	if (GI->Key_Down(DIK_J))
	{
		GI->StopAll();
		GI->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_STAGE2));
	}
}

HRESULT CLevel_Stage1::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Stage1::Ready_Lights()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	LIGHTDESC			LightDesc;
	ZeroMemory(&LightDesc, sizeof(LIGHTDESC));

	LightDesc.eType = LIGHTDESC::TYPE_DIRECTIONAL;
	LightDesc.bRender = true;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(0.3f, 0.3f, 0.3f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(pGameInstance->Add_Light(m_pDevice, m_pContext, LightDesc)))
		return E_FAIL;

	ZeroMemory(&LightDesc, sizeof(LIGHTDESC));
	LightDesc.eType = LIGHTDESC::TYPE_POINT;
	LightDesc.bRender = false;
	LightDesc.vPosition = _float4(90.0f, 5.0f, 103.f, 1.f);
	LightDesc.fRange = 20.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = LightDesc.vDiffuse;

	if (FAILED(pGameInstance->Add_StaticLight(m_pDevice, m_pContext, LightDesc)))
		return E_FAIL;



	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

HRESULT CLevel_Stage1::Ready_Layer_Camera(const _tchar * pLayerTag)
{
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	CCamera::CAMERADESC			CameraDesc;

	CameraDesc.vEye = _float4(0.f, 10.f, -10.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fFovy = XMConvertToRadians(60.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.2f;
	CameraDesc.fFar = 300.0f;

	CameraDesc.TransformDesc.fSpeedPerSec = 25.f;
	CameraDesc.TransformDesc.fRotationPerSec = XMConvertToRadians(90.0f);


	/*if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Camera_Free"), LEVEL_STATIC, pLayerTag, &CameraDesc)))
		return E_FAIL;*/

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Camera_Player"), LEVEL_STATIC, pLayerTag, &CameraDesc)))
		return E_FAIL;



	Safe_Release(pGameInstance);

	return S_OK;
}


HRESULT CLevel_Stage1::Ready_Layer_BackGround(const _tchar * pLayerTag)
{
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Terrain"), LEVEL_STAGE1, pLayerTag)))
		return E_FAIL;
	Safe_Release(pGameInstance);

	return S_OK;
}


HRESULT CLevel_Stage1::Ready_Load_AnimModel(char * DatName)
{

	string FileSave = DatName;

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

HRESULT CLevel_Stage1::Ready_UI(char * DatName)
{
	string FileSave = DatName;

	string temp = "../Data/UIData/";

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

	while (true)
	{
		_tchar temp[256];
		_tchar temp2[256];
		ReadFile(hFile, temp, sizeof(_tchar) * 256, &dwByte, nullptr);
		ReadFile(hFile, temp2, sizeof(_tchar) * 256, &dwByte, nullptr);
		_tchar* UIPath = new _tchar[256];
		_tchar* UIName = new _tchar[256];
		for (int i = 0; i < 256; ++i)
		{
			UIPath[i] = temp[i];
			UIName[i] = temp2[i];
		}
		int UITexNum;
		int UIIndex;
		_float UIPosX;
		_float UIPosY;
		_float UISizeX;
		_float UISizeY;
		ReadFile(hFile, &UITexNum, sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &UIIndex, sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &UIPosX, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UIPosY, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UISizeX, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UISizeY, sizeof(_float), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete(UIPath);
			Safe_Delete(UIName);
			break;
		}

		CUI::UIINFO* UIInfo;
		UIInfo = new CUI::UIINFO;
		UIInfo->eLevel = LEVEL_STATIC;
		UIInfo->TexPath = UIPath;
		UIInfo->TexName = UIName;
		UIInfo->TexNum = UITexNum;
		UIInfo->UIIndex = UIIndex;
		UIInfo->UIPosX = UIPosX;
		UIInfo->UIPosY = UIPosY;
		UIInfo->UISizeX = UISizeX;
		UIInfo->UISizeY = UISizeY;

		if (FAILED(GI->Add_GameObjectToLayer(UIName, LEVEL_STATIC, L"Layer_UI", &UIInfo)))
		{
			wstring a = L"Please Load ProtoType";
			wstring b = a + UIName;
			const _tchar* c = b.c_str();
			MSG_BOX(c);

			Safe_Delete_Array(UIPath);
			Safe_Delete_Array(UIName);
			Safe_Delete(UIInfo);
			return E_FAIL;
		}

		Safe_Delete(UIInfo);
		Safe_Delete(UIPath);
		Safe_Delete(UIName);
	}

	CloseHandle(hFile);

	return S_OK;
}



HRESULT CLevel_Stage1::Ready_Load_Model(char * DatName)
{
	string FileSave = DatName;

	string temp = "../Data/ModelInstanceObject/";

	FileSave = temp  + FileSave + ".dat";

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
		_tchar* ModelName = new _tchar[256];
		ReadFile(hFile, ModelName, sizeof(_tchar) * 256, &dwByte, nullptr);		

		CMesh::MESHINFO* MeshInfo;
		MeshInfo = new CMesh::MESHINFO;
		MeshInfo->sTag = ModelName;	

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete_Array(ModelName);
			Safe_Delete(MeshInfo);
			break;
		}
		if (FAILED(GI->Add_GameObjectToLayer(ModelName, LEVEL_STAGE1, L"Layer_ModelObject", MeshInfo)))
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
	// 3. 파일 소멸
	CloseHandle(hFile);
	
	return S_OK;
}


CLevel_Stage1 * CLevel_Stage1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Stage1*		pInstance = new CLevel_Stage1(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevel_GamePlay"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Stage1::Free()
{
	__super::Free();

}