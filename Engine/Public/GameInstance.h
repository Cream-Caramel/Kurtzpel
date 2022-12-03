#pragma once

#include "Timer_Manager.h"
#include "Graphic_Device.h"
#include "Input_Device.h"
#include "Level_Manager.h"
#include "Object_Manager.h"
#include "Component_Manager.h"
#include "PipeLine.h"
#include <iostream>
#include "Light_Manager.h"
#include <random>
#include "Picking.h"
#include "Font_Manager.h"
#include "Target_Manager.h"
#include "Sound_Manager.h"

/* 클라이언트로 보여주기위한 가장 대표적인 클래스이다. */
/* 각종 매니져클래스들의 주요함수를 클라로 보여준다.  */
/* 엔진초기화. */
/* 엔진정리. */

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)

private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public: /* For.Engine */
	HRESULT Initialize_Engine(_uint iNumLevel, HINSTANCE hInst, const GRAPHICDESC& GraphicDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	void Tick_Engine(_float fTimeDelta);
	void Clear(_uint iLevelIndex);
	

public: /* For.Graphic_Device */
	HRESULT Clear_BackBuffer_View(_float4 vClearColor);
	HRESULT Clear_DepthStencil_View();
	HRESULT Present();
	ID3D11DepthStencilView* Get_ShadowDepthStencil();


public: /* For.Level_Manager */				
	HRESULT Open_Level(_uint iLevelIndex, class CLevel* pNewLevel);
	HRESULT Render_Level();

public: /* For.Object_Manager */
	HRESULT Add_Prototype(const _tchar* pPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_GameObjectToLayer(const _tchar* pPrototypeTag, _uint iLevelIndex, const _tchar* pLayerTag, void* pArg = nullptr);
	class CGameObject* Clone_GameObject(const _tchar* pPrototypeTag, void* pArg = nullptr);
public: /*For.Component_Manager*/
	HRESULT Add_Prototype(_uint iLevelIndex, const _tchar* pPrototypeTag, class CComponent* pPrototype);
	class CComponent* Clone_Component(_uint iLevelIndex, const _tchar* pPrototypeTag, void* pArg = nullptr);

public: /* for.Timer_Manager */
	_float Get_TimeDelta(const _tchar* pTimerTag);
	HRESULT Add_Timer(const _tchar* pTimerTag);
	HRESULT Update_Timer(const _tchar* pTimerTag);	
	void Set_Speed(const _tchar* pTimerTag, _float fSpeed);

public: /* For.Input_Device */
	_long Get_DIMMoveState(DIMM eMouseMoveID);
	_bool Key_Down(_uchar eKeyID);
	_bool Key_Up(_uchar eKeyID);
	_bool Key_Pressing(_uchar eKeyID);
	_bool Mouse_Down(DIMK eMouseKeyID);
	_bool Mouse_Up(DIMK eMouseKeyID);
	_bool Mouse_Pressing(DIMK eMouseKeyID);

public:
	_uint Get_Random(_uint iStart, _uint iEnd);
	_float Get_FloatRandom(_float iStart, _float iEnd);

public: /* For.PipeLine */
	void Set_Transform(CPipeLine::TRANSFORMSTATE eTransformState, _fmatrix TransformMatrix);
	_matrix Get_TransformMatrix(CPipeLine::TRANSFORMSTATE eTransformState) const;		
	_float4x4 Get_TransformFloat4x4(CPipeLine::TRANSFORMSTATE eTransformState) const;
	_float4x4 Get_TransformFloat4x4_Inverse(CPipeLine::TRANSFORMSTATE eTransformState) const;
	_float4x4 Get_TransformFloat4x4_TP(CPipeLine::TRANSFORMSTATE eTransformState) const;
	_float4 Get_CamPosition();
	_float3 Get_CamDir(CPipeLine::DIRSTATE eState);

public: /* For.Light_Manager */
	const LIGHTDESC* Get_LightDesc(_uint iIndex);
	const LIGHTDESC* Get_StaticLightDesc(_uint iIndex);
	HRESULT Set_StaticLight(_float fTimeLimit, _float fRange, _float4 vPos, _uint iIndex);
	HRESULT Add_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const LIGHTDESC& LightDesc);
	HRESULT Add_StaticLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const LIGHTDESC& LightDesc);
	HRESULT Reset_Lights();
	_matrix Get_LightMatrix();
	void Set_LightMatrix(_matrix Matrix);

public: /* For.Font_Manager */
	HRESULT Add_Fonts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFontTag, const _tchar* pFontFilePath);
	HRESULT Render_Fonts(const _tchar* pFontTag, const _tchar* pTextm, _float2 vPosition, _fvector vColor = XMVectorSet(0.f, 1.f, 0.f, 1.f),
		_float fAngle = 0.f, _float2 vOrigin = _float2(0.f, 0.f), _float2 vScale = _float2(1.f, 1.f));

public:/*For. Sound_Manager*/
	HRESULT PlaySound(TCHAR* pSoundKey, const _uint& eID, const float& fVolume);
	HRESULT PlayBGM(TCHAR * pSoundKey, const float& fVolume);
	HRESULT StopSound(const _uint& eID);
	HRESULT StopAll();
	int  VolumeUp(const _uint& eID, const _float& _vol);
	int  VolumeDown(const _uint& eID, const _float& _vol);
	

private:
	CGraphic_Device*				m_pGraphic_Device = nullptr;
	CInput_Device*					m_pInput_Device = nullptr;
	CSound_Manager*					m_pSound_Manager = nullptr;
	CLevel_Manager*					m_pLevel_Manager = nullptr;
	CObject_Manager*				m_pObject_Manager = nullptr;
	CComponent_Manager*				m_pComponent_Manager = nullptr;
	CTimer_Manager*					m_pTimer_Manager = nullptr;
	CPipeLine*						m_pPipeLine = nullptr;
	CLight_Manager*					m_pLight_Manager = nullptr;
	CPicking*						m_pPicking = nullptr;
	CFont_Manager*					m_pFont_Manager = nullptr;
	CTarget_Manager*				m_pTarget_Manager = nullptr;

public:
	static void Release_Engine();
	virtual void Free() override;
};

END