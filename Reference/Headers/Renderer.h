#pragma once

#include "Component.h"

/* ȭ�鿡 �׷������� ��ü���� �׷������� ������� ��Ƽ� �����Ѵ�.  */
/* �����ϰ� �ִ� ���b���� �����Լ��� ȣ�����ֳ�. */

BEGIN(Engine)

class ENGINE_DLL CRenderer final : public CComponent
{
public: /* �׷����� ������ ���� ���� */
	enum RENDERGROUP { RENDER_PRIORITY, RENDER_SHADOW, RENDER_NONALPHABLEND, RENDER_NONLIGHT, RENDER_FRONTALPHA, RENDER_ALPHABLEND, RENDER_UI, RENDER_END };


private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRenderer(const CRenderer& rhs);
	virtual ~CRenderer() = default;



public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
public:
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pGameObject);	
	HRESULT Draw();
	void Set_bRenderTarget() { m_bRenderTarget = !m_bRenderTarget; }
#ifdef _DEBUG
public:
	HRESULT Add_DebugGroup(class CComponent* pDebugCom);
#endif // _DEBUG

private:
	list<class CGameObject*>				m_RenderObjects[RENDER_END];
	typedef list<class CGameObject*>		RENDEROBJECTS;

#ifdef _DEBUG
private:
	list<class CComponent*>					m_DebugObject;
#endif // _DEBUG


private:
	class CTarget_Manager*					m_pTarget_Manager = nullptr;
	class CLight_Manager*					m_pLight_Manager = nullptr;
	_bool m_bRenderTarget = false;
#ifdef _DEBUG
private:
	class CShader*							m_pShader = nullptr;
	class CVIBuffer_Rect*					m_pVIBuffer = nullptr;
	_float4x4								m_ViewMatrix, m_ProjMatrix;
#endif // _DEBUG

private:
	HRESULT Render_Priority();
	HRESULT Render_ShadowDepth();
	HRESULT Render_NonAlphaBlend();
	HRESULT Render_Lights();
	HRESULT Render_Blend();
	HRESULT Render_NonLight();
	HRESULT Render_FrontAlpha();
	HRESULT Render_AlphaBlend();
	HRESULT Render_UI();
	HRESULT Render_BlurDownSample();
	HRESULT Render_BlurHorizontal();
	HRESULT Render_BlurVertical();
	HRESULT Render_BlurUpSample();
	HRESULT Render_Grow();

#ifdef _DEBUG
	HRESULT Render_Debug();

#endif

private: /* For.Deferred Lighting */ 
	


public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END