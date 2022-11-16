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
class CTheo final : public CAnimMesh
{
	enum STATE {
		DOWN, HITEND, HITLOOF, HITSTART, RUN, SKILL1, SKILL2, SKILL3, SKILL4
		, SKILL5, SKILL6, APPEAR, IDLE, WALKBACK, WALK, FINISH, STATE_END};

	enum OBB {OBB_BODY, OBB_LHAND, OBB_RHAND,  OBB_END};
	enum SOCKET {SOCKET_LHAND, SOCKET_RHAND, SOCKET_END};
private:
	CTheo(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CTheo(const CTheo& rhs);
	virtual ~CTheo() = default;

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
	void DebugKeyInput();

private:
	void Set_State(STATE eState); // ���¸� ���� ������ �ϴ� �ִϸ��̼��� ���⼭ �ִϸ��̼� ����
	void Set_Dir(); // ������ ����
	void End_Animation(); // �ִϸ��̼��� ������ �� �۾��� ���� ������ ���� �ִϸ��̼��� ���⼭ �ִϸ��̼� ����
	void Update(_float fTimeDelta); // ���� ���¿� ���� �۾�����
	HRESULT Ready_Sockets();
	HRESULT Load_UI(char* DatName);
	
private:
	STATE m_eCurState; // ���� ����
	STATE m_eNextState; // �ٲ���� ����
	_float3 m_vTargetLook; // �÷��̾ �ٶ������ ����
	_float m_fWalkSpeed = 1.5f;
	_float m_fRunSpeed = 3.f;
	_float m_fRushSpeed = 12.f;
	_float m_fRushShakeAcc = 0.25f;
	_float m_fRushTempo = 0.5f;
	_float m_fFinishSpeed = 0.f;
private:
	_bool m_bFinishStart = false;
	_bool m_bLHand = false;
	_bool m_bRHand = false;
	_bool m_bPattern = false;
	_bool m_bRushRight = true;
	_uint m_iPreMotion = 0;
	_uint m_iNextMotion = 1;
	_uint m_iPreAttack = 0;
	_uint m_iNextAttack = 1;
	CAnimModel* m_pAnimModel = nullptr;
	COBB* m_pOBB[OBB_END]{ nullptr };
	CNavigation* m_pNavigation = nullptr;
	CPlayer* m_pTarget = nullptr;
	vector<class CHierarchyNode*>		m_Sockets;
	

public:
	static CTheo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
