#include "stdafx.h"
#include "..\Public\Loader.h"

#include "LoaderInclude.h"

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
	case LEVEL_STAGE2:
		pLoader->Loading_ForStage2();
		break;
	case LEVEL_STAGE3:
		pLoader->Loading_ForStage3();
		break;
	case LEVEL_STAGE4:
		pLoader->Loading_ForStage4();
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

	Loading_ObjectProtoType();

	lstrcpy(m_szLoadingText, TEXT("텍스처 로딩중"));
	LoadTexture("Level_Static");
	LoadTexture("BossBar");

	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STAGE1, TEXT("Prototype_Component_Texture_Terrain"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Grass_%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_VIBuffer_Terrain */
	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Height.bmp")))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Trail"),
		CTrail::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("InstanceModel"),
		CInstanceModel::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("모델 로딩중 "));

	LoadAnimModel("Level_Static");
	LoadAnimModel("PlayerEffect");
	LoadAnimModel("StaticEffect");
	LoadAnimModel("GolemEffect");
	LoadModel("Level_Static");
	LoadModel("Level_Stage1");
	LoadModel("PlayerEffect");
	//LoadModel("Level_Stage2");
	//LoadModel("Level_Stage3");
	//LoadModel("Level_Stage4");
	
	LoadParticleTexture();
	PTM->Initalize();
	Loading_Shader();

	Loading_Component();

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다.  "));

	Safe_Release(pGameInstance);

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_ObjectProtoType()
{
	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	lstrcpy(m_szLoadingText, TEXT("객체원형 로딩중"));

	/* 개ㅑㄱ체원형 로드한다. */
	/* For.Prototype_GameObject_Terrain*/
	if (FAILED(pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("AlphaParticle"),
		CAlphaParticle::Create(m_pDevice, m_pContext))))
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

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerEx"),
		CPlayerEx::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerRockBreak"),
		CPlayerRockBreak::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Rock"),
		CRock::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerGage"),
		CPlayerGage::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerGage2"),
		CPlayerGage2::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerTrailMain"),
		CPlayerTrailMain::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerTrailSub1"),
		CPlayerTrailSub1::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerTrailSub2"),
		CPlayerTrailSub2::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("PlayerLight"),
		CPlayerLight::Create(m_pDevice, m_pContext))))
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

	if (FAILED(pGameInstance->Add_Prototype(TEXT("TRSky"),
		CTRSky::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("TRStair"),
		CTRStair::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("TRFloor"),
		CTRFloor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SideLiner"),
		CSideLiner::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Theo"),
		CTheo::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Golem"),
		CGolem::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("GolemRock1"),
		CGolemRock1::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("GolemRock2"),
		CGolemRock2::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("GolemRock3"),
		CGolemRock3::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Dragon"),
		CDragon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillFrame"),
		CSkillFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("ExSkillFrame"),
		CExSkillFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("ExGauge"),
		CExGauge::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("KeyE"),
		CKeyE::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("KeyF"),
		CKeyF::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("KeyR"),
		CKeyR::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("KeyQ"),
		CKeyQ::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("KeyShift"),
		CKeyShift::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("KeyTab"),
		CKeyTab::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillIconE"),
		CSkillIconE::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillIconQ"),
		CSkillIconQ::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillIconR"),
		CSkillIconR::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillIconF"),
		CSkillIconF::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillIconShift"),
		CSkillIconShift::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("SkillIconTab"),
		CSkillIconTab::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("HPBarFrame"),
		CHPBarFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("MPBarFrame"),
		CMPBarFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("MPBar"),
		CMPBar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("BarLine"),
		CBarLine::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("BossHPBarFrame"),
		CBossHPBarFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("BossMPBarFrame"),
		CBossMPBarFrame::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("BossMPBar"),
		CBossMPBar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("BossBarLine"),
		CBossBarLine::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_Prototype(TEXT("Count"),
		CCount::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	char* a = new char[256];
	char b[256] = "sad";
	a[0] = b[0];

	string c = a;

	Safe_Delete_Array(a);



	Safe_Release(pGameInstance);

	return S_OK;
}

HRESULT CLoader::Loading_Shader()
{
	/* For.Prototype_Component_Shader_Terrain */
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Terrain"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX_DECLARATION::Elements, VTXNORTEX_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_ColorCube*/
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Cube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cube.hlsl"), VTXCUBE_DECLARATION::Elements, VTXCUBE_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_Model */
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Model"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMODEL_DECLARATION::Elements, VTXMODEL_DECLARATION::iNumElements))))
		return E_FAIL;

	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_EffectModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_EffectModel.hlsl"), VTXMODEL_DECLARATION::Elements, VTXMODEL_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_AnimModel */
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_AnimModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel.hlsl"), VTXANIMMODEL_DECLARATION::Elements, VTXANIMMODEL_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_RectInstance*/
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_ModelInstance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModelInstance.hlsl"), MODELINSTANCE_DECLARATION::Elements, MODELINSTANCE_DECLARATION::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPoint*/
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPoint"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPoint.hlsl"), VTXPOINT_DECLARATION::Elements, VTXPOINT_DECLARATION::iNumElements))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading_Component()
{
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Collider_AABB"),
		CAABB::Create(m_pDevice, m_pContext, CCollider::TYPE_AABB))))
		return E_FAIL;

	/* For.Prototype_Component_Collider_OBB */
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		COBB::Create(m_pDevice, m_pContext, CCollider::TYPE_OBB))))
		return E_FAIL;

	/* For.Prototype_Component_Collider_Sphere */
	if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		CSphere::Create(m_pDevice, m_pContext, CCollider::TYPE_SPHERE))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::LoadInstance(const char * FileName)
{
	string FileSave = FileName;

	string temp = "../Data/ModelInstance/";

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

		return E_FAIL;
	}

	DWORD		dwByte = 0;
	

	while (true)
	{
		_tchar* ModelName = new _tchar[256];
		ReadFile(hFile, ModelName, sizeof(_tchar) * 256, &dwByte, nullptr);

		_float3 Right;
		ReadFile(hFile, &Right, sizeof(_float3), &dwByte, nullptr);
		_float3 Up;
		ReadFile(hFile, &Up, sizeof(_float3), &dwByte, nullptr);
		_float3 Look;
		ReadFile(hFile, &Look, sizeof(_float3), &dwByte, nullptr);
		_float3 Pos;
		ReadFile(hFile, &Pos, sizeof(_float3), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete(ModelName);
			break;
		}

		_float3* MatrixInfo = new _float3[4];
		MatrixInfo[0] = Right;
		MatrixInfo[1] = Up;
		MatrixInfo[2] = Look;
		MatrixInfo[3] = Pos;

		wstring Search = ModelName;
		auto& Result = m_InstanceInfo.find(Search);
		if (Result == m_InstanceInfo.end())
		{
			vector<_float3*> MatrixVector;
			MatrixVector.push_back(MatrixInfo);
			m_InstanceInfo.emplace(Search, MatrixVector);
		}
		else
			Result->second.push_back(MatrixInfo);


		Safe_Delete_Array(ModelName);
	}



	// 3. 파일 소멸
	CloseHandle(hFile);

	return S_OK;
}

HRESULT CLoader::Loading_ForLogoLevel()
{
	return S_OK;
}

HRESULT CLoader::Loading_ForStage1()
{
	if (FAILED(GI->Add_Prototype(LEVEL_STAGE1, TEXT("NavigationStage1"),
		CNavigation::Create(m_pDevice, m_pContext, "Level_Stage1"))))
		return E_FAIL;

	LoadInstance("Level_Stage1");

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_ForStage2()
{
	if (FAILED(GI->Add_Prototype(LEVEL_STAGE2, TEXT("NavigationStage2"),
		CNavigation::Create(m_pDevice, m_pContext, "Level_Stage2"))))
		return E_FAIL;

	LoadInstance("Level_Stage2");

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_ForStage3()
{
	if (FAILED(GI->Add_Prototype(LEVEL_STAGE3, TEXT("NavigationStage3"),
		CNavigation::Create(m_pDevice, m_pContext, "Level_Stage3"))))
		return E_FAIL;

	LoadInstance("Level_Stage3");

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_ForStage4()
{
	if (FAILED(GI->Add_Prototype(LEVEL_STAGE4, TEXT("NavigationStage4"),
		CNavigation::Create(m_pDevice, m_pContext, "Level_Stage4"))))
		return E_FAIL;

	LoadInstance("Level_Stage4");

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
		_matrix PivotMatrix = XMMatrixIdentity();
		PivotMatrix = XMMatrixRotationY(XMConvertToRadians(270.0f));
		if (FAILED(GI->Add_Prototype(LEVEL_STATIC, ProtoName,
			CAnimModel::Create(m_pDevice, m_pContext, ModelName, PivotMatrix))))
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
	if (strcmp(DatName, "Level_Static"))
		LoadInstance(DatName);
	
	DWORD		dwByte = 0;

	char* SavePath = new char[256];
	strcpy_s(SavePath, sizeof(char) * 256, DatName);

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

		if (!strcmp(DatName, "Level_Static"))
		{
			if (FAILED(GI->Add_Prototype(LEVEL_STATIC, ProtoName,
				CModel::Create(m_pDevice, m_pContext, ModelName, DatName))))
			{
				Safe_Delete(ProtoName);
				return E_FAIL;
			}
			RM->Pushtchar(ProtoName);
			Safe_Delete(ModelName);
		}

		else if (!strcmp(DatName, "Level_Stage1"))
		{
			wstring Name = ProtoName;
			auto& iter = m_InstanceInfo.find(Name);
			if (iter != m_InstanceInfo.end())
			{
				if (FAILED(GI->Add_Prototype(LEVEL_STAGE1, ProtoName,
					CModelsInstance::Create(m_pDevice, m_pContext, ModelName, DatName, iter->second))))
				{
					Safe_Delete(ProtoName);
					return E_FAIL;
				}
			}
			
			RM->Pushtchar(ProtoName);
			Safe_Delete(ModelName);
		}

		else if (!strcmp(DatName, "Level_Stage2"))
		{
			wstring Name = ProtoName;
			auto& iter = m_InstanceInfo.find(Name);
			if (iter != m_InstanceInfo.end())
			{
				if (FAILED(GI->Add_Prototype(LEVEL_STAGE2, ProtoName,
					CModelsInstance::Create(m_pDevice, m_pContext, ModelName, DatName, iter->second))))
				{
					Safe_Delete(ProtoName);
					return E_FAIL;
				}
			}

			RM->Pushtchar(ProtoName);
			Safe_Delete(ModelName);
		}

		else if (!strcmp(DatName, "Level_Stage3"))
		{
			wstring Name = ProtoName;
			auto& iter = m_InstanceInfo.find(Name);
			if (iter != m_InstanceInfo.end())
			{
				if (FAILED(GI->Add_Prototype(LEVEL_STAGE3, ProtoName,
					CModelsInstance::Create(m_pDevice, m_pContext, ModelName, DatName, iter->second))))
				{
					Safe_Delete(ProtoName);
					return E_FAIL;
				}
			}

			RM->Pushtchar(ProtoName);
			Safe_Delete(ModelName);
		}

		else if (!strcmp(DatName, "Level_Stage4"))
		{
			wstring Name = ProtoName;
			auto& iter = m_InstanceInfo.find(Name);
			if (iter != m_InstanceInfo.end())
			{
				if (FAILED(GI->Add_Prototype(LEVEL_STAGE4, ProtoName,
					CModelsInstance::Create(m_pDevice, m_pContext, ModelName, DatName, iter->second))))
				{
					Safe_Delete(ProtoName);
					return E_FAIL;
				}
			}

			RM->Pushtchar(ProtoName);
			Safe_Delete(ModelName);
		}

		else if (!strcmp(DatName, "PlayerEffect"))
		{
			if (FAILED(GI->Add_Prototype(LEVEL_STATIC, ProtoName,
				CModel::Create(m_pDevice, m_pContext, ModelName, DatName))))
			{
				Safe_Delete(ProtoName);
				return E_FAIL;
			}
			RM->Pushtchar(ProtoName);
			Safe_Delete(ModelName);
		}
		
	}

	delete SavePath;
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

HRESULT CLoader::LoadParticleTexture()
{
	string FileSave = "../Data/ParticleData/ParticleTexture.dat";

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
		_tchar Name[256];
		ReadFile(hFile, &Name, sizeof(_tchar) * 256, &dwByte, nullptr);
		int iTexNum;
		ReadFile(hFile, &iTexNum, sizeof(int), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
			break;

		_tchar* TexPath = new _tchar[256];
		_tchar* TexName = new _tchar[256];

		lstrcpy(TexPath, Path);
		lstrcpy(TexName, Name);

		if (FAILED(GI->Add_Prototype(LEVEL_STATIC, TexName,
			CTexture::Create(m_pDevice, m_pContext, TexPath, iTexNum))))
			MSG_BOX(TEXT("Fail To Load Texture"));

		RM->Pushtchar(TexPath);
		RM->Pushtchar(TexName);
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
	for (auto& iter : m_InstanceInfo)
	{
		iter.first;
		for (auto& Pointer : iter.second)
		{
			Safe_Delete(Pointer);
		}
	}
	m_InstanceInfo.clear();
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);


}
