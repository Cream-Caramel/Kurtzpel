#pragma once

#include "Client_Defines.h"
#include "Base.h"

BEGIN(Engine)
class CRenderer;
class CGameInstance;
END


BEGIN(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Tick(_float fTimeDelta);
	HRESULT Render();	

private:
	_bool						m_bScreen = false;
	CGameInstance*				m_pGameInstance = nullptr;
	CRenderer*					m_pRenderer = nullptr;
	RECT m_Clip;
	POINT m_p1;
	POINT m_p2;
private:
	ID3D11Device*				m_pDevice = nullptr;
	ID3D11DeviceContext*		m_pContext = nullptr;

#ifdef _DEBUG
private:
	_float			m_fTimeAcc = 0.f;
	_tchar			m_szFPS[256] = TEXT("");
	_uint			m_iNumDraw = 0;
#endif // _DEBUG

private:
	HRESULT Open_Level(LEVEL eLevelID);

private:  /* For.Static */
	HRESULT Ready_Prototype_Component();
	HRESULT Ready_Prototype_GameObject();


public:
	static CMainApp* Create();
	virtual void Free();
};


END