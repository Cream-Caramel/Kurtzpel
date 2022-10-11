#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "GameInstance.h"
#include "UI_Manager.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CRenderer;
class CTransform;
class CVIBuffer_Rect;

END

BEGIN(Client)

class CUI : public CGameObject
{
protected:
	CUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI(const CUI& rhs);
	virtual ~CUI() = default;

public:
	typedef struct UIInfo
	{
		LEVEL eLevel;
		const _tchar* TexPath;
		const _tchar* TexName;
		int TexNum;
		int UIIndex = 0;
		float UIPosX = 400.f;
		float UIPosY = 300.f;
		float UISizeX = 100.f;
		float UISizeY = 100.f;
	}UIINFO;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void Set_PositionX(float fX) { m_fX = fX; }
	void Set_PositionY(float fY) { m_fY = fY; }
	void Set_SizeX(float fSizeX) { m_fSizeX = fSizeX; }
	void Set_SizeY(float fSizeY) { m_fSizeY = fSizeY; }
	float Get_SizeX() { return m_fSizeX; }
	float Get_SizeY() { return m_fSizeY; }
	float Get_PositionX() { return m_fX; }
	float Get_PositionY() { return m_fY; }
	UIInfo* Get_UiInfo() { return m_UIInfo; }

protected:
	int m_iUIIndex;
	float m_fSizeX;
	float m_fSizeY;
	float m_fX;
	float m_fY;
	UIInfo* m_UIInfo;

protected:
	CShader*				m_pShaderCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CVIBuffer_Rect*			m_pVIBufferCom = nullptr;

protected:
	HRESULT Ready_Components();

public:
	static CUI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END