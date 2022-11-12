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
	void Set_State(STATE eState); // 상태를 설정 보간을 하는 애니메이션은 여기서 애니메이션 셋팅
	void Set_Dir(DIR eDir); // 방향을 설정
	void End_Animation(); // 애니메이션이 끝났을 때 작업을 수행 보간을 안할 애니메이션은 여기서 애니메이션 셋팅
	void Get_KeyInput(_float fTimeDelta); // 현재 상태에 따라 키입력 함수 호출
	void Update(_float fTimeDelta); // 현재 상태에 따라 작업수행
	void Set_AniInfo(); // 애니메이션 정보 동기화
	void Set_PlayerUseInfo(); //다른곳에서 사용할 플레이어의 정보를 갱신함
	_bool Get_UseSkill() { return m_bUseSkill; } //스킬을 사용할수있는상태인지 체크
	STATE Get_State() { return m_eCurState; } // 다른곳에서 사용하기위한 플레이어 상태를 넘겨줌
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
	bool Input_Direction(); // 키입력에 따라 방향갱신
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
	_bool m_bAction = false; // 액션중인지 아닌지 액션중이 아니면 칼의 위치는 등 액션중이라면 칼의 위치는 오른손
	_bool m_bUseSkill = true; //스킬을 사용할수있는지 대기 or 달리기 도중에 가능
	_bool m_bBattle = false; //전투를 했는지 안했는지
	_bool m_bKeyInput = false; //방향키가 눌렸는지
	_bool m_bSpinComboEnd = false; // 스핀콤보가 끝났는지
	_bool m_bRespawn = false; // 리스폰중인지 
	_bool m_bDoubleSlash = false; //EX스킬중인지
	_bool m_bDoubleSlashFov = false; //EX스킬 확대중인지
	_bool m_bSpinCombo = false; // 스핀콤보중인지
	_bool m_bEx1CreateGage = false;
	_bool m_bChargeCreateGage = false;

	_float m_fJumpSpeed = 15.f; // 점프중 스피드
	_float m_fRunSpeed = 8.f; // 달리기이동스피드
	_float m_fRunEndSpeed = 8.f; //달리기엔드스피드
	_float m_fDashSpeed = 20.f; // 대쉬이동스피드
	_float m_fNC1Speed = 5.f; //공격1 전진속도
	_float m_fNC2Speed = 5.f; //공격1 전진속도
	_float m_fNC3Speed = 6.f; //공격1 전진속도
	_float m_fNC4Speed = 6.f; //공격1 전진속도
	_float m_fNC5Speed = 5.f; //공격1 전진속도
	_float m_fNC6Speed = 8.f; //공격1 전진속도
	_float m_fCamDistanceX; // 플레이어와 카메라의 X축거리 차이
	_float m_fCamDistanceZ; // 플레이어와 카메라의 Z축거리 차이
	_float m_fVoidFront = 20.f; //앞구르기 스피드
	_float m_fVoidBack = 4.f; // 덤블링 스피드
	_float m_fNomalCombo1Acc = 0.f; // NomalCombo1에서 좌클릭이 얼마동안 눌렸는지
	_float m_fSpinComboSpeed = 2.f; // 스핀콤보 이동속도
	_float m_fSpinComboStartSpeed = 5.f; //스핀콤보시작 전진속도
	_float m_fSpinComboEndSpeed = 6.f; //스핀콤보마무리 전진속도
	_float m_fFastComboStartSpeed = 5.f; //FAST콤보시작 전진속도
	_float m_fFastComboEndSpeed = 8.f; // FAST콤보 마무리 전진속도
	_float m_fRockBreakSpeed = 6.f; // RockBreak 전진속도
	_float m_fChargeCrashSpeed = 6.f; //ChargeCrash 전진속도
	_float m_fSpinComboAcc = 0.f; //스핀콤보 히트간격
	_float m_fFastComboAcc = 0.f; //패스트콤보 히트간격
	_float m_fRunSoundAcc = 0.f; //달리기 사운드 누적
	_float m_fRunSoundTempo = 0.3f; //달리기 사운드 간격
	_float m_fSpinComboLoofAcc = 0.f;
	_float m_fSpinComboLoofTempo = 0.4f;
	_float m_fEx1AttackSpeed = 0.f;
	_float m_fExReadyAcc = 0.f;
	
	
	
	_float m_fTurnSpeed = 0.f; //이팩트 턴스피드
	_float m_fRenderLimit = 0.f; //랜더 시간
	_float m_fMoveSpeed = 0.f;
	_float m_fMoveSpeedTempo = 0.f;
	_float3 m_vMoveDir = { 1.f,0.f,0.f };
	_matrix m_SwordTrailMatrix;
	CMesh::TURNDIR m_eTurnDir;

	
#pragma endregion Variable

private:
	STATE m_eCurState; // 현재 상태
	STATE m_eNextState; // 바꿔야할 상태
	DIR m_eDir = DIR_UP; // 현재 방향
	_float3 m_vTargetLook; // 플레이어가 바라봐야할 방향
	_float3 m_vPlayerPos; // 현재 플레이어의 위치
	
private:
	CAnimModel* m_pAnimModel[MODEL_END]; // 플레이어, 상의, 하의 모델
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
