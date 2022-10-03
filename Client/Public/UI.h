#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

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
private:
	CUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI(const CUI& rhs);
	virtual ~CUI() = default;

public:
	typedef struct UIInfo
	{
		const _tchar* TexPath;
		const _tchar* TexName;
		int TexNum;
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
	void Set_Position(float fX, float fY) { m_fX = fX, m_fY = fY; }
	void Set_Size(float fSizeX, float fSizeY) { m_fSizeX = fSizeX, m_fSizeY = fSizeY; }
	void Set_PositionX(float fX) { m_fX = fX; }
	void Set_PositionY(float fY) { m_fY = fY; }
	void Set_SizeX(float fSizeX) { m_fSizeX = fSizeX; }
	void Set_SizeY(float fSizeY) { m_fSizeY = fSizeY; }
	float Get_SizeX() { return m_fSizeX; }
	float Get_SizeY() { return m_fSizeY; }
	float Get_PositionX() { return m_fX; }
	float Get_PositionY() { return m_fY; }
	char* Get_UITag() { return m_UITag; }
	int Get_UITexNum() {return m_iTexNum; }
	bool Get_Select() { return m_bSelect; }
	void Set_Select(bool bSelect) { m_bSelect = bSelect; }
	UIInfo* Get_UiInfo() { return m_UIInfo; }
	ID3D11ShaderResourceView* Get_SRV(int Texnum);

private:
	const _tchar* TexPath;
	const _tchar* TexName;
	int m_iTexNum;
	float m_fSizeX;
	float m_fSizeY;
	float m_fX;
	float m_fY;
	float m_Cool = 0.f;
	float temp = 0.f;
	bool m_bSelect = false;
	UIInfo* m_UIInfo;
	char* m_UITag;

private:
	CShader*				m_pShaderCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CVIBuffer_Rect*			m_pVIBufferCom = nullptr;

private:
	HRESULT Ready_Components();

public:
	static CUI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END