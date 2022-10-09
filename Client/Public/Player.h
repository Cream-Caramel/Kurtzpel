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
	enum DIR {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT, DIR_LU, DIR_RU, DIR_LD, DIR_RD, DIR_END};
	enum STATE {HITBACK, HITFRONT, JUMP, JUMPEND, JUMPUP, JUMPSTART, IDLE, DASH, DIE, RESPAWN, RUN, RUNEND, SPINCOMBOEND, SPINCOMBOLOOF
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

#pragma region MainFunction
public:
	_vector Get_PlayerPos();

	void Set_State(STATE eState); // ���¸� ���� ������ �ϴ� �ִϸ��̼��� ���⼭ �ִϸ��̼� ����
	void Set_Dir(DIR eDir); // ������ ����
	void End_Animation(); // �ִϸ��̼��� ������ �� �۾��� ���� ������ ���� �ִϸ��̼��� ���⼭ �ִϸ��̼� ����
	void Get_KeyInput(_float fTimeDelta); // ���� ���¿� ���� Ű�Է� �Լ� ȣ��
	void Update(_float fTimeDelta); // ���� ���¿� ���� �۾�����
	void Set_AniInfo(); // �ִϸ��̼� ���� ����ȭ
	void Set_PlayerUseInfo(); //�ٸ������� ����� �÷��̾��� ������ ������
#pragma endregion MainFunction

#pragma region UtilFunction
	void Jump(_float fTimeDelta); // �߷¿� ���� Y�� ���� ó�� ������ �� ���⿡ ���� XZ�� ����
	void JumpMove(_float fTimeDelta); // ������ �� ���⿡ ���� �̵��� ����
#pragma endregion UtilFunction
	
#pragma region KeyInput
private:
	bool Input_Direction(); // Ű�Է¿� ���� ���ⰻ��
	void Jump_KeyInput(_float fTimeDelta);
	void JumpUp_KeyInput(_float fTimeDelta);
	void JumpEnd_KeyInput(_float fTimeDelta);
	void Idle_KeyInput(_float fTimeDelta);
	void Dash_KeyInput(_float fTimeDelta);
	void Run_KeyInput(_float fTimeDelta);
	void RunEnd_KeyInput(_float fTimeDelta);
	void VoidFrontEnd_KeyInput(_float fTimeDelta);
	void VoidBackEnd_KeyInput(_float fTimeDelta);
	void SpinComboEnd_KeyInput(_float fTimeDelta);
	void SpinComboLoof_KeyInput(_float fTimeDelta);
	void SpinComboStart_KeyInput(_float fTimeDelta);
	void FastComboEnd_KeyInput(_float fTimeDelta);
	void FastComboStart_KeyInput(_float fTimeDelta);
	void RockBreak_KeyInput(_float fTimeDelta);
	void ChargeCrash_KeyInput(_float fTimeDelta);
	void ChargeReady_KeyInput(_float fTimeDelta);
	void AirCombo1_KeyInput(_float fTimeDelta);
	void AirCombo2_KeyInput(_float fTimeDelta);
	void AirCombo3_KeyInput(_float fTimeDelta);
	void AirComboEnd_KeyInput(_float fTimeDelta);
	void NomalCombo1_KeyInput(_float fTimeDelta);
	void NomalCombo2_KeyInput(_float fTimeDelta);
	void NomalCombo3_KeyInput(_float fTimeDelta);
	void NomalCombo4_KeyInput(_float fTimeDelta);
	void NomalCombo5_KeyInput(_float fTimeDelta);
	void NomalCombo6_KeyInput(_float fTimeDelta);
	void GroundReady_KeyInput(_float fTimeDelta);
	void GroundRun_KeyInput(_float fTimeDelta);
	void LeapReady_KeyInput(_float fTimeDelta);
	void LeapRun_KeyInput(_float fTimeDelta);
	void Ex1Ready_KeyInput(_float fTimeDelta);
	void Ex2Ready_KeyInput(_float fTimeDelta);
#pragma endregion KeyInput

#pragma region Variable
	_bool m_bJump = false; //����������
	_bool m_bKeyInput = false; //����Ű�� ���ȴ���
	_bool m_bSpinComboEnd = false; // �����޺��� ��������
	_float m_fGravity = 0.f; //�߷� 
	_float m_fJumpPower = 5.f; //�پ������ ��
	_float m_fJumpSpeed; // ������ ���ǵ�
	DIR m_eJumpDir = DIR_END; //�������۽� �ٶ�ô� ����
	_float m_fRunSpeed = 8.f; // �޸����̵����ǵ�
	_float m_fRunEndSpeed = 8.f; //�޸��⿣�彺�ǵ�
	_float m_fDashSpeed = 20.f; // �뽬�̵����ǵ�
	_float m_fNC1Speed = 5.f; //����1 �����ӵ�
	_float m_fNC2Speed = 5.f; //����1 �����ӵ�
	_float m_fNC3Speed = 6.f; //����1 �����ӵ�
	_float m_fNC4Speed = 6.f; //����1 �����ӵ�
	_float m_fNC5Speed = 5.f; //����1 �����ӵ�
	_float m_fNC6Speed = 8.f; //����1 �����ӵ�
	_float m_fCamDistanceX; // �÷��̾�� ī�޶��� X��Ÿ� ����
	_float m_fCamDistanceZ; // �÷��̾�� ī�޶��� Z��Ÿ� ����
	_float m_fVoidFront = 20.f; //�ձ����� ���ǵ�
	_float m_fVoidBack = 4.f; // ���� ���ǵ�
	_float m_fNomalCombo1Acc = 0.f; // NomalCombo1���� ��Ŭ���� �󸶵��� ���ȴ���
	_float m_fSpinComboSpeed = 2.f; // �����޺� �̵��ӵ�
	_float m_fSpinComboStartSpeed = 5.f; //�����޺����� �����ӵ�
	_float m_fSpinComboEndSpeed = 6.f; //�����޺������� �����ӵ�
	_float m_fFastComboStartSpeed = 5.f;
	_float m_fFastComboEndSpeed = 8.f;
	_float m_fRockBreakSpeed = 6.f;
	_float m_fChargeCrashSpeed = 6.f;
#pragma endregion Variable

private:
	STATE m_eCurState; // ���� ����
	STATE m_eNextState; // �ٲ���� ����
	DIR m_eDir = DIR_UP; // ���� ����
	_float3 m_vTargetLook; // �÷��̾ �ٶ������ ����
	_float3 m_vPlayerPos; // ���� �÷��̾��� ��ġ
	
private:
	CAnimModel* m_pAnimModel[MODEL_END]; // �÷��̾�, ����, ���� ��

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
