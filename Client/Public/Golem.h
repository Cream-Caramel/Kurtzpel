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
	virtual void Collision(CGameObject* pOther, string sTag)override;
	HRESULT Ready_Collider();
	void Set_NextMotion();
	void Set_NextAttack();
	void Range_Attack();
	void Close_Attack();

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
	_float3 m_vTargetLook; // �ٶ������ ����
	_float m_fRushSpeed = 0.f;
	_float m_fRunSpeed = 3.f;
private:
	_bool m_bLHand = false;
	_bool m_bRHand = false;
	_bool m_bPattern = false;
	_bool m_bAttack = false;
	
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