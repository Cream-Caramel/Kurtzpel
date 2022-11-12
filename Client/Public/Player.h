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

class CPlayer final : public CAnimMesh
{
public:
	enum AnimModel {MODEL_PLAYER, MODEL_TOP, MODEL_BOTTOM, MODEL_END};
	enum Parts { PARTS_HEAD, PARTS_HAIRBACK, PARTS_HAIRFRONT, PARTS_HAIRSIDE, PARTS_HAIRTAIL, PARTS_SWORD, PARTS_END };
	enum SwordTrail {SWORDTRAIL_MAIN, SWORDTRAIL_SUB1, SWORDTRAIL_SUB2, SWORDTRAIL_END};
	enum Socket {SOCKET_HEAD, SOCKET_WEAPONHANDR, SOCKET_WEAPON_SPINE_R, SOCKET_END};
	enum DIR {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT, DIR_LU, DIR_RU, DIR_LD, DIR_RD, DIR_END};
	enum STATE {HITBACK, HITFRONT
		, JUMP, JUMPEND, JUMPUP, JUMPSTART
		, IDLE
		, DASH
		, DIE, RESPAWN
		, RUN, RUNEND
		, SPINCOMBOEND, SPINCOMBOLOOF, SPINCOMBOSTART
		, FASTCOMBOEND, FASTCOMBOSTART 
		, ROCKBREAK
		, CHARGECRASH, CHARGEREADY
		, AIRCOMBO1, AIRCOMBO2, AIRCOMBO3, AIRCOMBO4, AIRCOMBOEND
		, VOIDFRONTEND, VOIDBACKEND, VOIDFRONT, VOIDBACK
		, NOMALCOMBO1, NOMALCOMBO2, NOMALCOMBO3, NOMALCOMBO4, NOMALCOMBO5, NOMALCOMBO6
		, GROUNDCRASH, GROUNDREADY, GROUNDRUN
		, LEAPDOWN, LEAPUP, LEAPEND, LEAPREADY, LEAPRUN, LEAPSTART
		, BLADEATTACK
		, SLASHATTACK
		, ROCKSHOT
		, EX1ATTACK	, EX2ATTACK
		, EX1READY, EX2READY
		, STATE_END};
	enum OBB {OBB_BODY, OBB_END};

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
	virtual void Collision(CGameObject* pOther, string sTag)override;

#pragma region MainFunction
public:
	_vector Get_PlayerPos();
	HRESULT Ready_Collider();
	void Set_State(STATE eState); // ���¸� ���� ������ �ϴ� �ִϸ��̼��� ���⼭ �ִϸ��̼� ����
	void Set_Dir(DIR eDir); // ������ ����
	void End_Animation(); // �ִϸ��̼��� ������ �� �۾��� ���� ������ ���� �ִϸ��̼��� ���⼭ �ִϸ��̼� ����
	void Get_KeyInput(_float fTimeDelta); // ���� ���¿� ���� Ű�Է� �Լ� ȣ��
	void Update(_float fTimeDelta); // ���� ���¿� ���� �۾�����
	void Set_AniInfo(); // �ִϸ��̼� ���� ����ȭ
	void Set_PlayerUseInfo(); //�ٸ������� ����� �÷��̾��� ������ ������
	_bool Get_UseSkill() { return m_bUseSkill; } //��ų�� ����Ҽ��ִ»������� üũ
	STATE Get_State() { return m_eCurState; } // �ٸ������� ����ϱ����� �÷��̾� ���¸� �Ѱ���
	_float Get_NaviPosY();
	HRESULT Create_Navigation(char* FileName);
	void Set_bBattle(_bool bBattle) { m_bBattle = bBattle; }
	void Reset_BattleIndex();
	_bool Get_bJump();
	void Set_SwordTrailMatrix();
#pragma endregion MainFunction

#pragma region UtilFunction
	_float Get_PlayerHp() { return m_fNowHp; }
	void Set_PlayerHp(_float iHp) { m_fNowHp = iHp; }
	_float Get_PlayerMp() { return m_fNowMp; }
	void Set_PlayerMp(_float iMp) { m_fNowMp = iMp; }
	_bool Get_Respawn() { return m_bRespawn; }
#pragma endregion UtilFunction
	
#pragma region PrivateFunction
private:
	void Set_FastComboTime(_float fTimeDelta);
	void Hit_Shake();
	void Change_WeaponPos();
	void RockBreakVoice();
	void RastAttackVoice();
	void Ex1AttackLight();
	void ChargeAttackLight();
	void CreateGage();
#pragma region PrivateFunction

#pragma region KeyInput
private:
	void Check_Battle();
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
	void Ex1Attack_keyInput(_float fTimeDelta);
	void Ex1Ready_KeyInput(_float fTimeDelta);
	void Ex2Ready_KeyInput(_float fTimeDelta);

#pragma endregion KeyInput

#pragma region Variable
	_bool m_bAction = false; // �׼������� �ƴ��� �׼����� �ƴϸ� Į�� ��ġ�� �� �׼����̶�� Į�� ��ġ�� ������
	_bool m_bUseSkill = true; //��ų�� ����Ҽ��ִ��� ��� or �޸��� ���߿� ����
	_bool m_bBattle = false; //������ �ߴ��� ���ߴ���
	_bool m_bKeyInput = false; //����Ű�� ���ȴ���
	_bool m_bSpinComboEnd = false; // �����޺��� ��������
	_bool m_bRespawn = false; // ������������ 
	_bool m_bDoubleSlash = false; //EX��ų������
	_bool m_bDoubleSlashFov = false; //EX��ų Ȯ��������
	_bool m_bSpinCombo = false; // �����޺�������
	_bool m_bEx1CreateGage = false;
	_bool m_bChargeCreateGage = false;

	_float m_fJumpSpeed = 15.f; // ������ ���ǵ�
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
	_float m_fFastComboStartSpeed = 5.f; //FAST�޺����� �����ӵ�
	_float m_fFastComboEndSpeed = 8.f; // FAST�޺� ������ �����ӵ�
	_float m_fRockBreakSpeed = 6.f; // RockBreak �����ӵ�
	_float m_fChargeCrashSpeed = 6.f; //ChargeCrash �����ӵ�
	_float m_fSpinComboAcc = 0.f; //�����޺� ��Ʈ����
	_float m_fFastComboAcc = 0.f; //�н�Ʈ�޺� ��Ʈ����
	_float m_fRunSoundAcc = 0.f; //�޸��� ���� ����
	_float m_fRunSoundTempo = 0.3f; //�޸��� ���� ����
	_float m_fSpinComboLoofAcc = 0.f;
	_float m_fSpinComboLoofTempo = 0.4f;
	_float m_fEx1AttackSpeed = 0.f;
	_float m_fExReadyAcc = 0.f;
	
	
	
	_float m_fTurnSpeed = 0.f; //����Ʈ �Ͻ��ǵ�
	_float m_fRenderLimit = 0.f; //���� �ð�
	_float m_fMoveSpeed = 0.f;
	_float m_fMoveSpeedTempo = 0.f;
	_float3 m_vMoveDir = { 1.f,0.f,0.f };
	_matrix m_SwordTrailMatrix;
	CMesh::TURNDIR m_eTurnDir;

	
#pragma endregion Variable

private:
	STATE m_eCurState; // ���� ����
	STATE m_eNextState; // �ٲ���� ����
	DIR m_eDir = DIR_UP; // ���� ����
	_float3 m_vTargetLook; // �÷��̾ �ٶ������ ����
	_float3 m_vPlayerPos; // ���� �÷��̾��� ��ġ
	
private:
	CAnimModel* m_pAnimModel[MODEL_END]; // �÷��̾�, ����, ���� ��
	COBB* m_pOBB[OBB_END]{ nullptr };
	CNavigation* m_pNavigation = nullptr;

	vector<CMesh*>				m_Parts;
	vector<CMesh*>				m_SwordTrails;
	CMesh*						m_pSwordEx = nullptr;
	vector<class CHierarchyNode*>		m_Sockets;
	HRESULT Ready_Sockets();
	HRESULT Ready_PlayerParts();
	HRESULT Update_Parts();
	HRESULT Update_SwordTrails(STATE eState);
public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END
