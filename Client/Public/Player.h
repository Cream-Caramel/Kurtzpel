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


public:
	_vector Get_PlayerPos();

	void Set_State(STATE eState); // 상태를 설정
	void Set_Dir(DIR eDir); // 방향을 설정
	void End_Animation(); // 애니메이션이 끝났을 때 작업을 수행
	void Get_KeyInput(_float fTimeDelta); // 현재 상태에 따라 키입력 함수 호출
	bool Input_Direction(); // 키입력에 따라 방향갱신
	void Update(_float fTimeDelta); // 현재 상태에 따라 작업수행
	void Set_AniInfo(); // 애니메이션 정보 동기화
	void Set_PlayerUseInfo(); //다른곳에서 사용할 플레이어의 정보를 갱신함
	
#pragma region KeyInput
private:
	void Jump_KeyInput(_float fTimeDelta);
	void JumpDown_KeyInput(_float fTimeDelta);
	void JumpUp_KeyInput(_float fTimeDelta);
	void Idle_KeyInput(_float fTimeDelta);
	void Run_KeyInput(_float fTimeDelta);
	void RunEnd_KeyInput(_float fTimeDelta);
	void SpinComboLoof_KeyInput(_float fTimeDelta);
	void FastComboStart_KeyInput(_float fTimeDelta);
	void ChargeReady_KeyInput(_float fTimeDelta);
	void AirCombo1_KeyInput(_float fTimeDelta);
	void AirCombo2_KeyInput(_float fTimeDelta);
	void AirCombo3_KeyInput(_float fTimeDelta);
	void NomalCombo1_KeyInput(_float fTimeDelta);
	void NomalCombo2_KeyInput(_float fTimeDelta);
	void NomalCombo3_KeyInput(_float fTimeDelta);
	void NomalCombo4_KeyInput(_float fTimeDelta);
	void NomalCombo5_KeyInput(_float fTimeDelta);
	void GroundReady_KeyInput(_float fTimeDelta);
	void GroundRun_KeyInput(_float fTimeDelta);
	void LeapReady_KeyInput(_float fTimeDelta);
	void LeapRun_KeyInput(_float fTimeDelta);
	void Ex1Ready_KeyInput(_float fTimeDelta);
	void Ex2Ready_KeyInput(_float fTimeDelta);
#pragma endregion KeyInput

#pragma region Variable
	_bool m_bJump = false; //점프중인지
	_bool m_bKeyInput = false; //방향키가 눌렸는지
	_float m_fRunSpeed = 8.f; // 달리기이동스피드
	_float m_fDashSpeed = 20.f; // 대쉬이동스피드
	_float m_fNC1Speed = 5.f; //공격1 전진속도
	_float m_fNC2Speed = 5.f; //공격1 전진속도
	_float m_fNC3Speed = 6.f; //공격1 전진속도
	_float m_fNC4Speed = 6.f; //공격1 전진속도
	_float m_fNC5Speed = 5.f; //공격1 전진속도
	_float m_fNC6Speed = 8.f; //공격1 전진속도
	_float m_fCamDistanceX; // 플레이어와 카메라의 X축거리 차이
	_float m_fCamDistanceZ; // 플레이어와 카메라의 Z축거리 차이
#pragma endregion Variable

private:
	STATE m_eCurState; // 현재 상태
	STATE m_eNextState; // 바꿔야할 상태
	DIR m_eDir = DIR_UP; // 현재 방향
	_float m_fJumpSpeed; // 점프 스피드
	_float3 m_vTargetLook; // 플레이어가 바라봐야할 방향
	_float3 m_vPlayerPos; // 현재 플레이어의 위치
	
private:
	CAnimModel* m_pAnimModel[MODEL_END]; // 플레이어, 상의, 하의 모델

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
