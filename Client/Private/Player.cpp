#include "stdafx.h"
#include "..\Public\Player.h"
#include "GameInstance.h"
#include "HierarchyNode.h"
#include "Pointer_Manager.h"
#include "UI_Manager.h"
#include "Collider.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "Navigation.h"
#include "Camera_Manager.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	for (auto& iter : m_pAnimModel)
		iter = nullptr;
}

CPlayer::CPlayer(const CPlayer& rhs)
	:CAnimMesh(rhs)
{
}

HRESULT CPlayer::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);
	sTag = m_MeshInfo->sTag;
	
    if (FAILED(Ready_Collider()))
		return E_FAIL;
	
	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Player"), (CComponent**)&m_pAnimModel[MODEL_PLAYER])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerTop", TEXT("Top"), (CComponent**)&m_pAnimModel[MODEL_TOP])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerBottom", TEXT("Bottom"), (CComponent**)&m_pAnimModel[MODEL_BOTTOM])))
		return E_FAIL;
	
	/*if (FAILED(Create_Navigation("Level_Stage1")))
		return E_FAIL;*/
	
	m_eCurState = IDLE;
	m_eNextState = IDLE;
	m_vTargetLook = { 0.f,0.f,1.f};

	Set_AniInfo();

	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->Set_AnimIndex(m_eCurState);
	}
	
	m_fMaxHp = 100;
	m_fMaxMp = 100.f;
	m_fNowHp = m_fMaxHp;
	m_fNowMp = m_fMaxMp;
	m_fColiisionTime = 1.2f;
	PM->Add_Player(this);
	Ready_Sockets();
	Ready_PlayerParts();

	UM->Add_Player(this);
	m_bColliderRender = true;
	
	m_pTransformCom->Set_Gravity(0.f);
	m_pTransformCom->Set_JumpPower(0.6f);

	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (!m_pAnimModel[0]->GetChangeBool())
		m_eCurState = m_eNextState;

	if (GI->Key_Down(DIK_1))
		Set_State(DIE);

	if (GI->Key_Down(DIK_0))
		m_bColliderRender = !m_bColliderRender;

	if (GI->Key_Down(DIK_3))
		CRM->Start_Fov(90.f, 120.f);

	if (GI->Key_Down(DIK_4))
		CRM->Set_FovDir(true);

	if (GI->Key_Down(DIK_5))
		CRM->Start_Scene("Scene_Stage1");

	if (GI->Key_Down(DIK_8))
		m_pNavigation->Set_NaviRender();

	m_bKeyInput = false;
	m_bUseSkill = false;
	Get_KeyInput(fTimeDelta);

	

	if (!m_bCollision)
	{
		m_fCollisionAcc += 1.f * fTimeDelta;
		if (m_fCollisionAcc >= m_fColiisionTime)
		{
			m_fCollisionAcc = 0.f;
			m_bCollision = true;
		}
	}

	Update(fTimeDelta);

	if (m_pTransformCom->Get_Jump())
	{
		m_pTransformCom->Jump(fTimeDelta);
	}

	Update_Parts();
	for (auto& pPart : m_Parts)
		pPart->Tick(fTimeDelta);
}

void CPlayer::LateTick(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_U))
		UM->Set_ExGaugeTex(1);
	if(!m_bSpinCombo)
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMLoadFloat3(&m_vTargetLook), 0.3f);

	for(int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->Play_Animation(fTimeDelta, m_pAnimModel[i]);
	}

	for (auto& pPart : m_Parts)
		pPart->LateTick(fTimeDelta);

	End_Animation();

	for (auto& pPart : m_Parts)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, pPart);

	for (int i = 0; i < OBB_END; ++i)
		m_pOBB[i]->Update(m_pTransformCom->Get_WorldMatrix());

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	
	if(m_bCollision)
		CM->Add_OBBObject(CCollider_Manager::COLLIDER_PLAYER, this, m_pOBB[OBB_BODY]);
	
	Check_Battle();

	Set_PlayerUseInfo();
}

HRESULT CPlayer::Render()
{
	if 	(nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	for (int i = 0; i < MODEL_END; ++i)
	{
		if (m_pAnimModel[i] != nullptr)
		{
			_uint		iNumMeshes = m_pAnimModel[i]->Get_NumMeshes();
			for (_uint j = 0; j < iNumMeshes; ++j)
			{
				if (FAILED(m_pAnimModel[i]->SetUp_OnShader(m_pShaderCom, m_pAnimModel[i]->Get_MaterialIndex(j), TEX_DIFFUSE, "g_DiffuseTexture")))
					return E_FAIL;

				if (FAILED(m_pAnimModel[i]->Render(m_pShaderCom, j)))
					return E_FAIL;
			}
		}
	}

	for (_uint i = 0; i < OBB_END; ++i)
	{
		if (m_bColliderRender)
		{
			m_pOBB[i]->Render();
		}
	}

	m_pNavigation->Render();

	
	return S_OK;
}

void CPlayer::Collision(CGameObject * pOther, string sTag)
{
	if (sTag == "Monster_Attack")
	{
		if (m_bMotionChange)
		{
			int random = GI->Get_Random(1, 2);
			if (random == 1)
			{		
				Hit_Shake();
				for (int i = 0; i < MODEL_END; ++i)
				{
					m_pAnimModel[i]->Set_AnimIndex(HITBACK);
					m_eNextState = HITBACK;
				}
			}
			else
			{
				Hit_Shake();
				for (int i = 0; i < MODEL_END; ++i)
				{
					m_pAnimModel[i]->Set_AnimIndex(HITBACK);
					m_eNextState = HITFRONT;
				}
			}

			Minus_Hp(pOther->Get_Damage());
		}
		else
			Minus_Hp(pOther->Get_Damage());
		if (m_fNowHp <= 0.f)
		{
			m_fNowHp = 0.f;
			m_bDie = true;
			Set_State(DIE);
		}
		
		m_bCollision = false;

	}
}

_vector CPlayer::Get_PlayerPos()
{
	return m_pTransformCom->Get_State(CTransform::STATE_POSITION);
}

HRESULT CPlayer::Ready_Collider()
{
	CCollider::COLLIDERDESC		ColliderDesc;

	ColliderDesc.vSize = _float3(0.7f, 2.f, 0.7f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Player_Body";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Head"), (CComponent**)&m_pOBB[OBB_BODY], &ColliderDesc)))
		return E_FAIL;


	return S_OK;
}

void CPlayer::Set_State(STATE eState)
{
	if (m_eNextState == eState)
		return;

	m_eNextState = eState;

	switch (m_eNextState)
	{
	case Client::CPlayer::HITBACK:		
		break;
	case Client::CPlayer::HITFRONT:
		break;
	case Client::CPlayer::JUMP:		
		break;
	case Client::CPlayer::JUMPEND:
		CRM->Set_FovDir(true);
		break;
	case Client::CPlayer::JUMPUP:
		
		break;
	case Client::CPlayer::JUMPSTART:
		CRM->Start_Fov(80.f, 100.f);
		m_pTransformCom->Set_Jump(true);
		m_pTransformCom->Set_Gravity(0.f);
		m_pTransformCom->Set_JumpPower(0.6f);
		m_fJumpSpeed = 10.f;
		break;
	case Client::CPlayer::IDLE:
		CRM->Set_FovDir(true);
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::DASH:
		m_fNowMp -= 5.f;
		m_fDashSpeed = 20.f;
		CRM->Start_Fov(80.f, 100.f);
		CRM->Set_FovDir(true);
		break;
	case Client::CPlayer::DIE:
		CRM->Start_Fov(30.f, 30.f);
		break;
	case Client::CPlayer::RESPAWN:
		CRM->Set_FovSpeed(60.f);
		CRM->Set_FovDir(true);
		break;
	case Client::CPlayer::RUN:	
		m_fRunSpeed = 8.f;
		break;
	case Client::CPlayer::RUNEND:
		m_fRunEndSpeed = 8.f;
		break;
	case Client::CPlayer::SPINCOMBOEND:
		
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		
		m_fNowMp -= 15.f;
		m_Parts[PARTS_SWORD]->Set_Damage(2.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(999);
		break;
	case Client::CPlayer::FASTCOMBOEND:
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		CRM->Start_Fov(80.f, 100.f);
		m_fNowMp -= 10.f;
		m_fFastComboStartSpeed = 5.f;
		m_Parts[PARTS_SWORD]->Set_Damage(2.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(30);
		m_fFastComboAcc = 0.f;
		break;
	case Client::CPlayer::ROCKBREAK:
		m_fNowMp -= 5.f;
		m_fRockBreakSpeed = 6.f;
		m_Parts[PARTS_SWORD]->Set_Damage(1.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::CHARGECRASH:
		CRM->Set_FovSpeed(150.f);
		CRM->Set_FovDir(true);
		m_fChargeCrashSpeed = 6.f;
		m_Parts[PARTS_SWORD]->Set_Damage(100.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		
		break;
	case Client::CPlayer::CHARGEREADY:
		CRM->Start_Fov(100.f, 20.f);
		m_fNowMp -= 20.f;
		break;
	case Client::CPlayer::AIRCOMBO1:
		m_fNowMp -= 3.f;
		m_pTransformCom->Set_JumpPower(0.f);
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBO2:
		m_fNowMp -= 3.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBO3:
		m_fNowMp -= 3.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBO4:
		m_fNowMp -= 3.f;
		m_Parts[PARTS_SWORD]->Set_Damage(14.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBOEND:
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		CRM->Set_FovDir(true);
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::VOIDFRONTEND:
		
		break;
	case Client::CPlayer::VOIDFRONT:
		CRM->Start_Fov(40.f, 100.f);
		CRM->Set_FovDir(true);
		m_fNowMp -= 10.f;
		break;
	case Client::CPlayer::VOIDBACKEND:
		break;
	case Client::CPlayer::VOIDBACK:
		CRM->Start_Fov(40.f, 70.f);
		CRM->Set_FovDir(true);
		m_fNowMp -= 10.f;
		break;
	case Client::CPlayer::NOMALCOMBO1:
		m_fNC1Speed = 5.f;
		m_fNomalCombo1Acc = 0.f;
		m_fNowMp -= 2.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::NOMALCOMBO2:
		m_fNowMp -= 3.f;
		m_fNC2Speed = 5.f;
		m_Parts[PARTS_SWORD]->Set_Damage(7.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::NOMALCOMBO3:
		m_fNowMp -= 5.f;
		m_fNC3Speed = 6.f;
		m_Parts[PARTS_SWORD]->Set_Damage(8.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(2);
		break;
	case Client::CPlayer::NOMALCOMBO4:
		m_fNowMp -= 5.f;
		m_fNC4Speed = 6.f;
		m_Parts[PARTS_SWORD]->Set_Damage(10.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(2);
		break;
	case Client::CPlayer::NOMALCOMBO5:
		m_fNowMp -= 3.f;
		m_fNC5Speed = 5.f;
		m_Parts[PARTS_SWORD]->Set_Damage(10.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::NOMALCOMBO6:
		m_fNowMp -= 5.f;
		m_fNC6Speed = 8.f;
		m_Parts[PARTS_SWORD]->Set_Damage(20.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::GROUNDCRASH:
		
		break;
	case Client::CPlayer::GROUNDREADY:
		
		break;
	case Client::CPlayer::GROUNDRUN:
		
		break;
	case Client::CPlayer::LEAPDOWN:
	
		break;
	case Client::CPlayer::LEAPUP:
	
		break;
	case Client::CPlayer::LEAPEND:
	
		break;
	case Client::CPlayer::LEAPREADY:
	
		break;
	case Client::CPlayer::LEAPRUN:
	
		break;
	case Client::CPlayer::LEAPSTART:
		
		break;
	case Client::CPlayer::BLADEATTACK:
		m_fNowMp -= 20.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(10);
		break;
	case Client::CPlayer::SLASHATTACK:
		m_fNowMp -= 30.f;
		m_Parts[PARTS_SWORD]->Set_Damage(5.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(30);
		break;
	case Client::CPlayer::ROCKSHOT:
		
		break;
	case Client::CPlayer::EX1ATTACK:
		
		break;
	case Client::CPlayer::EX2ATTACK:
		
		break;
	case Client::CPlayer::EX1READY:
		
		break;
	case Client::CPlayer::EX2READY:	
		break;
	
	}
	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->SetNextIndex(m_eNextState);
		m_pAnimModel[i]->SetChangeBool(true);
	}
}

void CPlayer::Set_Dir(DIR eDir)
{
	m_eDir = eDir;

	_matrix View = XMLoadFloat4x4(&GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW));
	_vector vCamLook = View.r[2];

	_vector vCamRight = XMVector3Normalize(XMVector3Cross(_vector{ 0.f,1.f,0.f }, vCamLook));
	
	vCamLook = XMVector3Normalize(XMVector3Cross(vCamRight, _vector{ 0.f,1.f,0.f }));

	switch (m_eDir)
	{
	case Client::CPlayer::DIR_UP:
		XMStoreFloat3(&m_vTargetLook, (vCamLook));
		break;
	case Client::CPlayer::DIR_DOWN:
		XMStoreFloat3(&m_vTargetLook, (vCamLook * -1.f));
		break;
	case Client::CPlayer::DIR_RIGHT:
		XMStoreFloat3(&m_vTargetLook, (vCamRight));
		break;
	case Client::CPlayer::DIR_LEFT:
		XMStoreFloat3(&m_vTargetLook, (vCamRight * -1.f));
		break;
	case Client::CPlayer::DIR_LU:
		XMStoreFloat3(&m_vTargetLook, XMVector3Normalize(vCamLook - vCamRight));
		break;
	case Client::CPlayer::DIR_RU:
		XMStoreFloat3(&m_vTargetLook, XMVector3Normalize(vCamLook + vCamRight));
		break;
	case Client::CPlayer::DIR_LD:
		XMStoreFloat3(&m_vTargetLook, XMVector3Normalize(vCamLook * -1.f + vCamRight * -1.f));
		break;
	case Client::CPlayer::DIR_RD:
		XMStoreFloat3(&m_vTargetLook, XMVector3Normalize(vCamLook * -1.f + vCamRight));
		break;
	}

	/*switch (m_eDir)
	{
	case Client::CPlayer::DIR_UP:
		m_vTargetLook = { 0.f,0.f,1.f };
		break;
	case Client::CPlayer::DIR_DOWN:
		m_vTargetLook = { 0.f,0.f,-1.f };
		break;
	case Client::CPlayer::DIR_RIGHT:
		m_vTargetLook = { 1.f,0.f,0.f };
		break;
	case Client::CPlayer::DIR_LEFT:
		m_vTargetLook = { -1.f,0.f,0.f };
		break;
	case Client::CPlayer::DIR_LU:
		m_vTargetLook = { -1.f,0.f,1.f };
		break;
	case Client::CPlayer::DIR_RU:
		m_vTargetLook = { 1.f,0.f,1.f };
		break;
	case Client::CPlayer::DIR_LD:
		m_vTargetLook = { -1.f,0.f,-1.f };
		break;
	case Client::CPlayer::DIR_RD:
		m_vTargetLook = { 1.f,0.f,-1.f };
		break;
	}*/
}

void CPlayer::End_Animation()
{
	if (m_pAnimModel[0]->GetAniEnd())
	{
		switch (m_eCurState)
		{
		case Client::CPlayer::HITBACK:
			Set_State(IDLE);
			break;
		case Client::CPlayer::HITFRONT:
			Set_State(IDLE);
			break;
		case Client::CPlayer::JUMP:
			
			break;
		case Client::CPlayer::JUMPEND:
			Set_State(IDLE);
			break;
		case Client::CPlayer::JUMPUP:
			Set_State(JUMP);
			break;
		case Client::CPlayer::JUMPSTART:
			Set_State(JUMPUP);
			break;
		case Client::CPlayer::IDLE:
			break;
		case Client::CPlayer::DASH:
			Set_State(IDLE);
			break;
		case Client::CPlayer::DIE:
			Set_State(RESPAWN);
			m_bDie = false;
			m_bRespawn = true;
			break;
		case Client::CPlayer::RESPAWN:
			Set_State(IDLE);
			m_bRespawn = false;
			break;
		case Client::CPlayer::RUN:
			break;
		case Client::CPlayer::RUNEND:
			Set_State(IDLE);
			break;
		case Client::CPlayer::SPINCOMBOEND:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::SPINCOMBOLOOF:	
			if (m_bSpinComboEnd)
			{
			for (int i = 0; i < MODEL_END; ++i)
				m_pAnimModel[i]->Set_AnimIndex(SPINCOMBOEND);			
			m_eNextState = SPINCOMBOEND;
			m_bSpinComboEnd = false;
			m_fSpinComboEndSpeed = 6.f;
			}
			m_Parts[PARTS_SWORD]->Set_Damage(5.f);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::SPINCOMBOSTART:
			for (int i = 0; i < MODEL_END; ++i)
				m_pAnimModel[i]->Set_AnimIndex(SPINCOMBOLOOF);			
			m_eNextState = SPINCOMBOLOOF;
			m_Parts[PARTS_SWORD]->Set_Damage(2.f);
			m_fSpinComboSpeed = 2.f;
			m_fSpinComboStartSpeed = 5.f;
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::FASTCOMBOEND:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::FASTCOMBOSTART:
			for (int i = 0; i < MODEL_END; ++i)
				m_pAnimModel[i]->Set_AnimIndex(FASTCOMBOEND);
			m_eNextState = FASTCOMBOEND;
			m_Parts[PARTS_SWORD]->Set_Damage(10.f);
			m_Parts[PARTS_SWORD]->Set_MaxHit(1);
			m_fFastComboAcc = 0.f;
			m_Parts[PARTS_SWORD]->Set_Collision(false);		
			m_fFastComboEndSpeed = 8.f;		
			CRM->Set_FovDir(true);
			break;
		case Client::CPlayer::ROCKBREAK:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::CHARGECRASH:
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			Set_State(IDLE);
			break;
		case Client::CPlayer::CHARGEREADY:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::AIRCOMBO1:
			Set_State(JUMP);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::AIRCOMBO2:
			Set_State(JUMP);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::AIRCOMBO3:
			Set_State(JUMP);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::AIRCOMBO4:
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::AIRCOMBOEND:
			Set_State(IDLE);
			break;
		case Client::CPlayer::VOIDFRONTEND:
			Set_State(IDLE);
			break;
		case Client::CPlayer::VOIDBACKEND:
			Set_State(IDLE);
			break;
		case Client::CPlayer::VOIDFRONT:
			for (int i = 0; i < MODEL_END; ++i)
				m_pAnimModel[i]->Set_AnimIndex(VOIDFRONTEND);			
			m_eNextState = VOIDFRONTEND;
			m_fVoidFront = 20.f;					
			break;
		case Client::CPlayer::VOIDBACK:
			for (int i = 0; i < MODEL_END; ++i)
				m_pAnimModel[i]->Set_AnimIndex(VOIDBACKEND);
			m_eNextState = VOIDBACKEND;
			m_fVoidBack = 4.f;
			break;
		case Client::CPlayer::NOMALCOMBO1:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::NOMALCOMBO2:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::NOMALCOMBO3:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::NOMALCOMBO4:		
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::NOMALCOMBO5:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::NOMALCOMBO6:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::GROUNDCRASH:
			break;
		case Client::CPlayer::GROUNDREADY:
			break;
		case Client::CPlayer::GROUNDRUN:
			break;
		case Client::CPlayer::LEAPDOWN:
			break;
		case Client::CPlayer::LEAPUP:
			break;
		case Client::CPlayer::LEAPEND:
			break;
		case Client::CPlayer::LEAPREADY:
			break;
		case Client::CPlayer::LEAPRUN:
			break;
		case Client::CPlayer::LEAPSTART:
			break;
		case Client::CPlayer::BLADEATTACK:
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::SLASHATTACK:			
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::ROCKSHOT:
			break;
		case Client::CPlayer::EX1ATTACK:
			break;
		case Client::CPlayer::EX2ATTACK:
			break;
		case Client::CPlayer::EX1READY:
			break;
		case Client::CPlayer::EX2READY:
			break;
		}
	}
}

void CPlayer::Get_KeyInput(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_Q))
		UM->Set_KeyDown(0);

	if (GI->Key_Down(DIK_E))
		UM->Set_KeyDown(1);

	if (GI->Key_Down(DIK_F))
		UM->Set_KeyDown(2);

	if (GI->Key_Down(DIK_R))
		UM->Set_KeyDown(3);

	if (GI->Key_Down(DIK_TAB))
		UM->Set_KeyDown(4);

	if (GI->Key_Down(DIK_C) || GI->Key_Down(DIK_V))
		UM->Set_KeyDown(5);
	switch (m_eCurState)
	{
	case Client::CPlayer::HITBACK:
		if (GI->Key_Pressing(DIK_V))
		{
			if (!UM->Get_CoolTime(4))
			{
				UM->Set_CoolTime(4);
				Set_State(VOIDBACK);
			}
			return;
		}
		break;
	case Client::CPlayer::HITFRONT:
		if (GI->Key_Pressing(DIK_C))
		{
			if (!UM->Get_CoolTime(4))
			{
				UM->Set_CoolTime(4);
				Set_State(VOIDFRONT);
			}
			return;
		}
		break;
	case Client::CPlayer::JUMP:
		Jump_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::JUMPUP:
		JumpUp_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::JUMPEND:
		JumpEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::IDLE:
		Idle_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::DASH:
		Dash_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::RUN:
		Run_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::RUNEND:
		RunEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::VOIDFRONTEND:
		VoidFrontEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::VOIDBACKEND:
		VoidBackEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::SPINCOMBOEND:
		SpinComboEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		SpinComboLoof_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		SpinComboStart_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::FASTCOMBOEND:
		FastComboEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		FastComboStart_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::ROCKBREAK:
		RockBreak_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::CHARGECRASH:
		ChargeCrash_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::CHARGEREADY:
		ChargeReady_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::AIRCOMBO1:
		AirCombo1_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::AIRCOMBO2:
		AirCombo2_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::AIRCOMBO3:
		AirCombo3_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::AIRCOMBOEND:
		AirComboEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO1:
		NomalCombo1_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO2:
		NomalCombo2_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO3:
		NomalCombo3_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO4:
		NomalCombo4_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO5:
		NomalCombo5_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO6:
		NomalCombo6_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::GROUNDREADY:
		GroundReady_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::GROUNDRUN:
		GroundRun_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::LEAPREADY:
		LeapReady_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::LEAPRUN:
		LeapRun_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::EX1READY:
		Ex1Ready_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::EX2READY:
		Ex2Ready_KeyInput(fTimeDelta);
		break;
	}
}

void CPlayer::Set_FastComboTime(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
	{
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
	{
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(5) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(6))
	{
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(7) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(8))
	{
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(9) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(10))
	{
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(10))
	{
		m_fFastComboAcc += 1.f * fTimeDelta;
		if (m_fFastComboAcc >= 0.4f)
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			m_fFastComboAcc = 0.f;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
	}

	else
		m_Parts[PARTS_SWORD]->Set_Collision(false);

}

void CPlayer::Hit_Shake()
{
	CRM->Start_Fov(70.f, 100.f);
	CRM->Set_FovDir(true);
	CRM->Start_Shake(0.3f, 2.f, 0.02f);
}

void CPlayer::Check_Battle()
{
	if (!m_bBattle)
	{
		switch (PM->Get_NowLevel())
		{
		case LEVEL_STAGE2:
		if (m_pNavigation->Get_CurrentIndex() == 41)
		{
			m_bBattle = true;
			m_pNavigation->Set_BattleIndex(41);
			PM->Add_Monster("Level_Stage2");
		}
		break;
		case LEVEL_STAGE3:
		if (m_pNavigation->Get_CurrentIndex() == 473)
		{
			m_bBattle = true;
			m_pNavigation->Set_BattleIndex(473);
			PM->Add_Monster("Level_Stage3");
		}
		break;
		case LEVEL_STAGE4:
		if (m_pNavigation->Get_CurrentIndex() == 145)
		{
			m_bBattle = true;
			m_pNavigation->Set_BattleIndex(145);
		}
		break;
		}
	}
}

bool CPlayer::Input_Direction()
{

	if (GI->Key_Pressing(DIK_W))
	{
		Set_Dir(DIR_UP);
		m_bKeyInput = true;
		if (GI->Key_Pressing(DIK_D))
		{
			Set_Dir(DIR_RU);
			m_bKeyInput = true;
		}
		if (GI->Key_Pressing(DIK_A))
		{
			Set_Dir(DIR_LU);
			m_bKeyInput = true;
		}
	}
	else if (GI->Key_Pressing(DIK_S))
	{
		Set_Dir(DIR_DOWN);
		m_bKeyInput = true;
		if (GI->Key_Pressing(DIK_D))
		{
			Set_Dir(DIR_RD);
			m_bKeyInput = true;
		}
		if (GI->Key_Pressing(DIK_A))
		{
			Set_Dir(DIR_LD);
			m_bKeyInput = true;
		}

	}
	else if (GI->Key_Pressing(DIK_D))
	{
		Set_Dir(DIR_RIGHT);
		m_bKeyInput = true;
	}
	else if (GI->Key_Pressing(DIK_A))
	{
		Set_Dir(DIR_LEFT);
		m_bKeyInput = true;
	}
	
	return m_bKeyInput; 
	/*if (PM->Get_CameraPlayerPos().z < m_vPlayerPos.z && m_fCamDistanceZ > m_fCamDistanceX)
	{
		if (GI->Key_Pressing(DIK_W))
		{
			Set_Dir(DIR_UP);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_RU);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_LU);
				m_bKeyInput = true;
			}
		}
		else if (GI->Key_Pressing(DIK_S))
		{
			Set_Dir(DIR_DOWN);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_RD);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_LD);
				m_bKeyInput = true;
			}

		}
		else if (GI->Key_Pressing(DIK_D))
		{
			Set_Dir(DIR_RIGHT);
			m_bKeyInput = true;
		}
		else if (GI->Key_Pressing(DIK_A))
		{
			Set_Dir(DIR_LEFT);
			m_bKeyInput = true;
		}
	}
	else if(PM->Get_CameraPlayerPos().z > m_vPlayerPos.z && m_fCamDistanceZ > m_fCamDistanceX)
	{
		if (GI->Key_Pressing(DIK_W))
		{
			Set_Dir(DIR_DOWN);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_LD);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_RD);
				m_bKeyInput = true;
			}
		}
		else if (GI->Key_Pressing(DIK_S))
		{
			Set_Dir(DIR_UP);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_LU);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_RU);
				m_bKeyInput = true;
			}

		}
		else if (GI->Key_Pressing(DIK_D))
		{
			Set_Dir(DIR_LEFT);
			m_bKeyInput = true;
		}
		else if (GI->Key_Pressing(DIK_A))
		{
			Set_Dir(DIR_RIGHT);
			m_bKeyInput = true;
		}
	}

	else if (PM->Get_CameraPlayerPos().x < m_vPlayerPos.x && m_fCamDistanceZ < m_fCamDistanceX)
	{
		if (GI->Key_Pressing(DIK_W))
		{
			Set_Dir(DIR_RIGHT);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_RD);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_RU);
				m_bKeyInput = true;
			}
		}
		else if (GI->Key_Pressing(DIK_S))
		{
			Set_Dir(DIR_LEFT);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_LD);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_LU);
				m_bKeyInput = true;
			}

		}
		else if (GI->Key_Pressing(DIK_D))
		{
			Set_Dir(DIR_DOWN);
			m_bKeyInput = true;
		}
		else if (GI->Key_Pressing(DIK_A))
		{
			Set_Dir(DIR_UP);
			m_bKeyInput = true;
		}
	}

	else if (PM->Get_CameraPlayerPos().x > m_vPlayerPos.x && m_fCamDistanceZ < m_fCamDistanceX)
	{
		if (GI->Key_Pressing(DIK_W))
		{
			Set_Dir(DIR_LEFT);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_LU);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_LD);
				m_bKeyInput = true;
			}
		}
		else if (GI->Key_Pressing(DIK_S))
		{
			Set_Dir(DIR_RIGHT);
			m_bKeyInput = true;
			if (GI->Key_Pressing(DIK_D))
			{
				Set_Dir(DIR_RU);
				m_bKeyInput = true;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				Set_Dir(DIR_RD);
				m_bKeyInput = true;
			}

		}
		else if (GI->Key_Pressing(DIK_D))
		{
			Set_Dir(DIR_UP);
			m_bKeyInput = true;
		}
		else if (GI->Key_Pressing(DIK_A))
		{
			Set_Dir(DIR_DOWN);
			m_bKeyInput = true;
		}
	}
	return m_bKeyInput;*/
	
}

void CPlayer::Update(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CPlayer::HITBACK:
		m_bMotionChange = true;
		break;
	case Client::CPlayer::HITFRONT:
		m_bMotionChange = true;
		break;
	case Client::CPlayer::JUMP:
		m_bMotionChange = false;
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fJumpSpeed, m_pNavigation, fTimeDelta);
			if (m_pTransformCom->Get_JumpEnd(m_pTransformCom->Get_State(CTransform::STATE_POSITION), m_pNavigation) && m_pTransformCom->Get_Jump())
			{
				m_pTransformCom->Set_Jump(false);
				m_pTransformCom->Set_Gravity(0.f);
				Set_State(JUMPEND);
				m_pTransformCom->Set_JumpEndPos(m_pNavigation);
			}
		}
		break;
	case Client::CPlayer::JUMPEND:
		m_bMotionChange = false;
		break;
	case Client::CPlayer::JUMPUP:
		m_bMotionChange = false;
		m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 15.f, m_pNavigation, fTimeDelta);
		break;
	case Client::CPlayer::JUMPSTART:
		m_bMotionChange = false;
		m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 15.f, m_pNavigation, fTimeDelta);
		m_pTransformCom->Set_Jump(true);
		break;
	case Client::CPlayer::IDLE:
		m_bMotionChange = true;
		for(int i = 0; i < OBB_END; ++i)
			m_pOBB[i]->ChangeExtents(_float3(0.7f, 2.f, 0.7f));
		m_bUseSkill = true;
		break;
	case Client::CPlayer::DASH:
		m_bMotionChange = true;
		for (int i = 0; i < OBB_END; ++i)
			m_pOBB[i]->ChangeExtents(_float3(0.7f, 1.5f, 1.4f));
		if (m_fDashSpeed > 0.5f)
			m_fDashSpeed -= 0.5f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fDashSpeed, m_pNavigation, fTimeDelta);	
		break;
	case Client::CPlayer::DIE:
		m_bCollision = false;
		break;
	case Client::CPlayer::RESPAWN:
		m_bCollision = false;
		break;
	case Client::CPlayer::RUN:
		m_bMotionChange = true;
		m_bUseSkill = true;
		for (int i = 0; i < OBB_END; ++i)
			m_pOBB[i]->ChangeExtents(_float3(0.7f, 1.5f, 1.2f));	
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRunSpeed, m_pNavigation, fTimeDelta);
		break;
	case Client::CPlayer::RUNEND:
		m_bMotionChange = true;
		if (m_fRunEndSpeed > 0.15f)
		{
			m_fRunEndSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRunEndSpeed, m_pNavigation, fTimeDelta);
		}
		break;
	case Client::CPlayer::SPINCOMBOEND:
		m_bSpinCombo = false;
		m_bMotionChange = false;
		if (m_fSpinComboEndSpeed > 0.15f)
		{
			m_fSpinComboEndSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fSpinComboEndSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		m_bMotionChange = false;
		m_fSpinComboAcc += 1.f * fTimeDelta;
		if (m_fSpinComboAcc >= 0.3f)
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			m_fSpinComboAcc = 0.f;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		m_bMotionChange = false;
		if (m_fSpinComboStartSpeed > 0.15f)
		{
			m_fSpinComboStartSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fSpinComboStartSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2))
			if (m_fSpinComboAcc >= 0.3f)
			{
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				m_fSpinComboAcc = 0.f;
			}
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::FASTCOMBOEND:
		m_bMotionChange = false;
		if (m_fFastComboEndSpeed > 0.15f)
		{
			m_fFastComboEndSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fFastComboEndSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			_bool a = m_Parts[PARTS_SWORD]->Get_bCollision();
			int c = 2;
			m_Parts[PARTS_SWORD]->Set_Collision(true);

		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		m_bMotionChange = false;
		if (m_fFastComboStartSpeed > 0.15f)
		{
			m_fFastComboStartSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fFastComboStartSpeed, m_pNavigation, fTimeDelta);
		}
			Set_FastComboTime(fTimeDelta);	
		break;
	case Client::CPlayer::ROCKBREAK:
		m_bMotionChange = false;
		if (m_fRockBreakSpeed > 0.15f)
		{
			m_fRockBreakSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRockBreakSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Shake(0.3f, 2.f, 0.02f);
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		
		break;
	case Client::CPlayer::CHARGECRASH:
		m_bMotionChange = false;
		if (m_fChargeCrashSpeed > 0.1f)
		{
			m_fChargeCrashSpeed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fChargeCrashSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Shake(0.4f, 6.f, 0.05f);
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::CHARGEREADY:
		m_bMotionChange = false;
		break;
	case Client::CPlayer::AIRCOMBO1:
		m_bMotionChange = false;
		if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(1) + 1.f)
		{
			m_pTransformCom->Set_Gravity(0.f);
			m_pTransformCom->Set_Jump(false);
		}
		else
		{
			m_pTransformCom->Set_Jump(true);
			if (m_pTransformCom->Get_JumpEnd(m_pTransformCom->Get_State(CTransform::STATE_POSITION), m_pNavigation) && m_pTransformCom->Get_Jump())
			{
				m_pTransformCom->Set_Jump(false);
				m_pTransformCom->Set_Gravity(0.f);
				Set_State(JUMPEND);
				m_pTransformCom->Set_JumpEndPos(m_pNavigation);
			}
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::AIRCOMBO2:
		m_bMotionChange = false;
		if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(1) + 1.f)
		{
			m_pTransformCom->Set_Gravity(0.f);
			m_pTransformCom->Set_Jump(false);
		}
		else
		{
			m_pTransformCom->Set_Jump(true);
			if (m_pTransformCom->Get_JumpEnd(m_pTransformCom->Get_State(CTransform::STATE_POSITION), m_pNavigation) && m_pTransformCom->Get_Jump())
			{
				m_pTransformCom->Set_Jump(false);
				m_pTransformCom->Set_Gravity(0.f);
				Set_State(JUMPEND);
				m_pTransformCom->Set_JumpEndPos(m_pNavigation);
			}
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::AIRCOMBO3:
		m_bMotionChange = false;
		if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(1) + 1.f)
		{
			m_pTransformCom->Set_Gravity(0.f);
			m_pTransformCom->Set_Jump(false);
		}
		else
		{
			m_pTransformCom->Set_Jump(true);
			if (m_pTransformCom->Get_JumpEnd(m_pTransformCom->Get_State(CTransform::STATE_POSITION), m_pNavigation) && m_pTransformCom->Get_Jump())
			{
				m_pTransformCom->Set_Jump(false);
				m_pTransformCom->Set_Gravity(0.f);
				Set_State(JUMPEND);
				m_pTransformCom->Set_JumpEndPos(m_pNavigation);
			}
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::AIRCOMBO4:
		m_bMotionChange = false;
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0))
			{
				m_pTransformCom->Set_Gravity(1.f);
				m_pTransformCom->Set_Jump(true);
			}
		
				if (m_pTransformCom->Get_JumpEnd(m_pTransformCom->Get_State(CTransform::STATE_POSITION), m_pNavigation) && m_pTransformCom->Get_Jump())
				{
					m_pTransformCom->Set_Jump(false);
					m_pTransformCom->Set_Gravity(0.f); 
					Set_State(AIRCOMBOEND);
					m_pTransformCom->Set_JumpEndPos(m_pNavigation);
				}
			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
				m_Parts[PARTS_SWORD]->Set_Collision(true);
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
		break;
	case Client::CPlayer::AIRCOMBOEND:
		m_bMotionChange = false;
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::VOIDFRONTEND:
		break;
	case Client::CPlayer::VOIDBACKEND:
		break;
	case Client::CPlayer::VOIDFRONT:
		m_bCollision = false;
		if (m_fVoidFront > 0.5f)
			m_fVoidFront -= 0.5f;		
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fVoidFront, m_pNavigation, fTimeDelta);
		break;
	case Client::CPlayer::VOIDBACK:
		m_bCollision = false;
		if (m_fVoidBack > 0.5f)
			m_fVoidBack += 0.2f;	
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), -m_fVoidBack, m_pNavigation, fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO1:
		m_bMotionChange = false;
		if (m_fNC1Speed > 0.15f)
		{
			m_fNC1Speed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC1Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::NOMALCOMBO2:
		m_bMotionChange = false;
		if (m_fNC2Speed > 0.15f)
		{
			m_fNC2Speed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC2Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::NOMALCOMBO3:
		m_bMotionChange = false;
		if (m_fNC3Speed > 0.1f)
		{
			m_fNC3Speed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC3Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			return;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			return;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::NOMALCOMBO4:
		m_bMotionChange = false;
		if (m_fNC4Speed > 0.1f)
		{
			m_fNC4Speed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC4Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			return;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			return;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::NOMALCOMBO5:
		m_bMotionChange = false;
		if (m_fNC5Speed > 0.15f)
		{
			m_fNC5Speed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC5Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Shake(0.02f, 1.5f, 0.02f);
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::NOMALCOMBO6:
		m_bMotionChange = false;
		if (m_fNC6Speed > 0.f)
		{		
			if (m_fNC6Speed >= 4.f)
			{
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION) + _vector{ 0.f,0.05f,0.f,0.f });
			}
			else
			{
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION) + _vector{ 0.f,-0.05f,0.f,0.f });
			}
			m_fNC6Speed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC6Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Fov(70.f, 100.f);
			CRM->Set_FovDir(true);
			CRM->Start_Shake(0.03f, 2.f, 0.02f);
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::GROUNDCRASH:
		break;
	case Client::CPlayer::GROUNDREADY:
		break;
	case Client::CPlayer::GROUNDRUN:
		break;
	case Client::CPlayer::LEAPDOWN:
		break;
	case Client::CPlayer::LEAPUP:
		break;
	case Client::CPlayer::LEAPEND:
		break;
	case Client::CPlayer::LEAPREADY:
		break;
	case Client::CPlayer::LEAPRUN:
		break;
	case Client::CPlayer::LEAPSTART:
		break;
	case Client::CPlayer::BLADEATTACK:
		m_bCollision = false;
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Fov(30.f, 200.f);
			CRM->Start_Shake(0.5f, 1.5f, 0.03f);
		}
			
		else
		{
			CRM->Set_FovDir(true);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			
		}
		break;
	case Client::CPlayer::SLASHATTACK:
		m_bCollision = false;
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::ROCKSHOT:
		break;
	case Client::CPlayer::EX1ATTACK:
		break;
	case Client::CPlayer::EX2ATTACK:
		break;
	case Client::CPlayer::EX1READY:
		break;
	case Client::CPlayer::EX2READY:
		break;
	case Client::CPlayer::STATE_END:
		break;
	default:
		break;
	}
}

void CPlayer::Set_AniInfo()
{
	int AniNums = m_pAnimModel[0]->Get_NumAnimations();
	int AniIndex = 0;
	for (int i = 0; i < AniNums; ++i)
	{
		m_pAnimModel[0]->Set_AnimIndex(AniIndex);
		m_pAnimModel[1]->Set_AnimIndex(AniIndex);
		m_pAnimModel[2]->Set_AnimIndex(AniIndex);
		m_pAnimModel[1]->SetDuration(m_pAnimModel[0]->GetDuration());
		m_pAnimModel[1]->SetTickPerSecond(m_pAnimModel[0]->GetTickPerSecond());
		m_pAnimModel[2]->SetDuration(m_pAnimModel[0]->GetDuration());
		m_pAnimModel[2]->SetTickPerSecond(m_pAnimModel[0]->GetTickPerSecond());
		++AniIndex;
	}
}

void CPlayer::Set_PlayerUseInfo()
{
	XMStoreFloat3(&m_vPlayerPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_fCamDistanceX = fabs(PM->Get_CameraPlayerPos().x - m_vPlayerPos.x);
	m_fCamDistanceZ = fabs(PM->Get_CameraPlayerPos().z - m_vPlayerPos.z);
}

_float CPlayer::Get_NaviPosY()
{
	return m_pNavigation->Get_PosY(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

HRESULT CPlayer::Create_Navigation(char * FileName)
{
	if (m_pNavigation != nullptr)
		Safe_Release(m_pNavigation);

	if (!strcmp(FileName, "Level_Stage1"))
	{
		CNavigation::NAVIGATIONDESC NaviDesc;
		NaviDesc.iCurrentIndex = 1;
		if (FAILED(__super::Add_Component(LEVEL_STAGE1, L"NavigationStage1", TEXT("NavigationStage1"), (CComponent**)&m_pNavigation, &NaviDesc)))
			return E_FAIL;
		_vector vPos = { 90.f, 0.6f, 103.f, 1.f };
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	}

	else if (!strcmp(FileName, "Level_Stage2"))
	{
		CNavigation::NAVIGATIONDESC NaviDesc;
		NaviDesc.iCurrentIndex = 13;
		if (FAILED(__super::Add_Component(LEVEL_STAGE2, L"NavigationStage2", TEXT("NavigationStage2"), (CComponent**)&m_pNavigation, &NaviDesc)))
			return E_FAIL;
		_vector vPos = { 100.2f, 0.147f, -1.5f, 1.f };
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		Set_Dir(DIR_UP);
	}

	else if (!strcmp(FileName, "Level_Stage3"))
	{
		CNavigation::NAVIGATIONDESC NaviDesc;
		NaviDesc.iCurrentIndex = 13;
		if (FAILED(__super::Add_Component(LEVEL_STAGE3, L"NavigationStage3", TEXT("NavigationStage3"), (CComponent**)&m_pNavigation, &NaviDesc)))
			return E_FAIL;
		_vector vPos = { 77.43f, 0.06f, 52.49f, 1.f };
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		Set_Dir(DIR_RIGHT);
	}

	else if (!strcmp(FileName, "Level_Stage4"))
	{
		CNavigation::NAVIGATIONDESC NaviDesc;
		NaviDesc.iCurrentIndex = 8;
		if (FAILED(__super::Add_Component(LEVEL_STAGE4, L"NavigationStage4", TEXT("NavigationStage4"), (CComponent**)&m_pNavigation, &NaviDesc)))
			return E_FAIL;
		_vector vPos = { 60.f, 0.5f, 0.f, 1.f };
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		Set_Dir(DIR_UP);
	}

	return S_OK;
}

void CPlayer::Reset_BattleIndex()
{
	m_pNavigation->Set_BattleIndex(-1);
}


void CPlayer::Jump_KeyInput(_float fTimeDelta)
{
	Input_Direction();
	
	if (GI->Mouse_Pressing(DIMK_LBUTTON))
	{
		Set_State(AIRCOMBO1);
		m_pTransformCom->Set_Gravity(0.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		return;
	}

	//if()
}

void CPlayer::JumpUp_KeyInput(_float fTimeDelta)
{
	Input_Direction();
}

void CPlayer::JumpEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0))
	{
		if (Input_Direction())
		{
			Set_State(RUN);
			return;
		}

		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}

		if (GI->Key_Pressing(DIK_SPACE))
		{
			Set_State(JUMPSTART);
			return;
		}

		if (GI->Key_Pressing(DIK_C))
		{
			Set_State(VOIDFRONT);
			return;
		}

		if (GI->Key_Pressing(DIK_V))
		{
			Set_State(VOIDBACK);
			return;
		}
	}
}

void CPlayer::Idle_KeyInput(_float fTimeDelta)
{
	if (GI->Key_Pressing(DIK_W) || GI->Key_Pressing(DIK_A) || GI->Key_Pressing(DIK_S) || GI->Key_Pressing(DIK_D))
	{
		Set_State(RUN);
		return;
	}

	if (GI->Key_Pressing(DIK_LSHIFT))
	{
			Set_State(DASH);	
		return;
	}

	if (GI->Mouse_Down(DIMK_LBUTTON))
	{
		Set_State(NOMALCOMBO1);
		return;
	}

	if (GI->Key_Pressing(DIK_C))
	{
		if (!UM->Get_CoolTime(4))
		{
			UM->Set_CoolTime(4);
			Set_State(VOIDFRONT);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_V))
	{
		if (!UM->Get_CoolTime(4))
		{
			UM->Set_CoolTime(4);
			Set_State(VOIDBACK);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_SPACE))
	{
		Set_State(JUMPSTART);
		return;
	}

	if (GI->Key_Pressing(DIK_F))
	{
		if (!UM->Get_CoolTime(2) && m_fNowMp >= 20.f)
		{ 
			UM->Set_CoolTime(2);
			Set_State(BLADEATTACK);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_R))
	{
		if (m_fNowMp >= 30.f && UM->Get_ExGaugeTex() >= 43)
		{
			Set_State(SLASHATTACK);
			UM->Reset_ExGaugeTex();
		}
		return;
	}

	if (GI->Key_Pressing(DIK_Q))
	{
		if (!UM->Get_CoolTime(0))
		{
			UM->Set_CoolTime(0);
			Set_State(FASTCOMBOSTART);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_E))
	{
		if (!UM->Get_CoolTime(1))
		{
			UM->Set_CoolTime(1);
			Set_State(ROCKBREAK);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_TAB))
	{
		if (!UM->Get_CoolTime(3))
		{
			UM->Set_CoolTime(3);
			Set_State(CHARGEREADY);
		}
		return;
	}
		
}

void CPlayer::Dash_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0))
	{
		if (Input_Direction())
			Set_State(RUN);
	}
}

void CPlayer::Run_KeyInput(_float fTimeDelta)
{
	if (!Input_Direction())
	{
		Set_State(RUNEND);
		return;
	}
	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}

	if (GI->Mouse_Down(DIMK_LBUTTON))
	{
		Set_State(NOMALCOMBO1);
		return;
	}

	if (GI->Key_Pressing(DIK_C))
	{
		if (!UM->Get_CoolTime(4))
		{
			UM->Set_CoolTime(4);
			Set_State(VOIDFRONT);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_V))
	{
		if (!UM->Get_CoolTime(4))
		{
			UM->Set_CoolTime(4);
			Set_State(VOIDBACK);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_SPACE))
	{
		Set_State(JUMPSTART);
		return;
	}

	if (GI->Key_Pressing(DIK_F))
	{
		if (!UM->Get_CoolTime(2))
		{
			UM->Set_CoolTime(2);
			Set_State(BLADEATTACK);
		}
			
		return;
	}

	if (GI->Key_Pressing(DIK_R))
	{
		Set_State(SLASHATTACK);
		UM->Reset_ExGaugeTex();
		return;
	}

	if (GI->Key_Pressing(DIK_Q))
	{
		if (!UM->Get_CoolTime(0))
		{
			UM->Set_CoolTime(0);
			Set_State(FASTCOMBOSTART);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_E))
	{
		if (!UM->Get_CoolTime(1))
		{
			UM->Set_CoolTime(1);
			Set_State(ROCKBREAK);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_TAB))
	{
		if (!UM->Get_CoolTime(3))
		{
			UM->Set_CoolTime(3);
			Set_State(CHARGEREADY);
		}
		return;
	}

	if (GI->Key_Pressing(DIK_2))
	{
		Set_State(CHARGEREADY);
		return;
	}

}

void CPlayer::RunEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() > 15.f)
	{
		if (GI->Key_Pressing(DIK_W) || GI->Key_Pressing(DIK_A) || GI->Key_Pressing(DIK_S) || GI->Key_Pressing(DIK_D))
		{
			Set_State(RUN);
			return;
		}

		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}

		if (GI->Mouse_Down(DIMK_LBUTTON))
		{
			Set_State(NOMALCOMBO1);
			return;
		}

		if (GI->Key_Pressing(DIK_C))
		{
			if (!UM->Get_CoolTime(4))
			{
				UM->Set_CoolTime(4);
				Set_State(VOIDFRONT);
			}
			return;
		}

		if (GI->Key_Pressing(DIK_V))
		{
			if (!UM->Get_CoolTime(4))
			{
				UM->Set_CoolTime(4);
				Set_State(VOIDBACK);
			}
			return;
		}

		if (GI->Key_Pressing(DIK_SPACE))
		{
			Set_State(JUMPSTART);
			return;
		}

		if (GI->Key_Pressing(DIK_F))
		{
			if (!UM->Get_CoolTime(2) && m_fNowMp >= 20.f)
			{
				UM->Set_CoolTime(2);
				Set_State(BLADEATTACK);
			}
			return;
		}

		if (GI->Key_Pressing(DIK_R))
		{
			if (m_fNowMp >= 30.f && UM->Get_ExGaugeTex() >= 43)
			{
				Set_State(SLASHATTACK);
				UM->Reset_ExGaugeTex();
			}
			return;
		}

		if (GI->Key_Pressing(DIK_Q))
		{
			if (!UM->Get_CoolTime(0))
			{
				UM->Set_CoolTime(0);
				Set_State(FASTCOMBOSTART);
			}
			return;
		}

		if (GI->Key_Pressing(DIK_E))
		{
			if (!UM->Get_CoolTime(1))
			{
				UM->Set_CoolTime(1);
				Set_State(ROCKBREAK);
			}
			return;
		}

		if (GI->Key_Pressing(DIK_2))
		{
			Set_State(CHARGEREADY);
			return;
		}
	}


}

void CPlayer::VoidFrontEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		if(Input_Direction())
		Set_State(RUN);
	}
}

void CPlayer::VoidBackEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		if (Input_Direction())
			Set_State(RUN);
	}
}

void CPlayer::SpinComboEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
			Set_State(RUN);

		if (GI->Key_Pressing(DIK_LSHIFT))
			Set_State(DASH);
	}
}

void CPlayer::SpinComboLoof_KeyInput(_float fTimeDelta)
{
	m_fSpinComboSpeed = 2.f;
	if (GI->Mouse_Up(DIMK_LBUTTON))
	{
		m_bSpinComboEnd = true;
	}

	m_bSpinCombo = true;


	Input_Direction();

	m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 5.f, m_pNavigation, fTimeDelta);

	/*if (GI->Key_Pressing(DIK_W))
	{	
		if (GI->Key_Pressing(DIK_D))
		{
			m_pTransformCom->Move_RU(fTimeDelta, m_fSpinComboSpeed);
			return;
		}
		if (GI->Key_Pressing(DIK_A))
		{
			m_pTransformCom->Move_LU(fTimeDelta, m_fSpinComboSpeed);
			return;
		}
		m_fSpinComboSpeed = 3;
		m_pTransformCom->Move_Up(fTimeDelta, m_fSpinComboSpeed);
		return;
	}
	else if (GI->Key_Pressing(DIK_S))
	{
		
		if (GI->Key_Pressing(DIK_D))
		{
			m_pTransformCom->Move_RD(fTimeDelta, m_fSpinComboSpeed);
			return;
		}
		if (GI->Key_Pressing(DIK_A))
		{			
			m_pTransformCom->Move_LD(fTimeDelta, m_fSpinComboSpeed);
			return;
		}
		m_fSpinComboSpeed = 3;
		m_pTransformCom->Move_Down(fTimeDelta, m_fSpinComboSpeed);
		
	}
	else if (GI->Key_Pressing(DIK_D))
	{
		m_fSpinComboSpeed = 3;
		m_pTransformCom->Move_Right(fTimeDelta, m_fSpinComboSpeed);
	}
	else if (GI->Key_Pressing(DIK_A))
	{
		m_fSpinComboSpeed = 3;
		m_pTransformCom->Move_Left(fTimeDelta, m_fSpinComboSpeed);
	}*/

}

void CPlayer::SpinComboStart_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (GI->Key_Pressing(DIK_W))
		{
			if (GI->Key_Pressing(DIK_D))
			{
				m_pTransformCom->Move_RU(fTimeDelta, m_fSpinComboSpeed);
				return;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				m_pTransformCom->Move_LU(fTimeDelta, m_fSpinComboSpeed);
				return;
			}
			m_fSpinComboSpeed = 3;
			m_pTransformCom->Move_Up(fTimeDelta, m_fSpinComboSpeed);
			return;
		}
		else if (GI->Key_Pressing(DIK_S))
		{

			if (GI->Key_Pressing(DIK_D))
			{
				m_pTransformCom->Move_RD(fTimeDelta, m_fSpinComboSpeed);
				return;
			}
			if (GI->Key_Pressing(DIK_A))
			{
				m_pTransformCom->Move_LD(fTimeDelta, m_fSpinComboSpeed);
				return;
			}
			m_fSpinComboSpeed = 3;
			m_pTransformCom->Move_Down(fTimeDelta, m_fSpinComboSpeed);

		}
		else if (GI->Key_Pressing(DIK_D))
		{
			m_fSpinComboSpeed = 3;
			m_pTransformCom->Move_Right(fTimeDelta, m_fSpinComboSpeed);
		}
		else if (GI->Key_Pressing(DIK_A))
		{
			m_fSpinComboSpeed = 3;
			m_pTransformCom->Move_Left(fTimeDelta, m_fSpinComboSpeed);
		}
	}
}

void CPlayer::FastComboEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
		{
			Set_State(RUN);
			return;
		}
	}
}

void CPlayer::FastComboStart_KeyInput(_float fTimeDelta)
{
	if(m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();
}

void CPlayer::RockBreak_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
			Set_State(RUN);

		if (GI->Key_Pressing(DIK_LSHIFT))
			Set_State(DASH);
	}
}

void CPlayer::ChargeCrash_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
		{
			Set_State(RUN);
			return;
		}
		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}
	}
}

void CPlayer::ChargeReady_KeyInput(_float fTimeDelta)
{
	Input_Direction();

	if (GI->Mouse_Pressing(DIMK_LBUTTON))
	{
		Set_State(CHARGECRASH);
		return;
	}


}

void CPlayer::AirCombo1_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (GI->Mouse_Pressing(DIMK_LBUTTON))
		{
			Set_State(AIRCOMBO2);
		}

	}
}

void CPlayer::AirCombo2_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (GI->Mouse_Pressing(DIMK_LBUTTON))
			Set_State(AIRCOMBO3);
	}
}

void CPlayer::AirCombo3_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (GI->Mouse_Pressing(DIMK_LBUTTON))
			Set_State(AIRCOMBO4);
	}
}

void CPlayer::AirComboEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0))
	{
		if (Input_Direction())
		{
			Set_State(RUN);
			return;
		}
		if (GI->Key_Pressing(DIK_C))
		{
			Set_State(VOIDFRONT);
			return;
		}
		if (GI->Key_Pressing(DIK_V))
		{
			Set_State(VOIDBACK);
			return;
		}
		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}
	}
}

void CPlayer::NomalCombo1_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
				
		if (GI->Mouse_Pressing(DIMK_LBUTTON))
		{
			m_fNomalCombo1Acc += 1.f * fTimeDelta;
			if (m_fNomalCombo1Acc > 0.2f)
			{
				Set_State(SPINCOMBOSTART);
				return;
			}
		}
		

		if (GI->Mouse_Up(DIMK_LBUTTON))
		{			
			Set_State(NOMALCOMBO2);
			return;			
		}

	
		if (GI->Mouse_Down(DIMK_RBUTTON))
		{
			Set_State(NOMALCOMBO5);
			return;
		}

		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}
	}	
}

void CPlayer::NomalCombo2_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (GI->Mouse_Down(DIMK_LBUTTON))
		{
			Set_State(NOMALCOMBO3);
			return;
		}

		if (GI->Mouse_Down(DIMK_RBUTTON))
		{
			Set_State(NOMALCOMBO4);
			return;
		}

		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}
	}
}

void CPlayer::NomalCombo3_KeyInput(_float fTimeDelta)
{
	
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
			Set_State(RUN);

		if (GI->Key_Pressing(DIK_LSHIFT))
			Set_State(DASH);
	}
	
}

void CPlayer::NomalCombo4_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
			Set_State(RUN);

		if (GI->Key_Pressing(DIK_LSHIFT))
			Set_State(DASH);
	}
}

void CPlayer::NomalCombo5_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (GI->Mouse_Down(DIMK_RBUTTON))
		{
			Set_State(NOMALCOMBO6);
			Input_Direction();
		}
	}
}

void CPlayer::NomalCombo6_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
	{
		Input_Direction();
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1))
	{
		if (Input_Direction())
			Set_State(RUN);

		if (GI->Key_Pressing(DIK_LSHIFT))
			Set_State(DASH);
	}
}

void CPlayer::GroundReady_KeyInput(_float fTimeDelta)
{
}

void CPlayer::GroundRun_KeyInput(_float fTimeDelta)
{
}

void CPlayer::LeapReady_KeyInput(_float fTimeDelta)
{
}

void CPlayer::LeapRun_KeyInput(_float fTimeDelta)
{
}

void CPlayer::Ex1Ready_KeyInput(_float fTimeDelta)
{
}

void CPlayer::Ex2Ready_KeyInput(_float fTimeDelta)
{
}

HRESULT CPlayer::Ready_Sockets()
{

	CHierarchyNode*		pHead = m_pAnimModel[MODEL_PLAYER]->Get_HierarchyNode("Head");
	if (nullptr == pHead)
		return E_FAIL;
	m_Sockets.push_back(pHead);

	CHierarchyNode*		pWeaponHandR = m_pAnimModel[MODEL_PLAYER]->Get_HierarchyNode("Weapon_Hand_R");
	if (nullptr == pWeaponHandR)
		return E_FAIL;
	m_Sockets.push_back(pWeaponHandR);
	
	return S_OK;
}

HRESULT CPlayer::Ready_PlayerParts()
{	

	MESHINFO* MeshInfo = new MESHINFO;

	CGameObject*		pPlayerHead = GI->Clone_GameObject(TEXT("PlayerHead"),MeshInfo);	
	if (nullptr == pPlayerHead)
		return E_FAIL;
	m_Parts.push_back((CMesh*)pPlayerHead);

	CGameObject*		pPlayerHairBack = GI->Clone_GameObject(TEXT("PlayerHairBack"), MeshInfo);
	if (nullptr == pPlayerHairBack)
		return E_FAIL;
	m_Parts.push_back((CMesh*)pPlayerHairBack);

	CGameObject*		pPlayerHairFront = GI->Clone_GameObject(TEXT("PlayerHairFront"), MeshInfo);
	if (nullptr == pPlayerHairFront)
		return E_FAIL;
	m_Parts.push_back((CMesh*)pPlayerHairFront);

	CGameObject*		pPlayerHairSide = GI->Clone_GameObject(TEXT("PlayerHairSide"), MeshInfo);
	if (nullptr == pPlayerHairSide)
		return E_FAIL;
	m_Parts.push_back((CMesh*)pPlayerHairSide);

	CGameObject*		pPlayerHairTail = GI->Clone_GameObject(TEXT("PlayerHairTail"), MeshInfo);
	if (nullptr == pPlayerHairTail)
		return E_FAIL;
	m_Parts.push_back((CMesh*)pPlayerHairTail);

	CGameObject*		pPlayerSword = GI->Clone_GameObject(TEXT("PlayerSword"), MeshInfo);
	if (nullptr == pPlayerSword)
		return E_FAIL;
	m_Parts.push_back((CMesh*)pPlayerSword);
	

	Safe_Delete(MeshInfo);

	return S_OK;

	
}

HRESULT CPlayer::Update_Parts()
{
	
	_matrix HeadMatxrix = m_Sockets[SOCKET_HEAD]->Get_CombinedTransformation()* m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
	_matrix WeaponHandRMatrix = m_Sockets[SOCKET_WEAPONHANDR]->Get_CombinedTransformation()* m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
	m_Parts[PARTS_HEAD]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRBACK]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRFRONT]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRSIDE]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRTAIL]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_SWORD]->SetUp_State(WeaponHandRMatrix);
	
	return S_OK;
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();
	for (int i = 0; i < OBB_END; ++i)
		Safe_Release(m_pOBB[i]);

	Safe_Release(m_pAnimModel[MODEL_PLAYER]);
	Safe_Release(m_pAnimModel[MODEL_TOP]);
	Safe_Release(m_pAnimModel[MODEL_BOTTOM]);

	for (auto& pPart : m_Parts)
		Safe_Release(pPart);

	m_Parts.clear();
	Safe_Release(m_pNavigation);

}
