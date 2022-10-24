#pragma once

#include "Client_Defines.h"
#include "Base.h"

/* 특정 레벨을 위한 리소스, 객체원형 등을 생성하는 역활을 한다. */
/* 서브스레드릉ㄹ 생성하여 위 작업을 수행하게끔 한다. */

BEGIN(Client)

class CLoader final : public CBase
{
public:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public:
	LEVEL Get_NextLevelID() const {
		return m_eNextLevelID;
	}

	const _tchar* Get_LoadingText() const {
		return m_szLoadingText;
	}

	_bool Get_Finished() const {
		return m_isFinished;
	}

	CRITICAL_SECTION Get_CS() const {
		return m_CriticalSection;
	}

public:
	HRESULT Initialize(LEVEL eNextLevelID);

#pragma region Public
	HRESULT Loading_ObjectProtoType();
	HRESULT Loading_Shader();
	HRESULT Loading_Component();
	HRESULT LoadInstance(const char* FileName);
	HRESULT Loading_ForLogoLevel();
	HRESULT LoadAnimModel(char* DatName);
	HRESULT LoadModel(char* DatName);
	HRESULT LoadTexture(char* DatName);

#pragma endregion Public

#pragma region Static
	HRESULT Loading_ForStatic();

	
#pragma endregion Static

	

#pragma region Stage1
	HRESULT Loading_ForStage1();
#pragma endregion Stage1
	
#pragma region Stage4
	HRESULT Loading_ForStage4();
#pragma endregion Stage4

private:
	ID3D11Device*			m_pDevice = nullptr;
	ID3D11DeviceContext*	m_pContext = nullptr;

private:
	HANDLE				m_hThread = 0;
	CRITICAL_SECTION	m_CriticalSection;
	LEVEL				m_eNextLevelID = LEVEL_END;
	map<const wstring, vector<_float3*>> m_InstanceInfo;

private:
	_tchar				m_szLoadingText[256] = TEXT("");
	_bool				m_isFinished = false;

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END