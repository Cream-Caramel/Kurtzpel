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
class CDragon final : public CAnimMesh
{
	enum STATE {
		BACKSTEP,
		DIE,
		GROGGYEND, GROGGYLOOF, GROGGYSTART,
		SKILL1,
		SKILL3,
		SKILL4,
		SKILL5,
		SKILL6,
		SKILL7_1, SKILL7_2, SKILL7_3,
		SKILL8,
		SKILL9_1, SKILL9_2, SKILL9_3,
		SKILL10_1, SKILL10_2, 
		SKILL13,
		SKILL14_1, SKILL14_2, SKILL14_3,
		SKILL15,
		START,
		IDLE,
		WALK,
		FINISH,
		STATE_END};
	enum TRAIL {TRAIL_1, TRAIL_2, TRAIL_END};
	enum TRAILSTATE {TS3_1, TS3_2, TS5, TS10, TS13, TS_END};
	enum OBB {OBB_BODY, OBB_LHAND, OBB_RHAND, OBB_ATTACK, OBB_END};
	enum SOCKET {SOCKET_LHAND, SOCKET_RHAND, SOCKET_HEAD, SOCKET_END};
private:
	CDragon(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CDragon(const CDragon& rhs);
	virtual ~CDragon() = default;

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

private:
	void Set_State(STATE eState); // 상태를 설정 보간을 하는 애니메이션은 여기서 애니메이션 셋팅
	void Set_Dir(); // 방향을 설정
	void End_Animation(); // 애니메이션이 끝났을 때 작업을 수행 보간을 안할 애니메이션은 여기서 애니메이션 셋팅
	void Update(_float fTimeDelta); // 현재 상태에 따라 작업수행
	HRESULT Ready_Sockets();
	HRESULT Load_UI(char* DatName);
	void DebugKeyInput();
	void CreateFire1();
	void CreateFire2();
	void CreateTrail(TRAILSTATE eTS);
	void CreateLight(_float4 WorldPos);
	void HitEffect();
private:
	STATE m_eCurState; // 현재 상태
	STATE m_eNextState; // 바꿔야할 상태
	_float3 m_vTargetLook; // 바라봐야할 방향
	_float3 m_vBackStepLook; 
	_float m_fRushSpeed = 12.f;
	_float m_fRunSpeed = 3.f;
	_float m_fBackStepSpeed = 15.f;
	_float m_fFlyAttackSpeed = 20.f;
	_float m_fWalkShakeAcc = 0.f;
	_float m_fWalkTempo = 1.3f;
	_float m_fRushShakeAcc = 0.f;
	_float m_fRushTempo = 0.65f;
	_float m_fSkill3Speed = 5.f;
	_float m_fSkill4Speed = 40.f;
	_float m_fFinishSpeed = 30.f;
private:
	_bool m_bFinishStart = false;
	_bool m_bFinishUpdate = false;
	_bool m_bLHand = false;
	_bool m_bRHand = false;
	_bool m_bPattern = false;
	_bool m_bAttack = false;
	_bool m_bRushRight = true;
	_bool m_bDissolve = false;
	_uint m_iPreMotion = 0;
	_uint m_iNextMotion = 1;
	_uint m_iPreRangeAttack = 0;
	_uint m_iNextRangeAttack = 1;
	_uint m_iPreCloseAttack = 0;
	_uint m_iNextCloseAttack = 1;
	_uint m_iWalkCount = 0;

	_float m_fTurnSpeed = 0.f; //이팩트 턴스피드
	_float m_fRenderLimit = 0.f; //랜더 시간
	_float m_fMoveSpeed = 0.f;
	_float m_fMoveSpeedTempo = 0.f;
	_float m_fDissolveAcc = 0.f;
	_float3 m_vMoveDir = { 1.f,0.f,0.f };
	_matrix m_TrailMatrix;
	CMesh::TURNDIR m_eTurnDir;
	CTexture* m_pDissolveTexture = nullptr;
	CNavigation* m_pNavigation = nullptr;
	CAnimModel* m_pAnimModel = nullptr;
	COBB* m_pOBB[OBB_END]{ nullptr };
	CPlayer* m_pTarget = nullptr;
	vector<class CHierarchyNode*>		m_Sockets;
	vector<CMesh*> m_Trails;

public:
	static CDragon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
