#pragma once
#include "Client_Defines.h"
#include "AnimMesh.h"
#include "Mesh.h"
BEGIN(Engine)
class CHierarchyNode;
END

BEGIN(Client)

class CPlayer final : public CAnimMesh
{
public:
	enum AnimModel {MODEL_PLAYER, MODEL_TOP, MODEL_BOTTOM, MODEL_END};
	enum Parts { PARTS_HEAD, PARTS_HAIRBACK, PARTS_HAIRFRONT, PARTS_HAIRSIDE, PARTS_HAIRTAIL, PARTS_SWORD, PARTS_END };
	enum Socket {SOCKET_HEAD, SOCKET_WEAPONHANDR, SOCKET_END};
	enum DIR {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT, DIR_LU, DIR_RU, DIR_LD, DIR_RD};
	enum STATE {HITBACK, HITFRONT, JUMP, JUMPDOWN, JUMPEND, JUMPUP, JUMPSTART, IDLE, DASH, DIE, RESPAWN, RUN, RUNEND, SPINCOMBOEND, SPINCOMBOLOOF
		,SPINCOMBOSTART, FASTCOMBOEND, FASTCOMBOSTART, ROCKBREAK, CHARGECRASH, CHARGEREADY, AIRCOMBO1, AIRCOMBO2, AIRCOMBO3, AIRCOMBO4, AIRCOMBOEND
		,VOIDFRONTEND, VOIDBACKEND, VOIDFRONT, VOIDBACK, NOMALCOMBO1, NOMALCOMBO2, NOMALCOMBO3, NOMALCOMBO4, NOMALCOMBO5, NOMALCOMBO6, GROUNDCRASH
		,GROUNDREADY, GROUNDRUN, LEAPDOWN, LEAPUP, LEAPEND, LEAPREADY, LEAPRUN, LEAPSTART, BLADEATTACK, SLASHATTACK, ROCKSHOT, EX1ATTACK, EX2ATTACK
		,EX1READY, EX2READY, STATE_END};

private:
	CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayer(const CPlayer& rhs);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;


	void Set_State(STATE eState);
	void Set_Dir(DIR eDir);
	void End_Animation();
	void Get_KeyInput(_float fTimeDelta);
	void Input_Direction();
	void Update(_float fTimeDelta);
	void Set_AniInfo();
	
#pragma region KeyInput
private:
	void Jump_KeyInput(_float fTimeDelta);
	void JumpDown_KeyInput(_float fTimeDelta);
	void JumpUp_KeyInput(_float fTimeDelta);
#pragma endregion KeyInput

private:
	STATE m_AniIndex;
	STATE m_eCurState;
	DIR m_eDir = DIR_UP;
	_bool m_bJump = false;
	_float m_fJumpSpeed;
	_vector m_vTargetLook;
private:
	CAnimModel* m_pAnimModel[MODEL_END];

	vector<CMesh*>				m_Parts;
	vector<class CHierarchyNode*>		m_Sockets;
	HRESULT Ready_Sockets();
	HRESULT Ready_PlayerParts();
	HRESULT Update_Parts();

public:
	static CAnimMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
