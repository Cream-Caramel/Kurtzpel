#include "stdafx.h"
#include "..\Public\Particle_Manager.h"
#include "AlphaParticle.h"
#include "GameInstance.h"
#include "Pointer_Manager.h"

IMPLEMENT_SINGLETON(CParticle_Manager)

CParticle_Manager::CParticle_Manager()
{
}

void CParticle_Manager::Initalize()
{
	LoadParticle();
}

void CParticle_Manager::LoadParticle()
{
	string FileSave = "../Data/ParticleData/ParticleData.dat";
	
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

	DWORD		dwByte = 0;

	int ParticleSize;
	ReadFile(hFile, &ParticleSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ParticleSize; ++i)
	{
		char Name[256];
		PARTICLECREATE ParticleCreateInfo;
		ReadFile(hFile, Name, sizeof(char) * 256, &dwByte, nullptr);
		ReadFile(hFile, &ParticleCreateInfo, sizeof(PARTICLECREATE), &dwByte, nullptr);

		

		_tchar* ParticleName = ChangeTotChar(Name);
		_tchar* TexPath = new _tchar[256];
		_tchar* TexName = new _tchar[256];
		ReadFile(hFile, TexPath, sizeof(_tchar) * 256, &dwByte, nullptr);
		ReadFile(hFile, TexName, sizeof(_tchar) * 256, &dwByte, nullptr);

		ParticleCreateInfo.TexPath = TexPath;
		ParticleCreateInfo.TexName = TexName;
		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			delete TexPath;
			delete TexName;
			delete ParticleName;
			break;		
		}

		m_ParticleCreateInfo.emplace(ParticleName, ParticleCreateInfo);

	}


	// 3. 파일 소멸
	CloseHandle(hFile);
}



void CParticle_Manager::CreateParticle(_tchar * ParticleName, _float4 vPos, _bool bFollow, _bool bLoof, _bool bCenter)
{
	PARTICLECREATE ParticleCreateInfo = SearchParticle(ParticleName);
	for (int i = 0; i < ParticleCreateInfo.iParticleNum; ++i)
	{
		m_vPos.x = GI->Get_FloatRandom(ParticleCreateInfo.vMinPos.x, ParticleCreateInfo.vMaxPos.x);
		m_vPos.y = GI->Get_FloatRandom(ParticleCreateInfo.vMinPos.y, ParticleCreateInfo.vMaxPos.y);
		m_vPos.z = GI->Get_FloatRandom(ParticleCreateInfo.vMinPos.z, ParticleCreateInfo.vMaxPos.z);
		m_vSize.x = GI->Get_FloatRandom(ParticleCreateInfo.vMinSize.x, ParticleCreateInfo.vMaxSize.x);
		m_vSize.y = GI->Get_FloatRandom(ParticleCreateInfo.vMinSize.y, ParticleCreateInfo.vMaxSize.y);
		m_fSpeed = GI->Get_FloatRandom(ParticleCreateInfo.fMinSpeed, ParticleCreateInfo.fMaxSpeed);
		m_fLifeTime = GI->Get_FloatRandom(ParticleCreateInfo.fMinLifeTime, ParticleCreateInfo.fMaxLifeTime);

		CAlphaParticle::PARTICLEINFO ParticleInfo;
		ParticleInfo.vPosition = m_vPos;
		ParticleInfo.vSize = m_vSize;
		ParticleInfo.fSpeed = m_fSpeed;
		ParticleInfo.fLifeTime = m_fLifeTime;
		ParticleInfo.fGravity = ParticleCreateInfo.fGravity;
		ParticleInfo.vDirection = ParticleCreateInfo.vDir;
		ParticleInfo.TexPath = ParticleCreateInfo.TexPath;
		ParticleInfo.TexName = ParticleCreateInfo.TexName;
		ParticleInfo.TexNum = ParticleCreateInfo.TexNums;
		ParticleInfo.fFrameSpeed = ParticleCreateInfo.fFrameSpeed;
		ParticleInfo.vWorldPos = vPos;
		ParticleInfo.bFollow = bFollow;
		ParticleInfo.bLoof = bLoof;
		ParticleInfo.bCenter = bCenter;
		GI->Add_GameObjectToLayer(L"AlphaParticle", PM->Get_NowLevel(), L"Layer_Particle", &ParticleInfo);

	}
	
}

CParticle_Manager::PARTICLECREATE CParticle_Manager::SearchParticle(_tchar * ParticleName)
{
	for (auto& iter : m_ParticleCreateInfo)
	{
		if (!lstrcmp(iter.first, ParticleName))
			return iter.second;
	}
	PARTICLECREATE Fail;
	Fail.TexName = L"Null";
	return Fail;
}


void CParticle_Manager::Free()
{
	for (auto& iter : m_ParticleCreateInfo)
	{
		delete iter.second.TexPath;
		delete iter.second.TexName;
		delete iter.first;
	}
	m_ParticleCreateInfo.clear();
}


