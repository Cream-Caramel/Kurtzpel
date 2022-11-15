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
#include "PlayerSword.h"
#include "Particle_Manager.h"
#include "PlayerLight.h"
#include "PlayerEx.h"
#include "PlayerGage.h"
#include "PlayerGage2.h"

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
	m_bColliderRender = false;
	
	m_pTransformCom->Set_Gravity(0.f);
	m_pTransformCom->Set_JumpPower(0.4f);
	m_bAction = false;

	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{

	if (!m_pAnimModel[0]->GetChangeBool())
		m_eCurState = m_eNextState;

	if (GI->Key_Down(DIK_1))
		Set_State(DIE);

	if (GI->Key_Down(DIK_4))
	{
		
	}
	if (GI->Key_Down(DIK_0))
		m_bColliderRender = !m_bColliderRender;

	if (GI->Key_Down(DIK_5))
	{
		CAnimMesh::EFFECTINFO EffectInfo;
		EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
		EffectInfo.vScale = _float3{ 1.2f,1.2f,1.2f };
		GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
	}
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

	for (auto& pTrail : m_SwordTrails)
		pTrail->LateTick(fTimeDelta);

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

				if (FAILED(m_pAnimModel[i]->SetUp_OnShader(m_pShaderCom, m_pAnimModel[i]->Get_MaterialIndex(j), TEX_NORMALS, "g_NormalTexture")))
				{
					m_bNormalTex = false;
					m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float3));
				}
				else
				{
					m_bNormalTex = true;
					m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float3));
				}

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
			if (m_eCurState == HITBACK)
			{
				Hit_Shake();
				for (int i = 0; i < MODEL_END; ++i)
				{
					Set_State(HITFRONT);
				}
				if (m_fNowHp <= 0.f)
				{
					m_fNowHp = 0.f;
					m_bDie = true;
					Set_State(DIE);
					GI->PlaySoundW(L"DieVoice.ogg", SD_PLAYERVOICE, 0.9f);
				}

				m_bCollision = false;
				Minus_Hp(pOther->Get_Damage());
				return;
			}

			if (m_eCurState == HITFRONT)
			{
				Hit_Shake();
				for (int i = 0; i < MODEL_END; ++i)
				{
					Set_State(HITBACK);
				}
				if (m_fNowHp <= 0.f)
				{
					m_fNowHp = 0.f;
					m_bDie = true;
					Set_State(DIE);
				}

				m_bCollision = false;
				Minus_Hp(pOther->Get_Damage());
				return;
			}

			int random = GI->Get_Random(1, 2);
			if (random == 1)
			{
				Hit_Shake();
				for (int i = 0; i < MODEL_END; ++i)
				{
					Set_State(HITBACK);
				}
			}
			else
			{
				Hit_Shake();
				for (int i = 0; i < MODEL_END; ++i)
				{
					Set_State(HITFRONT);
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

	m_Parts[PARTS_SWORD]->Set_Collision(false);

	m_eNextState = eState;

	switch (m_eNextState)
	{
	case Client::CPlayer::HITBACK:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"HitBack.ogg", SD_PLAYERVOICE, 0.9f);
		m_bAction = false;
		break;
	case Client::CPlayer::HITFRONT:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"HitFront.ogg", SD_PLAYERVOICE, 0.9f);
		m_bAction = false;
		break;
	case Client::CPlayer::JUMP:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		m_bAction = false;
		break;
	case Client::CPlayer::JUMPEND:
		if (m_bAction)
		{
			m_bAction = false;
			Change_WeaponPos();
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
		}
		m_bAction = false;
		break;
	case Client::CPlayer::JUMPUP:
		if (m_bAction)
		{
			m_bAction = false;
			Change_WeaponPos();
		}
		m_bAction = false;
		break;
	case Client::CPlayer::JUMPSTART:	
		if (m_bAction)
		{
			m_bAction = false;
			Change_WeaponPos();
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
		}
		m_bAction = false;
		GI->PlaySoundW(L"Jump.ogg", SD_PLAYER1, 0.6f);
		m_pTransformCom->Set_Jump(true);
		m_pTransformCom->Set_Gravity(0.f);
		m_pTransformCom->Set_JumpPower(0.6f);
		m_fJumpSpeed = 10.f;
		break;
	case Client::CPlayer::IDLE:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		m_bAction = false;
		CRM->Set_FovSpeed(150.f);
		CRM->Set_FovDir(true);
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::DASH:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"Dash.ogg", SD_PLAYER1, 0.6f);
		m_bAction = false;
		m_fNowMp -= 5.f;
		m_fDashSpeed = 20.f;
		if (!CRM->Get_bFov())
		{
			CRM->Start_Fov(80.f, 100.f);
			CRM->Set_FovDir(true);
		}
		else
		{
			CRM->Fix_Fov(80.f, 100.f);
		}
		break;
	case Client::CPlayer::DIE:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		m_bAction = false;
		CRM->Start_Fov(30.f, 30.f);
		break;
	case Client::CPlayer::RESPAWN:
		if (m_bAction)
		{
			m_bAction = false;
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"Respawn.ogg", SD_PLAYERVOICE, 0.6f);
		m_bAction = false;
		CRM->Set_FovSpeed(60.f);
		CRM->Set_FovDir(true);
		m_bSpinCombo = false;
		break;
	case Client::CPlayer::RUN:	
		if (m_bAction)
		{
			m_fRunSoundAcc = m_fRunSoundTempo - 0.01f;
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		m_fRunSpeed = 8.f;
		break;
	case Client::CPlayer::RUNEND:
		if (m_bAction)
		{
			m_bAction = false;
			Change_WeaponPos();
		}
		m_bAction = false;
		GI->PlaySoundW(L"RunEnd.ogg", SD_PLAYER1, 0.6f);
		m_fRunEndSpeed = 8.f;
		break;
	case Client::CPlayer::SPINCOMBOEND:
		
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"SpinComboStart.ogg", SD_PLAYER1, 0.6f);
		CRM->Start_Fov(50.f, 100.f);
		m_fNowMp -= 15.f;
		m_Parts[PARTS_SWORD]->Set_Damage(2.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(999);
		break;
	case Client::CPlayer::FASTCOMBOEND:
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"FastVoice.ogg", SD_PLAYERVOICE, 0.9f);
		m_fNowMp -= 10.f;
		m_fFastComboStartSpeed = 5.f;
		m_Parts[PARTS_SWORD]->Set_Damage(2.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(30);
		m_fFastComboAcc = 0.f;
		break;
	case Client::CPlayer::ROCKBREAK:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		m_bAction = true;
		m_fNowMp -= 5.f;
		m_fRockBreakSpeed = 6.f;
		RockBreakVoice();
		GI->PlaySoundW(L"RockBreakStart.ogg", SD_PLAYER1, 0.6f);
		m_Parts[PARTS_SWORD]->Set_Damage(1.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::CHARGECRASH:
		GI->PlaySoundW(L"SpinComboEnd.ogg", SD_PLAYER1, 0.6f); 
		m_fChargeCrashSpeed = 6.f;
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		PM->Set_PlayerGage2_1(true);
		PM->Set_PlayerGage2_2(true);
		
		break;
	case Client::CPlayer::CHARGEREADY:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		m_bCharge1 = false;
		m_bCharge2 = false;
		PM->Set_PlayerGage2_1(false);
		PM->Set_PlayerGage2_2(false);
		GI->PlaySoundW(L"ChargeVoice.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"ChargeReady.ogg", SD_PLAYER1, 0.6f);
		m_bMotionChange = false;
		CRM->Start_Fov(80.f, 30.f);
		m_fNowMp -= 20.f;
		break;
	case Client::CPlayer::AIRCOMBO1:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"AttackVoice1.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"AirCombo1.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 3.f;
		m_pTransformCom->Set_JumpPower(0.f);
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBO2:
		GI->PlaySoundW(L"AttackVoice2.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"AirCombo2.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 3.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBO3:
		GI->PlaySoundW(L"AttackVoice3.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"AirCombo3.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 3.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBO4:
		GI->PlaySoundW(L"AttackVoice4.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"AirCombo4.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 3.f;
		m_Parts[PARTS_SWORD]->Set_Damage(14.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::AIRCOMBOEND:
		CRM->Start_Fov(50.f, 140.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.3f, 3.f, 0.03f);
		CRM->Set_FovDir(true);
		m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::VOIDFRONTEND:
		
		break;
	case Client::CPlayer::VOIDFRONT:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"VoidFront.ogg", SD_PLAYER1, 0.6f);
		CRM->Start_Fov(40.f, 100.f);
		CRM->Set_FovDir(true);
		m_fNowMp -= 10.f;
		break;
	case Client::CPlayer::VOIDBACKEND:
		break;
	case Client::CPlayer::VOIDBACK:
		if (m_bAction)
		{
			m_bAction = false;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(false);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"VoidBack.ogg", SD_PLAYER1, 0.6f);
		CRM->Start_Fov(40.f, 70.f);
		CRM->Set_FovDir(true);
		m_fNowMp -= 10.f;
		break;
	case Client::CPlayer::NOMALCOMBO1:
	{
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		Update_SwordTrails(NOMALCOMBO1);
		int iRandom = GI->Get_Random(1, 2);
		if (iRandom == 1)
			GI->PlaySoundW(L"AttackVoice1.ogg", SD_PLAYERVOICE, 0.9f);
		else
			GI->PlaySoundW(L"AttackVoice2.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"NormalCombo1.ogg", SD_PLAYER1, 0.6f);
		CRM->Start_Fov(55.f, 90.f);
		m_fNC1Speed = 5.f;
		m_fNomalCombo1Acc = 0.f;
		m_fNowMp -= 2.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	}
	case Client::CPlayer::NOMALCOMBO2:
		CRM->Fix_Fov(50.f, 90.f);
		GI->PlaySoundW(L"AttackVoice3.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"NormalCombo2.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 3.f;
		m_fNC2Speed = 5.f;
		m_Parts[PARTS_SWORD]->Set_Damage(7.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::NOMALCOMBO3:
		CRM->Fix_Fov(40.f, 90.f);
		RastAttackVoice();
		GI->PlaySoundW(L"NormalCombo3.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 5.f;
		m_fNC3Speed = 6.f;
		m_Parts[PARTS_SWORD]->Set_Damage(8.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(2);
		break;
	case Client::CPlayer::NOMALCOMBO4:
		CRM->Fix_Fov(40.f, 90.f);
		RastAttackVoice();
		GI->PlaySoundW(L"NormalCombo4.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 5.f;
		m_fNC4Speed = 6.f;
		m_Parts[PARTS_SWORD]->Set_Damage(10.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(2);
		break;
	case Client::CPlayer::NOMALCOMBO5:
		CRM->Fix_Fov(50.f, 90.f);
		m_fNowMp -= 3.f;
		m_fNC5Speed = 5.f;
		GI->PlaySoundW(L"AttackVoice4.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"NormalCombo5.ogg", SD_PLAYER1, 0.6f);
		m_Parts[PARTS_SWORD]->Set_Damage(10.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::NOMALCOMBO6:
		CRM->Fix_Fov(40.f, 90.f);
		m_fNowMp -= 5.f;
		m_fNC6Speed = 8.f;
		GI->PlaySoundW(L"AttackVoice7.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"RockBreakStart.ogg", SD_PLAYER1, 0.6f);
		m_Parts[PARTS_SWORD]->Set_Damage(20.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	case Client::CPlayer::BLADEATTACK:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		GI->PlaySoundW(L"BladeAttackStart.ogg", SD_PLAYER1, 1.f);
		m_fNowMp -= 20.f;
		m_Parts[PARTS_SWORD]->Set_Damage(4.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(10);
		break;
	case Client::CPlayer::SLASHATTACK:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		m_bDoubleSlash = true;
		m_fNowMp -= 30.f;
		GI->PlaySoundW(L"SlashVoice.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"DoubleSlash.ogg", SD_PLAYER2, 0.6f);
		m_Parts[PARTS_SWORD]->Set_Damage(5.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(30);
		break;
	case Client::CPlayer::ROCKSHOT:
		
		break;
	case Client::CPlayer::EX1ATTACK:
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
	
		PM->Set_PlayerGage2_1(true);
		PM->Set_PlayerGage2_2(true);
		GI->PlaySoundW(L"RockBreakStart.ogg", SD_PLAYER1, 0.6f);
		if(m_fEx1AttackSpeed != 15.1f)
			XMStoreFloat3(&m_vTargetLook, XMLoadFloat3(&PM->Get_BossPointer()->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		
		break;
	case Client::CPlayer::EX2ATTACK:
		
	case Client::CPlayer::EX1READY:
		
		break;
	case Client::CPlayer::EX2READY:	
		if (!m_bAction)
		{
			m_bAction = true;
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Trail(true);
			Change_WeaponPos();
		}
		m_bCharge1 = false;
		m_bCharge2 = false;
		PM->Set_PlayerGage2_1(false);
		PM->Set_PlayerGage2_2(false);
		GI->PlaySoundW(L"ChargeVoice.ogg", SD_PLAYERVOICE, 0.9f);
		GI->PlaySoundW(L"ChargeReady.ogg", SD_PLAYER1, 0.6f);
		m_fNowMp -= 20.f;
		CRM->Start_Fov(80.f, 30.f);
		m_Parts[PARTS_SWORD]->Set_MaxHit(1);
		break;
	}
	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->SetNextIndex(m_eNextState);
		m_pAnimModel[i]->SetChangeBool(true);
	}

	if (!m_bAction)
	{
		for (int i = 0; i < SWORDTRAIL_END; ++i)
		{
			m_SwordTrails[i]->Set_bRenderObj(false);
		}
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
			CRM->Fix_Fov(30.f, 160.f);
			CRM->Set_FovDir(true);
			RastAttackVoice();
			GI->PlaySoundW(L"SpinComboEnd.ogg", SD_PLAYER1, 0.6f);
			}
			
			m_Parts[PARTS_SWORD]->Set_Damage(5.f);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			break;
		case Client::CPlayer::SPINCOMBOSTART:
			for (int i = 0; i < MODEL_END; ++i)
				m_pAnimModel[i]->Set_AnimIndex(SPINCOMBOLOOF);		
	
			m_fSpinComboLoofAcc = m_fSpinComboLoofTempo - 0.01f;
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
			CRM->Fix_Fov(30.f, 160.f);
			CRM->Set_FovDir(true);
			GI->PlaySoundW(L"FastComboEnd.ogg", SD_PLAYER1, 0.6f);
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
			Set_State(IDLE);
			m_Parts[PARTS_SWORD]->Set_Collision(false);
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
	case Client::CPlayer::EX1ATTACK:
		Ex1Attack_keyInput(fTimeDelta);
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
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(FASTCOMBOSTART);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(EX1READY);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(5) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(6))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(FASTCOMBOSTART);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(7) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(8))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(EX1READY);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(9) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(10))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(FASTCOMBOSTART);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(11) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(12))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(EX1READY);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(13) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(14))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(FASTCOMBOSTART);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(15) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(16))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
		Update_SwordTrails(EX1READY);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(17) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(18))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Set_FovDir(true);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER2, 0.6f);
		Update_SwordTrails(FASTCOMBOSTART);
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(19) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(20))
	{
		CRM->Start_Fov(57.f, 100.f);
		CRM->Start_Shake(0.2f, 3.f, 0.03f);
		m_Parts[PARTS_SWORD]->Set_Collision(true);
		GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYERSKILL1, 0.6f);
		Update_SwordTrails(EX1READY);
		return;
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

void CPlayer::Change_WeaponPos()
{
	if (m_bAction)
		((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_RHand();
	else
		((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_Spine();
}

void CPlayer::RockBreakVoice()
{
	int iRandom = GI->Get_Random(1, 3);

	switch (iRandom)
	{
	case 1:
		GI->PlaySoundW(L"AttackVoice4.ogg", SD_PLAYERVOICE, 0.9f);
		break;
	case 2:
		GI->PlaySoundW(L"AttackVoice5.ogg", SD_PLAYERVOICE, 0.9f);
		break;
	case 3:
		GI->PlaySoundW(L"AttackVoice6.ogg", SD_PLAYERVOICE, 0.9f);
		break;
	}

	
}

void CPlayer::RastAttackVoice()
{
	int iRandom = GI->Get_Random(1, 3);

	switch (iRandom)
	{
	case 1:
		GI->PlaySoundW(L"AttackVoice5.ogg", SD_PLAYERVOICE, 0.9f);
		break;
	case 2:
		GI->PlaySoundW(L"AttackVoice6.ogg", SD_PLAYERVOICE, 0.9f);
		break;
	case 3:
		GI->PlaySoundW(L"AttackVoice7.ogg", SD_PLAYERVOICE, 0.9f);
		break;
	}
}

void CPlayer::Ex1AttackLight()
{
	CPlayerLight::PLAYERLIGHT PlayerLightInfo;
	XMStoreFloat4(&PlayerLightInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	PlayerLightInfo.fCloseSpeed = 1.f;
	PlayerLightInfo.vScale = { 4.f,10.f,4.f };
	PlayerLightInfo.vAngle = { 0.f,0.f,0.f };
	GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);


	for (int i = 0; i < 15; ++i)
	{
		XMStoreFloat4(&PlayerLightInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		PlayerLightInfo.fCloseSpeed = GI->Get_FloatRandom(0.05f, 0.1f);
		PlayerLightInfo.vScale = { GI->Get_FloatRandom(0.5f,2.f),10.f, GI->Get_FloatRandom(0.5f,2.f) };
		PlayerLightInfo.vAngle = { GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f) };
		GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);
	}
}

void CPlayer::ChargeAttackLight()
{
	CPlayerLight::PLAYERLIGHT PlayerLightInfo;
	_vector Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION) + XMLoadFloat3(&m_vTargetLook) * 5.f;
	XMStoreFloat4(&PlayerLightInfo.vPos, Pos);
	PlayerLightInfo.fCloseSpeed = 1.f;
	PlayerLightInfo.vScale = { 5.f,10.f,5.f };
	PlayerLightInfo.vAngle = { 0.f,0.f,0.f };
	GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);


	for (int i = 0; i < 15; ++i)
	{
		_vector Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION) + XMLoadFloat3(&m_vTargetLook) * 5.f;
		XMStoreFloat4(&PlayerLightInfo.vPos, Pos);
		PlayerLightInfo.fCloseSpeed = GI->Get_FloatRandom(0.05f, 0.1f);
		PlayerLightInfo.vScale = { GI->Get_FloatRandom(1.f,2.f),10.f, GI->Get_FloatRandom(1.f,2.f) };
		PlayerLightInfo.vAngle = { GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f) };
		GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);
	}
}

void CPlayer::CreateGage(_bool Gage2_1)
{
	
	_float fRotation = -10.f;
	for (int i = 0; i < 5; ++i)
	{
		if (Gage2_1)
		{
			fRotation += 60.f;
			CPlayerGage2::PLAYERGAGE2INFO PlayerGage2Info;
			PlayerGage2Info.bGage2_1 = true;
			PlayerGage2Info.eTurnDir = CMesh::TURN_BACK;
			PlayerGage2Info.fMaxUVIndexX = 1.f;
			PlayerGage2Info.fMaxUVIndexY = 4.f;
			PlayerGage2Info.fUVSpeed = 0.08f;
			XMStoreFloat4(&PlayerGage2Info.vWorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PlayerGage2Info.fRotation = fRotation;
			GI->Add_GameObjectToLayer(L"PlayerGage2", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerGage2Info);
		}
		else
		{
			fRotation += 60.f;
			CPlayerGage2::PLAYERGAGE2INFO PlayerGage2Info;
			PlayerGage2Info.bGage2_1 = false;
			PlayerGage2Info.eTurnDir = CMesh::TURN_BACK;
			PlayerGage2Info.fMaxUVIndexX = 2.f;
			PlayerGage2Info.fMaxUVIndexY = 2.f;
			PlayerGage2Info.fUVSpeed = 0.08f;
			XMStoreFloat4(&PlayerGage2Info.vWorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PlayerGage2Info.fRotation = fRotation;
			GI->Add_GameObjectToLayer(L"PlayerGage2", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerGage2Info);
		}
		
	}
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
			PM->Add_Monster("Level_Stage4");
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
				GI->PlaySoundW(L"JumpEnd.ogg", SD_PLAYER1, 0.6f);
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
		for (int i = 0; i < OBB_END; ++i)
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
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(1))
		{
			GI->PlaySoundW(L"Die_1.ogg", SD_PLAYER1, 0.9f);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			GI->PlaySoundW(L"Die_2.ogg", SD_PLAYER2, 0.9f);
		}
		break;
	case Client::CPlayer::RESPAWN:
		m_bCollision = false;
		break;
	case Client::CPlayer::RUN:

		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = true;
			m_fRunSoundAcc += 1.f * fTimeDelta;
			if (m_fRunSoundAcc >= m_fRunSoundTempo)
			{
				GI->PlaySoundW(L"Run.ogg", SD_PLAYER1, 0.6f);
				m_fRunSoundAcc = 0.f;
			}

			if (CRM->Get_bFov())
			{
				CRM->Set_FovSpeed(150.f);
				CRM->Set_FovDir(true);
			}
		}
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
		Set_SwordTrailMatrix();
		if (m_fSpinComboEndSpeed > 0.15f)
		{
			m_fSpinComboEndSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fSpinComboEndSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			Update_SwordTrails(SPINCOMBOEND);
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Shake(0.3f, 3.f, 0.03f);
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
		m_fSpinComboLoofAcc += 1.f * fTimeDelta;
		if (m_fSpinComboLoofAcc >= m_fSpinComboLoofTempo)
		{
			GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
			m_fSpinComboLoofAcc = 0.f;
		}

		m_fSpinComboAcc += 1.f * fTimeDelta;
		if (m_fSpinComboAcc >= 0.3f)
		{
			Update_SwordTrails(SPINCOMBOLOOF);
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			m_fSpinComboAcc = 0.f;
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		Set_SwordTrailMatrix();
		m_bMotionChange = false;
		if (m_fSpinComboStartSpeed > 0.15f)
		{
			m_fSpinComboStartSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fSpinComboStartSpeed, m_pNavigation, fTimeDelta);
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			GI->PlaySoundW(L"AttackVoice4.ogg", SD_PLAYERVOICE, 0.6f);
			GI->PlaySoundW(L"NormalCombo2.ogg", SD_PLAYER1, 0.6f);
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			Update_SwordTrails(SPINCOMBOSTART);

		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(6))
		{
			m_fSpinComboLoofAcc += 1.f * fTimeDelta;
			if (m_fSpinComboLoofAcc >= m_fSpinComboLoofTempo)
			{
				Update_SwordTrails(SPINCOMBOLOOF);
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				GI->PlaySoundW(L"SpinComboLoof.ogg", SD_PLAYER1, 0.6f);
				m_fSpinComboLoofAcc = 0.f;
			}
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
		break;
	case Client::CPlayer::FASTCOMBOEND:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
		if (m_fFastComboEndSpeed > 0.15f)
		{
			m_fFastComboEndSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fFastComboEndSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
		{
			Update_SwordTrails(FASTCOMBOEND);
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Shake(0.3f, 4.f, 0.04f);
			RastAttackVoice();
		}
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
		if (m_fFastComboStartSpeed > 0.15f)
		{
			m_fFastComboStartSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fFastComboStartSpeed, m_pNavigation, fTimeDelta);
		}
		Set_FastComboTime(fTimeDelta);
		break;
	case Client::CPlayer::ROCKBREAK:
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
			if (m_fRockBreakSpeed > 0.15f)
			{
				m_fRockBreakSpeed -= 0.15f;
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRockBreakSpeed, m_pNavigation, fTimeDelta);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
				Update_SwordTrails(ROCKBREAK);

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
				GI->Add_GameObjectToLayer(L"PlayerRockBreak", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
				GI->PlaySoundW(L"RockBreakEnd.ogg", SD_PLAYER1, 0.6f);
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				CRM->Start_Shake(0.3f, 3.f, 0.03f);

			}
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
		break;
	case Client::CPlayer::CHARGECRASH:
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
			if (m_fChargeCrashSpeed > 0.1f)
			{
				m_fChargeCrashSpeed -= 0.1f;
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fChargeCrashSpeed, m_pNavigation, fTimeDelta);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
				Update_SwordTrails(CHARGECRASH);


			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.vScale = _float3{ 2.5f,2.5f,2.5f };
				GI->Add_GameObjectToLayer(L"PlayerRockBreak", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
				((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_OBB(_float3(10.f, 10.f, 10.f));
				GI->PlaySoundW(L"ChargeAttack.ogg", SD_PLAYER1, 0.6f);
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				CRM->Start_Shake(0.4f, 6.f, 0.05f);
				CRM->Set_FovDir(true);
				_float4 WorldPos;
				_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION) + XMLoadFloat3(&m_vTargetLook) * 5.f;
				XMStoreFloat4(&WorldPos, vPos);
				if (m_bCharge1)
				{
					PTM->CreateParticle(L"Player1", WorldPos, false, true, CAlphaParticle::DIR_END);
					m_Parts[PARTS_SWORD]->Set_Damage(70.f);
				}
				else if (m_bCharge2)
				{
					PTM->CreateParticle(L"Player2", WorldPos, false, true, CAlphaParticle::DIR_END);
					m_Parts[PARTS_SWORD]->Set_Damage(50.f);
				}
				else
					m_Parts[PARTS_SWORD]->Set_Damage(30.f);
				ChargeAttackLight();
			}
			else
			{
				m_Parts[PARTS_SWORD]->Set_Collision(false);
				((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_OBB(_float3(0.3f, 2.2f, 0.3f));
			}
		}
		break;
	case Client::CPlayer::CHARGEREADY:
		m_bMotionChange = false;

		if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"ChargeYellow", WorldPos, false, true, CAlphaParticle::DIR_PLAYER);
		}
		else if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"ChargeOrange", WorldPos, false, true, CAlphaParticle::DIR_PLAYER);
			CRM->Start_Shake(0.2f, 1.f, 0.02f);
			m_bCharge2 = true;
		}

		else if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"ChargeOrange2", WorldPos, false, true, CAlphaParticle::DIR_PLAYER);
			CRM->Start_Shake(0.2f, 1.5f, 0.02f);
			m_bCharge1 = true;
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
		{
			CRM->Fix_Fov(70.f, 100.f);
			CRM->Start_Shake(0.3f, 2.5f, 0.03f);
			GI->PlaySoundW(L"ChargeReady.ogg", SD_PLAYER1, 0.6f);
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"PlayerGage2_2", WorldPos, false, true, CAlphaParticle::DIR_END);
			CreateGage(false);
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
		{
			CRM->Fix_Fov(60.f, 100.f);
			CRM->Start_Shake(0.4f, 3.5f, 0.04f);
			GI->PlaySoundW(L"ChargeReady.ogg", SD_PLAYER1, 1.f);
			PM->Set_PlayerGage2_2(true);
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"PlayerGage2_1", WorldPos, false, true, CAlphaParticle::DIR_END);
			CreateGage(true);
		}
		break;
	case Client::CPlayer::AIRCOMBO1:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
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

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
			Update_SwordTrails(AIRCOMBO1);

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::AIRCOMBO2:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
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

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
			Update_SwordTrails(AIRCOMBO2);

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::AIRCOMBO3:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
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

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
			Update_SwordTrails(AIRCOMBO3);

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			m_Parts[PARTS_SWORD]->Set_Collision(true);
		else
			m_Parts[PARTS_SWORD]->Set_Collision(false);
		break;
	case Client::CPlayer::AIRCOMBO4:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0))
			{
				m_pTransformCom->Set_Gravity(1.f);
				m_pTransformCom->Set_Jump(true);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
				Update_SwordTrails(AIRCOMBO4);
			if (m_pTransformCom->Get_JumpEnd(m_pTransformCom->Get_State(CTransform::STATE_POSITION), m_pNavigation) && m_pTransformCom->Get_Jump())
			{
				m_pTransformCom->Set_Jump(false);
				GI->PlaySoundW(L"AirComboEnd.ogg", SD_PLAYER1, 0.6f);
				m_pTransformCom->Set_Gravity(0.f);
				Set_State(AIRCOMBOEND);
				m_pTransformCom->Set_JumpEndPos(m_pNavigation);
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.WorldMatrix.r[1].m128_f32[1] = m_pNavigation->Get_PosY(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
				GI->Add_GameObjectToLayer(L"PlayerRockBreak", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
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
		if (UM->Get_Count() > 0)
			m_bCollision = false;
		if (m_fVoidFront > 0.5f)
			m_fVoidFront -= 0.5f;
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fVoidFront, m_pNavigation, fTimeDelta);
		break;
	case Client::CPlayer::VOIDBACK:
		if (UM->Get_Count() > 0)
			m_bCollision = false;
		if (m_fVoidBack > 0.5f)
			m_fVoidBack += 0.2f;
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), -m_fVoidBack, m_pNavigation, fTimeDelta);
		break;
	case Client::CPlayer::NOMALCOMBO1:
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
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
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
			Update_SwordTrails(NOMALCOMBO2);


		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
			if (m_fNC2Speed > 0.15f)
			{
				m_fNC2Speed -= 0.15f;
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC2Speed, m_pNavigation, fTimeDelta);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
				m_Parts[PARTS_SWORD]->Set_Collision(true);
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
		break;
	case Client::CPlayer::NOMALCOMBO3:
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
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

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(8) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(9))
			{
				Update_SwordTrails(NOMALCOMBO3);
				return;
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(6) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(7))
			{
				m_SwordTrailMatrix.r[0] = _vector{ -0.35f,0.16f,0.92f,0.f };
				m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.98f,0.17f,0.f };
				m_SwordTrailMatrix.r[2] = _vector{ 0.93f,0.06f,0.35f,0.f };
				m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
				m_fTurnSpeed = 18.f;
				m_fRenderLimit = 0.25f;
				m_fMoveSpeed = 3.f;
				m_fMoveSpeedTempo = 0.15f;
				m_eTurnDir = CMesh::TURN_FRONT;
				for (int i = 0; i < SWORDTRAIL_END; ++i)
				{
					m_SwordTrails[i]->Set_EffectMatrix(m_SwordTrailMatrix);
					m_SwordTrails[i]->Set_EffectInfo(m_fTurnSpeed, m_fRenderLimit, m_fMoveSpeed, m_fMoveSpeedTempo, m_vMoveDir, m_eTurnDir);
				}
				return;
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
			{
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				return;
			}
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
		break;
	case Client::CPlayer::NOMALCOMBO4:
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
			if (m_fNC4Speed > 0.1f)
			{
				m_fNC4Speed -= 0.1f;
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC4Speed, m_pNavigation, fTimeDelta);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(6) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(7))
				Update_SwordTrails(NOMALCOMBO4);

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
		}
		break;
	case Client::CPlayer::NOMALCOMBO5:
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
			if (m_fNC5Speed > 0.15f)
			{
				m_fNC5Speed -= 0.1f;
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC5Speed, m_pNavigation, fTimeDelta);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
				Update_SwordTrails(NOMALCOMBO5);

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
				GI->Add_GameObjectToLayer(L"PlayerRockBreak", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				CRM->Start_Shake(0.2f, 2.f, 0.02f);
			}
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
		break;
	case Client::CPlayer::NOMALCOMBO6:
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bMotionChange = false;
			Set_SwordTrailMatrix();
			if (m_fNC6Speed > 0.f)
			{
				m_fNC6Speed -= 0.15f;
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC6Speed, m_pNavigation, fTimeDelta);
			}

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
				Update_SwordTrails(NOMALCOMBO6);

			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(3))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.vScale = _float3{ 1.5f,1.5f,1.5f };
				GI->Add_GameObjectToLayer(L"PlayerRockBreak", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
				m_Parts[PARTS_SWORD]->Set_Collision(true);
				GI->PlaySoundW(L"RockBreakEnd.ogg", SD_PLAYER1, 0.6f);
				CRM->Start_Shake(0.3f, 3.f, 0.03f);
			}
			else
				m_Parts[PARTS_SWORD]->Set_Collision(false);
		}
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
		if (!m_pAnimModel[0]->GetChangeBool())
		{
			m_bCollision = false;
			m_bMotionChange = false;
			if (m_bDoubleSlashFov)
				CRM->Start_Fov(30.f, 20.f);

			m_pSwordEx->LateTick(fTimeDelta);
			_matrix WeaponHandRMatrix = m_Sockets[SOCKET_WEAPONHANDR]->Get_CombinedTransformation()* m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
			m_pSwordEx->SetUp_State(WeaponHandRMatrix);


			if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
			{

				GI->PlaySoundW(L"BladeAttack.ogg", SD_PLAYER1, 0.6f);
				GI->PlaySoundW(L"BladeVoice.ogg", SD_PLAYERVOICE, 0.9f);

			}

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
		}
		break;
	case Client::CPlayer::SLASHATTACK:
		m_bCollision = false;
		m_bMotionChange = false;
		Set_SwordTrailMatrix();
		if (m_bDoubleSlash)
		{
			CRM->Set_PlayerScene(true);
			CRM->Start_Scene("PlayerDoubleSlash");
			m_bDoubleSlash = false;
			return;
		}
		if (m_bDoubleSlashFov)
		{
			CRM->Start_Fov(30.f, 40.f);

		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
		{
			Update_SwordTrails(FASTCOMBOSTART);
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			GI->PlaySoundW(L"DoubleSlashStart.ogg", SD_PLAYER1, 0.6f);
			return;
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(5))
		{
			CRM->Set_FovSpeed(300.f);
			CRM->Set_FovDir(true);
			m_bDoubleSlashFov = false;
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
		{
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_OBB(_float3{ 4.f,4.f,4.f });
			if (!m_bDoubleSlashFov && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(5))
				m_bDoubleSlashFov = true;
		}

		else
		{
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_OBB(_float3(0.3f, 2.2f, 0.3f));
		}
		break;
	case Client::CPlayer::ROCKSHOT:
		break;
	case Client::CPlayer::EX1ATTACK:

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(1))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fEx1AttackSpeed, m_pNavigation, fTimeDelta);
			return;
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			EffectInfo.vScale = _float3{ 1.2f,1.2f,1.2f };
			GI->Add_GameObjectToLayer(L"Rock", PM->Get_NowLevel(), L"Layer_PlayerEffect", &EffectInfo);
			GI->PlaySoundW(L"RockBreakEnd.ogg", SD_PLAYER1, 0.6f);
			m_Parts[PARTS_SWORD]->Set_Collision(true);
			CRM->Start_Shake(0.4f, 6.f, 0.05f);
			CRM->Set_FovDir(true);
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_OBB(_float3{ 12.f,12.f,12.f });
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			if (m_bCharge1)
			{
				PTM->CreateParticle(L"Player1", WorldPos, false, true, CAlphaParticle::DIR_END);
				m_Parts[PARTS_SWORD]->Set_Damage(60.f);
			}
			else if (m_bCharge2)
			{
				PTM->CreateParticle(L"Player2", WorldPos, false, true, CAlphaParticle::DIR_END);
				m_Parts[PARTS_SWORD]->Set_Damage(40.f);
			}
			else
				m_Parts[PARTS_SWORD]->Set_Damage(20.f);

			Ex1AttackLight();
			return;
		}
		else
		{
			m_Parts[PARTS_SWORD]->Set_Collision(false);
			((CPlayerSword*)m_Parts[PARTS_SWORD])->Set_OBB(_float3(0.3f, 2.2f, 0.3f));
		}
		break;
	case Client::CPlayer::EX2ATTACK:
		break;
	case Client::CPlayer::EX1READY:

		break;
	case Client::CPlayer::EX2READY:
		m_bMotionChange = false;
		if (m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
		{	
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"ChargeYellow", WorldPos, false, true, CAlphaParticle::DIR_PLAYER);
		}
		else if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"ChargeOrange", WorldPos, false, true, CAlphaParticle::DIR_PLAYER);
			CRM->Start_Shake(0.2f, 1.f, 0.02f);
			m_bCharge2 = true;
		}

		else if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(4))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"ChargeOrange2", WorldPos, false, true, CAlphaParticle::DIR_PLAYER);
			CRM->Start_Shake(0.2f, 2.f, 0.03f);
			m_bCharge1 = true;
		}
		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(1) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(2))
		{
			CRM->Fix_Fov(70.f, 100.f);
			CRM->Start_Shake(0.3f, 2.5f, 0.03f);
			GI->PlaySoundW(L"ChargeReady.ogg", SD_PLAYER1, 0.6f);
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"PlayerGage2_2", WorldPos, false, true, CAlphaParticle::DIR_END);
			CreateGage(false);
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(3) && m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(4))
		{
			CRM->Fix_Fov(60.f, 100.f);
			CRM->Start_Shake(0.4f, 3.5f, 0.04f);
			GI->PlaySoundW(L"ChargeReady.ogg", SD_PLAYER1, 1.f);
			PM->Set_PlayerGage2_2(true);
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"PlayerGage2_1", WorldPos, false, true, CAlphaParticle::DIR_END);
			CreateGage(true);
		}
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

_bool CPlayer::Get_bJump()
{
	return m_pTransformCom->Get_Jump();
}

void CPlayer::Set_SwordTrailMatrix()
{
	for (int i = 0; i < SWORDTRAIL_END; ++i)
	{
		_matrix PlayerMatrix = m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
		m_SwordTrails[i]->SetUp_State(PlayerMatrix);
	}
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

	if (GI->Key_Pressing(DIK_E))
	{
		if (!UM->Get_CoolTime(1))
		{
			UM->Set_CoolTime(1);
			Set_State(EX2READY);
		}
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
			((CPlayerEx*)m_pSwordEx)->StartDissolve();
			Set_State(BLADEATTACK);		
		}
		return;
	}

	if (GI->Key_Pressing(DIK_R))
	{	
		
			Set_State(SLASHATTACK);
			UM->Reset_ExGaugeTex();	
			if (m_fNowMp >= 30.f && UM->Get_ExGaugeTex() >= 43)
			{
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

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		
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
	if (!m_pAnimModel[0]->GetChangeBool())
	{
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

		if (GI->Key_Pressing(DIK_E))
		{
			if (!UM->Get_CoolTime(1))
			{
				UM->Set_CoolTime(1);
				Set_State(EX2READY);
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
				((CPlayerEx*)m_pSwordEx)->StartDissolve();
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

		if (GI->Key_Pressing(DIK_4))
		{
		
			Set_State(ROCKBREAK);
			
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

		if (!Input_Direction())
		{
			Set_State(RUNEND);
			return;
		}
	}

}

void CPlayer::RunEnd_KeyInput(_float fTimeDelta)
{
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

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		
		return;
	}

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

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		
		return;
	}

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		
		return;
	}

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
	if (GI->Key_Pressing(DIK_C))
	{
		if (!UM->Get_CoolTime(4))
		{
			UM->Set_CoolTime(4);
			Set_State(VOIDFRONT);
			m_bSpinCombo = false;
		}
		return;
	}

	if (GI->Key_Pressing(DIK_V))
	{
		if (!UM->Get_CoolTime(4))
		{
			UM->Set_CoolTime(4);
			Set_State(VOIDBACK);
			m_bSpinCombo = false;
		}
		return;
	}

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		m_bSpinCombo = false;
		
		return;
	}

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

	if (GI->Key_Pressing(DIK_4))
	{
	
			Set_State(ROCKBREAK);
		
		return;
	}

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

	if (GI->Key_Pressing(DIK_4))
	{	
		Set_State(ROCKBREAK);	
		return;
	}

	if(m_pAnimModel[0]->GetPlayTime() <= m_pAnimModel[0]->GetTimeLimit(0))
		Input_Direction();
}

void CPlayer::RockBreak_KeyInput(_float fTimeDelta)
{
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
		CRM->Fix_Fov(30.f, 160.f);
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
	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
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

	if (GI->Key_Pressing(DIK_4))
	{

		Set_State(ROCKBREAK);
		
		return;
	}

	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(0))
	{
		if (Input_Direction())
		{
			Set_State(RUN);
			return;
		}
	}
}

void CPlayer::NomalCombo1_KeyInput(_float fTimeDelta)
{
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

	if (GI->Key_Pressing(DIK_4))
	{

		Set_State(ROCKBREAK);
		
		return;
	}

	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}

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
	}	
}

void CPlayer::NomalCombo2_KeyInput(_float fTimeDelta)
{
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

	if (GI->Key_Pressing(DIK_4))
	{

		Set_State(ROCKBREAK);
		
		return;
	}

	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}

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

	if (GI->Key_Pressing(DIK_4))
	{

		Set_State(ROCKBREAK);
		
		return;
	}
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

	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}
	
}

void CPlayer::NomalCombo4_KeyInput(_float fTimeDelta)
{
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

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		
		return;
	}

	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}

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
	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
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

	if (GI->Key_Pressing(DIK_4))
	{

		Set_State(ROCKBREAK);
		
		return;
	}


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
	if (GI->Key_Pressing(DIK_LSHIFT))
	{
		Set_State(DASH);
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

	if (GI->Key_Pressing(DIK_4))
	{
		
		Set_State(ROCKBREAK);
		
		return;
	}

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

void CPlayer::Ex1Attack_keyInput(_float fTimeDelta)
{
	if (!m_pAnimModel[0]->GetChangeBool())
	{
		if (GI->Key_Pressing(DIK_LSHIFT))
		{
			Set_State(DASH);
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

		if (GI->Key_Pressing(DIK_4))
		{

			Set_State(ROCKBREAK);

			return;
		}

		if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit(2))
		{
			if (Input_Direction())
				Set_State(RUN);

			if (GI->Key_Pressing(DIK_LSHIFT))
				Set_State(DASH);
		}
	}
}

void CPlayer::Ex1Ready_KeyInput(_float fTimeDelta)
{
}

void CPlayer::Ex2Ready_KeyInput(_float fTimeDelta)
{
	if (!m_pAnimModel[0]->GetChangeBool())
	{
		Input_Direction();

		if (GI->Mouse_Pressing(DIMK_LBUTTON))
		{
			CRM->Start_Fov(40.f, 120.f);
			_float3 BossPos = PM->Get_BossPointer()->Get_Pos();
			if (BossPos.x == 10000.f)
				m_fEx1AttackSpeed = 15.1f;
			else
				m_fEx1AttackSpeed = XMVectorGetX(XMVector4Length(XMLoadFloat3(&PM->Get_BossPointer()->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION))) * 1.f;
			Set_State(EX1ATTACK);

			CRM->Fix_Fov(30.f, 160.f);
			return;
		}
	}
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

	CHierarchyNode*		pWeaponSpineR = m_pAnimModel[MODEL_PLAYER]->Get_HierarchyNode("Weapon_Spine_R");
	if (nullptr == pWeaponSpineR)
		return E_FAIL;
	m_Sockets.push_back(pWeaponSpineR);
	
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

	CGameObject*		pTrailMain = GI->Clone_GameObject(TEXT("PlayerTrailMain"), MeshInfo);
	if (nullptr == pTrailMain)
		return E_FAIL;
	m_SwordTrails.push_back((CMesh*)pTrailMain);

	CGameObject*		pTrailSub1 = GI->Clone_GameObject(TEXT("PlayerTrailSub1"), MeshInfo);
	if (nullptr == pTrailSub1)
		return E_FAIL;
	m_SwordTrails.push_back((CMesh*)pTrailSub1);

	CGameObject*		pTrailSub2 = GI->Clone_GameObject(TEXT("PlayerTrailSub2"), MeshInfo);
	if (nullptr == pTrailSub2)
		return E_FAIL;
	m_SwordTrails.push_back((CMesh*)pTrailSub2);

	CGameObject*		pSwordEx = GI->Clone_GameObject(TEXT("PlayerEx"), MeshInfo);
	if (nullptr == pSwordEx)
		return E_FAIL;
	m_pSwordEx = ((CMesh*)pSwordEx);
	

	Safe_Delete(MeshInfo);

	return S_OK;

	
}

HRESULT CPlayer::Update_Parts()
{
	
	_matrix HeadMatxrix = m_Sockets[SOCKET_HEAD]->Get_CombinedTransformation()* m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
	
	m_Parts[PARTS_HEAD]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRBACK]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRFRONT]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRSIDE]->SetUp_State(HeadMatxrix);
	m_Parts[PARTS_HAIRTAIL]->SetUp_State(HeadMatxrix);

	if (m_bAction)
	{
		_matrix WeaponHandRMatrix = m_Sockets[SOCKET_WEAPONHANDR]->Get_CombinedTransformation()* m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
		m_Parts[PARTS_SWORD]->SetUp_State(WeaponHandRMatrix);
	}
	else
	{
		_matrix WeaponSpineRMatrix = m_Sockets[SOCKET_WEAPON_SPINE_R]->Get_CombinedTransformation()* m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
		m_Parts[PARTS_SWORD]->SetUp_State(WeaponSpineRMatrix);
	}
	return S_OK;
}

HRESULT CPlayer::Update_SwordTrails(STATE eState)
{
	for (int i = 0; i < SWORDTRAIL_END ; ++i)
	{
		_matrix PlayerMatrix = m_pAnimModel[MODEL_PLAYER]->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
		m_SwordTrails[i]->SetUp_State(PlayerMatrix);
	}
	

	switch (eState)
	{
	case Client::CPlayer::SPINCOMBOEND:
		m_SwordTrailMatrix.r[0] = _vector{ -0.58f,0.6f,0.53f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.65f,0.75f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.8f,0.44f,0.38f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 14.f;
		m_fRenderLimit = 0.2f;
		m_fMoveSpeed = 4.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		m_SwordTrailMatrix.r[0] = _vector{ -0.93f,0.f,0.34f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,1.f,0.f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ -0.34f,0.f,-0.93f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.3f,0.f,1.f };
		m_fTurnSpeed = 8.f;
		m_fRenderLimit = 0.3f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_FRONT;
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		m_SwordTrailMatrix.r[0] = _vector{ -0.58f,0.6f,0.53f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.65f,0.75f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.8f,0.44f,0.38f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 14.f;
		m_fRenderLimit = 0.2f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::FASTCOMBOEND:
		m_SwordTrailMatrix.r[0] = _vector{ -0.14f,0.65f,-0.74f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.75f,0.65f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.98f,0.09f,-0.1f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 14.f;
		m_fRenderLimit = 0.25f;
		m_fMoveSpeed = 4.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		m_SwordTrailMatrix.r[0] = _vector{ -0.14f,0.65f,-0.74f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.75f,0.65f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.98f,0.09f,-0.1f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 14.f;
		m_fRenderLimit = 0.2f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::ROCKBREAK:
		m_SwordTrailMatrix.r[0] = _vector{ -0.21f,0.96f,0.16f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.17f,0.98f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.97f,0.20f,0.03f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 14.f;
		m_fRenderLimit = 0.2f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::CHARGECRASH:
		m_SwordTrailMatrix.r[0] = _vector{ 0.7f,0.7f,0.f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.f,1.f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.7f,-0.7f,0.f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.5f,0.f,1.f };
		m_fTurnSpeed = 7.5f;
		m_fRenderLimit = 0.30f;
		m_fMoveSpeed = 4.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::CHARGEREADY:
		break;
	case Client::CPlayer::AIRCOMBO1:
		m_SwordTrailMatrix.r[0] = _vector{ 0.72f,0.68f,-0.12f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.17f,0.98f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.69f,-0.71f,0.12f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 10.f;
		m_fRenderLimit = 0.3f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::AIRCOMBO2:
		m_SwordTrailMatrix.r[0] = _vector{ -0.64f,0.72f,0.26f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.34f,0.93f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.76f,0.60f,0.21f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 10.f;
		m_fRenderLimit = 0.3f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::AIRCOMBO3:
		m_SwordTrailMatrix.r[0] = _vector{ -1.f,0.05f,0.02f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.34f,0.93f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.05f,0.93f,0.34f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 10.f;
		m_fRenderLimit = 0.3f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_FRONT;
		break;
	case Client::CPlayer::AIRCOMBO4:
		m_SwordTrailMatrix.r[0] = _vector{ -0.19f,0.98f,0.f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.f,1.f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.98f,0.19f,0.f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 10.f;
		m_fRenderLimit = 0.3f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::AIRCOMBOEND:
		break;
	case Client::CPlayer::NOMALCOMBO1:
		m_SwordTrailMatrix.r[0] = _vector{ -0.66f,-0.48f,0.57f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.76f,-0.64f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.75f,-0.42f,0.5f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.4f,0.f,1.f };
		m_fTurnSpeed = 8.f;
		m_fRenderLimit = 0.4f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;		
		m_eTurnDir = CMesh::TURN_FRONT;
		break;
	case Client::CPlayer::NOMALCOMBO2:
		m_SwordTrailMatrix.r[0] = _vector{ -0.93f,0.f,0.34f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,1.f,0.f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ -0.34f,0.f,-0.93f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 10.f;
		m_fRenderLimit = 0.4f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_FRONT;
		break;
	case Client::CPlayer::NOMALCOMBO3:
		m_SwordTrailMatrix.r[0] = _vector{ 0.37f,-0.80f,0.46f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.49f,-0.86f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.92f,0.32f,-0.18f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 15.f;
		m_fRenderLimit = 0.3f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_FRONT;
		break;
	case Client::CPlayer::NOMALCOMBO4:
		m_SwordTrailMatrix.r[0] = _vector{ 1.f,0.f,0.f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.25f,0.96f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.f,-0.96f,0.25f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 10.f;
		m_fRenderLimit = 0.9f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_FRONT;
		break;
	case Client::CPlayer::NOMALCOMBO5:
		m_SwordTrailMatrix.r[0] = _vector{ 0.7f,0.7f,0.f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.f,1.f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.7f,-0.7f,0.f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 5.5f;
		m_fRenderLimit = 0.4f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::NOMALCOMBO6:
		m_SwordTrailMatrix.r[0] = _vector{ 0.7f,0.7f,0.f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,0.f,1.f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.7f,-0.7f,0.f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.5f,0.f,1.f };
		m_fTurnSpeed = 7.5f;
		m_fRenderLimit = 0.30f;
		m_fMoveSpeed = 3.f;
		m_fMoveSpeedTempo = 0.15f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::BLADEATTACK:
		break;
	case Client::CPlayer::SLASHATTACK:
		break;
	case Client::CPlayer::EX1ATTACK:
		break;
	case Client::CPlayer::EX2ATTACK:
		break;
	case Client::CPlayer::EX1READY:
		//FastCombo2
		m_SwordTrailMatrix.r[0] = _vector{ -0.58f,0.6f,0.53f,0.f };
		m_SwordTrailMatrix.r[1] = _vector{ 0.f,-0.65f,0.75f,0.f };
		m_SwordTrailMatrix.r[2] = _vector{ 0.8f,0.44f,0.38f,0.f };
		m_SwordTrailMatrix.r[3] = _vector{ 0.f,1.f,0.f,1.f };
		m_fTurnSpeed = 14.f;
		m_fRenderLimit = 0.2f;
		m_fMoveSpeed = 0.f;
		m_fMoveSpeedTempo = 0.f;
		m_eTurnDir = CMesh::TURN_BACK;
		break;
	case Client::CPlayer::EX2READY:
		break;
	case Client::CPlayer::STATE_END:
		break;
	}
	
	for (int i = 0; i < SWORDTRAIL_END; ++i)
	{
		m_SwordTrails[i]->Set_EffectMatrix(m_SwordTrailMatrix);
		m_SwordTrails[i]->Set_EffectInfo(m_fTurnSpeed, m_fRenderLimit, m_fMoveSpeed, m_fMoveSpeedTempo, m_vMoveDir, m_eTurnDir);
	}


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

	for (auto& pSwordTrail : m_SwordTrails)
		Safe_Release(pSwordTrail);

	Safe_Release(m_pSwordEx);

	m_Parts.clear();
	Safe_Release(m_pNavigation);

}
