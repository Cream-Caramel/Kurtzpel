#pragma once
#include "Client_Defines.h"
#include "AnimMesh.h"
#include "Mesh.h"
BEGIN(Engine)
class CHierarchyNode;
class COBB;
class CNavigation;
END

BEGIN(Client)
class CPlayer;
class CGolem final : public CAnimMesh
{
	enum STATE {
		DOWN,
		DIE,
		RTDOWN,
		RUN,
		SKILL1,
		SKILL2,
		SKILL3,
		SKILL4_1, SKILL4_2, SKILL4_3,
		SKILL5_1, SKILL5_2, SKILL5_3,
		SKILL8,
		SKILL9,
		SKILL10_1, SKILL10_2, SKILL10_3,
		STANDUP,
		START,
		IDLE,
		STATE_END};

	enum OBB {OBB_BODY, OBB_LHAND, OBB_RHAND, OBB_ATTACK, OBB_END};
	enum SOCKET {SOCKET_LHAND, SOCKET_RHAND, SOCKET_END};
private:
	CGolem(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CGolem(const CGolem& rhs);
	virtual ~CGolem() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_ShadowDepth() override;
	virtual void Collision(CGameObject* pOther, string sTag)override;
	HRESULT Ready_Collider();
	void Set_NextMotion();
	void Set_NextAttack();
	void Range_Attack();
	void Close_Attack();
	_bool Get_RockOn() { return m_bRockOn; }
	void CreateSkillRock2();

private:
	void Set_State(STATE eState); // 상태를 설정 보간을 하는 애니메이션은 여기서 애니메이션 셋팅
	void Set_Dir(); // 방향을 설정
	void End_Animation(); // 애니메이션이 끝났을 때 작업을 수행 보간을 안할 애니메이션은 여기서 애니메이션 셋팅
	void Update(_float fTimeDelta); // 현재 상태에 따라 작업수행
	HRESULT Ready_Sockets();
	HRESULT Load_UI(char* DatName);
	void DebugKeyInput();
	void CreateLight();
	
private:
	STATE m_eCurState; // 현재 상태
	STATE m_eNextState; // 바꿔야할 상태
	_float3 m_vTargetLook; // 바라봐야할 방향
	_float m_fRushSpeed = 0.f;
	_float m_fRunSpeed = 3.f;
	_float m_fRunTempo = 0.f;
	_float m_fRunTempoAcc = 0.f;
	_float m_fGolemPattern = 0.f;
	_float m_fDissolveAcc = 0.f;
private:
	_bool m_bFinishCharge = false;
	_bool m_bFinishTime = false;
	_bool m_bFinishStart = false;
	_bool m_bLHand = false;
	_bool m_bRHand = false;
	_bool m_bPattern = false;
	_bool m_bAttack = false;
	_bool m_bRockOn = false;
	_bool m_bDissolve = false;
	_uint m_iPreMotion = 0;
	_uint m_iNextMotion = 1;
	_uint m_iPreRangeAttack = 0;
	_uint m_iNextRangeAttack = 1;
	_uint m_iPreCloseAttack = 0;
	_uint m_iNextCloseAttack = 1;
	CTexture* m_pDissolveTexture = nullptr;
	CNavigation* m_pNavigation = nullptr;
	CAnimModel* m_pAnimModel = nullptr;
	COBB* m_pOBB[OBB_END]{ nullptr };
	CPlayer* m_pTarget = nullptr;
	vector<class CHierarchyNode*>		m_Sockets;
	

public:
	static CGolem* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
