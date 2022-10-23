#pragma once

#include "Client_Defines.h"
#include "Camera.h"


BEGIN(Client)
class CPlayer;

class CCamera_Player final : public CCamera
{
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
	
private:
	_bool m_bShake = false;
	_vector m_vStartLook = { 0.f,0.f,0.f };
	_uint m_iPosInfoIndex = 0; // ¾ÀÄ«¸Þ¶óÆ÷½ºÀÎµ¦½º
	_uint m_iLookInfoIndex = 0; //¾À·èÆ÷½ºÀÎµ¦½º
	_bool m_bPosPlay = false;
	_bool m_bLookPlay = false;
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