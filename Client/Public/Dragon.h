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

	enum OBB {OBB_BODY, OBB_LHAND, OBB_RHAND, OBB_ATTACK, OBB_END};
	enum SOCKET {SOCKET_LHAND, SOCKET_RHAND, SOCKET_END};
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
	
private:
	STATE m_eCurState; // 현재 상태
	STATE m_eNextState; // 바꿔야할 상태
	_float3 m_vTargetLook; // 바라봐야할 방향
	_float m_fRushSpeed = 12.f;
	_float m_fRunSpeed = 3.f;
	_float m_fBackStepSpeed = 8.f;
	_float m_fFlyAttackSpeed = 20.f;
	_float m_fWalkShakeAcc = 0.f;
	_float m_fWalkTempo = 1.3f;
	_float m_fRushShakeAcc = 0.f;
	_float m_fRushTempo = 0.65f;
	_uint m_iWalkCount = 0;
	_float m_fSkill3Speed = 5.f;
	_float m_fSkill4Speed = 40.f;
private:
	_bool m_bFinish = false;
	_bool m_bLHand = false;
	_bool m_bRHand = false;
	_bool m_bPattern = false;
	_bool m_bAttack = false;
	_uint m_iPreMotion = 0;
	_uint m_iNextMotion = 1;
	_uint m_iPreRangeAttack = 0;
	_uint m_iNextRangeAttack = 1;
	_uint m_iPreCloseAttack = 0;
	_uint m_iNextCloseAttack = 1;

	CNavigation* m_pNavigation = nullptr;
	CAnimModel* m_pAnimModel = nullptr;
	COBB* m_pOBB[OBB_END]{ nullptr };
	CPlayer* m_pTarget = nullptr;
	vector<class CHierarchyNode*>		m_Sockets;
	

public:
	static CDragon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
