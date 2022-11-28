#pragma once

#include "Client_Defines.h"
#include "Camera.h"


BEGIN(Client)
class CPlayer;

class CCamera_Player final : public CCamera
{
public:
	enum FOVDIR {FOVUP, FOVDOWN, FOVEND};
private:
	CCamera_Player(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Player(const CCamera_Player& rhs, CTransform::TRANSFORMDESC* pArg);
	virtual ~CCamera_Player() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	_float3 Get_Pos();
	void Set_Pos(_float3 vPos);
	void LookAt(_float3 TargetPos);
	void PlayScene(_float fTimeDelta);
	void Set_ScenePosInfo(vector<POSINFO> PosInfos);
	void Set_SceneLookInfo(vector<LOOKINFO> LookInfos);
	void Start_Shake(_float fShakeTime, _float fShakePower, _float fShakeSpeed);
	void Shake(_float fTimeDelta);
	void End_Shake();
	void Start_Fov(_float fFov, _float fFovSpeed);
	void Fov(_float fTimeDelta);
	void FixFov(_float fFov, _float fFovSpeed);
	void End_Fov();
	void Set_FovSpeed(_float fFovSpeed) { m_fFovSpeed = fFovSpeed; }
	_float Get_Fov() { return m_fNowFOV; }
	void Stop_Scene();
	void Set_StopScene(_bool bStopScene) { m_bStopScene = bStopScene; }
private:
	_float m_fOriginFOV = 60.f;
	_float m_fTargetFOV = 0.f;
	_float m_fNowFOV = 60.f;
	_float m_fFovSpeed = 0.f;
	_float m_fPreTargetFOV = 0.f;
	FOVDIR m_eFovDir = FOVEND;
	_uint m_iFovCount = 0;
	_bool m_bShake = false;
	_bool m_bDir = false;
	_bool m_bStopScene = false;
	_float m_fShakeTime = 0.f;
	_float m_fShakePower = 0.f;
	_float m_fShakePowerAcc = 0.f;
	_float m_fShakeTimeAcc = 0.f;
	_float m_fShakeSpeed = 0.f;
	_float m_fShakeSpeedAcc = 0.f;
	_vector m_vRightPos = { 0.f,0.f,0.f,1.f };
	_vector m_vLeftPos = { 0.f,0.f,0.f,1.f };
	_float3 m_vOriginPos = { 0.f,0.f,0.f};
	_vector m_vStartLook = { 0.f,0.f,0.f };
	_uint m_iPosInfoIndex = 0; // ¾ÀÄ«¸Þ¶óÆ÷½ºÀÎµ¦½º
	_uint m_iLookInfoIndex = 0; //¾À·èÆ÷½ºÀÎµ¦½º
	_bool m_bPosPlay = false;
	_bool m_bLookPlay = false;
	_bool m_bPosStop = false;
	_bool m_bLookStop = false;
	_float m_fPosStopLimit = 0.f;
	_float m_fLookStopLimit = 0.f;
	CPlayer* m_pPlayer = nullptr;
	vector<POSINFO> m_PosInfo;
	vector<LOOKINFO> m_LookInfo;
	_float3 m_vDistance;
	CTransform* m_pLookTransform = nullptr;
public:
	static CCamera_Player* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END