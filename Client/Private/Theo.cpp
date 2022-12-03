#include "stdafx.h"
#include "..\Public\Theo.h"
#include "GameInstance.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "HierarchyNode.h"
#include "UI.h"
#include "UI_Manager.h"
#include "Pointer_Manager.h"
#include "Player.h"
#include "Navigation.h"
#include "Level_Loading.h"
#include "GolemRock1.h"
#include "GolemRock3.h"
#include "Rock.h"
#include "Particle_Manager.h"
#include "Ring.h"
#include "Wall.h"
#include "PlayerLight.h"

CTheo::CTheo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	
}

CTheo::CTheo(const CTheo& rhs)
	:CAnimMesh(rhs)
{
}

HRESULT CTheo::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CTheo::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);
	sTag = m_MeshInfo->sTag;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Theo"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	Ready_Sockets();

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	m_bColliderRender = false;
	m_pTransformCom->Rotation(_vector{ 0.f,1.f,0.f }, 180.f);

	m_eCurState = APPEAR;
	m_eNextState = APPEAR;
	m_vTargetLook = { 0.f,0.f,1.f };
	m_fOutLinePower = 3.f;
	m_pAnimModel->Set_AnimIndex(m_eCurState);

	m_fMaxHp = 500;
	m_fMaxMp = 100.f;
	m_fNowHp = m_fMaxHp;
	m_fNowMp = 95.f;
	m_fDamage = 10.f;

	m_fColiisionTime = 0.06f;

	m_pTarget = PM->Get_PlayerPointer();
	Safe_AddRef(m_pTarget);
	PM->Add_Boss(this);

	/*CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 1;
	if (FAILED(__super::Add_Component(LEVEL_STAGE1, L"NavigationStage1", TEXT("NavigationStage1"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;*/

	CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 42;
	if (FAILED(__super::Add_Component(LEVEL_STAGE2, L"NavigationStage2", TEXT("NavigationStage2"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;

	m_pNavigation->Set_BattleIndex(41);

	CRM->Start_Scene("Scene_Stage2Boss");

	UM->Add_Boss(this);
	Load_UI("BossBar");

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Noise"), TEXT("Com_Texture"), (CComponent**)&m_pDissolveTexture)))
		return E_FAIL;

	return S_OK;
}

void CTheo::Tick(_float fTimeDelta)
{
	if (m_fNowHp <= 0)
		m_fNowHp = 0;

	if (m_fNowMp >= 100.f)
		m_fNowMp = 100.f;

	if (m_fNowHp >= m_fMaxHp)
		m_fNowHp = m_fMaxHp;

	if (!m_pAnimModel->GetChangeBool())
		m_eCurState = m_eNextState;

	if (GI->Key_Down(DIK_0))
		m_bColliderRender = !m_bColliderRender;

	DebugKeyInput();

	if (!m_bCollision)
	{
		m_fCollisionAcc += 1.f * fTimeDelta;
		if (m_fCollisionAcc >= m_fColiisionTime)
		{
			m_fCollisionAcc = 0.f;
			m_bCollision = true;
		}
	}

	if (m_bHit)
	{
		m_fHitAcc += 1.f * fTimeDelta;
		if (m_fHitAcc >= 0.2f)
		{
			m_fHitAcc = 0.f;
			m_bHit = false;
		}
	}

	Update(fTimeDelta);

	if (m_bDissolve)
		m_fDissolveAcc += 0.2f * fTimeDelta;
}

void CTheo::LateTick(_float fTimeDelta)
{
	if (m_fNowHp <= 0 && !m_bDie)
	{
		Set_State(DOWN);
		m_bDie = true;
		m_bPattern = false;
		m_bCollision = false;
		m_bRHand = false;
		m_bLHand = false;
		GI->PlaySoundW(L"TheoDie.ogg", SD_MONSTERVOICE, 0.9f);
	}

	if (m_fDissolveAcc >= 0.8f)
	{
		PM->Delete_Boss();
		UM->On_Fade();
		UM->Set_Fade(CFadeInOut::FADEOUT);
		PM->Set_NextLevel(LEVEL_STAGE3);
		Set_Dead();
	}

	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMLoadFloat3(&m_vTargetLook), 0.1f);

	m_pAnimModel->Play_Animation(fTimeDelta, m_pAnimModel);

	End_Animation();

	m_pOBB[OBB_BODY]->Update(m_pTransformCom->Get_WorldMatrix());

	_matrix LHand = m_Sockets[SOCKET_LHAND]->Get_CombinedTransformation() * m_pAnimModel->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
	LHand.r[0] = XMVector3Normalize(LHand.r[0]);
	LHand.r[1] = XMVector3Normalize(LHand.r[1]);
	LHand.r[2] = XMVector3Normalize(LHand.r[2]);
	m_pOBB[OBB_LHAND]->Update(LHand);

	_matrix RHand = m_Sockets[SOCKET_RHAND]->Get_CombinedTransformation() * m_pAnimModel->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
	RHand.r[0] = XMVector3Normalize(RHand.r[0]);
	RHand.r[1] = XMVector3Normalize(RHand.r[1]);
	RHand.r[2] = XMVector3Normalize(RHand.r[2]);
	m_pOBB[OBB_RHAND]->Update(RHand);

	if (!m_bDie)
	{
		if (m_bCollision)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTER, this, m_pOBB[OBB_BODY]);

		if (m_bLHand)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB[OBB_LHAND]);

		if (m_bRHand)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB[OBB_RHAND]);
	}
	if(!m_bDissolve)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CTheo::Render()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	_uint		iNumMeshes = m_pAnimModel->Get_NumMeshes();
	for (_uint j = 0; j < iNumMeshes; ++j)
	{
		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(j), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (!m_bDissolve)
		{
			if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrixInverse", &m_pTransformCom->Get_WorldMatrixInverse(), sizeof(_float4x4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrixInverse", &GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
				return E_FAIL;

			m_pShaderCom->Set_RawValue("g_fOutLinePower", &m_fOutLinePower, sizeof(_float));

			if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, OUTLINEPASS)))
				return E_FAIL;
		}

		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(j), TEX_NORMALS, "g_NormalTexture")))
		{

			if (m_bDissolve)
			{
				m_pDissolveTexture->Set_SRV(m_pShaderCom, "g_DissolveTexture", 0);
				m_pShaderCom->Set_RawValue("g_fDissolveAcc", &m_fDissolveAcc, sizeof(float));
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_DISSOLVE)))
					return E_FAIL;
			}

			else if (m_bPattern)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NPATTERN)))
					return E_FAIL;

				if (m_bHit)
				{
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

					if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_PATTERNHIT)))
						return E_FAIL;
				}
			}

			else if (m_bFinish)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NFINISH)))
					return E_FAIL;
				if (m_bHit)
				{
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

					if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_FINISHHIT)))
						return E_FAIL;
				}
			}

			else
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NDEFAULT)))
					return E_FAIL;

				if (m_bHit)
				{
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

					if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NHIT)))
						return E_FAIL;
				}
			}
		}
		else
		{

			if (m_bDissolve)
			{
				m_pDissolveTexture->Set_SRV(m_pShaderCom, "g_DissolveTexture", 0);
				m_pShaderCom->Set_RawValue("g_fDissolveAcc", &m_fDissolveAcc, sizeof(float));
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_DISSOLVE)))
					return E_FAIL;
			}

			else if (m_bPattern)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_PATTERN)))
					return E_FAIL;

				if (m_bHit)
				{
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

					if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_PATTERNHIT)))
						return E_FAIL;
				}
			}

			else if (m_bFinish)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_FINISH)))
					return E_FAIL;
				if (m_bHit)
				{
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

					if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_FINISHHIT)))
						return E_FAIL;
				}
			}

			else
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_DEFAULT)))
					return E_FAIL;

				if (m_bHit)
				{
					m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

					if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NHIT)))
						return E_FAIL;
				}
			}
		}

		
	}


	for (int i = 0; i < OBB_END; ++i)
	{
		if(m_bColliderRender)
			m_pOBB[i]->Render();
	}
	return S_OK;
}

HRESULT CTheo::Render_ShadowDepth()
{
	_matrix		LightViewMatrix;
	LightViewMatrix = XMMatrixTranspose(GI->Get_LightMatrix());

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_LightViewMatrix", &LightViewMatrix, sizeof(_float4x4))))
		return E_FAIL;

	_matrix Fov60 = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (_float)1280.f / 720.f, 0.2f, 300.f);

	if (FAILED(m_pShaderCom->Set_RawValue("g_LightProjMatrix", &XMMatrixTranspose(Fov60), sizeof(_float4x4))))
		return E_FAIL;

	for (int i = 0; i < MODEL_END; ++i)
	{
		if (m_pAnimModel != nullptr)
		{
			_uint		iNumMeshes = m_pAnimModel->Get_NumMeshes();
			for (_uint j = 0; j < iNumMeshes; ++j)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_SHADOW)))
					return E_FAIL;
			}
		}
	}


	return S_OK;
}

void CTheo::Collision(CGameObject * pOther, string sTag)
{
	if (sTag == "Player_Body")
	{
		if (m_bFinish)
		{
			if (UM->Get_Count() > 0)
				UM->Set_Count(-1);
			else
			{
				m_bFinishStart = true;
				m_bFinish = false;
				m_fNowMp = 0.f;
			}
		}
		else
		{
			if (m_eCurState != SKILL3)
				m_fNowMp += m_fDamage / 2.f;

			if (m_fNowMp >= 100.f)
			{
				UM->Set_Count(2);
				m_bFinish = true;
			}
			else
			{
				m_bFinish = false;
			}
		}
	}

	if (sTag == "Player_Sword")
	{
		
		if (pOther->Can_Hit())
		{
			if (m_bPattern && pOther->Get_Damage() == 1.f)
			{
				m_bFinish = false;
				if (!m_bFinish)
					UM->Set_Count(2);
				GI->PlaySoundW(L"TheoGroggy.ogg", SD_MONSTERVOICE, 0.9f);
				m_bPattern = false;
				m_bLHand = false;
				m_bRHand = false;
				Set_State(HITSTART);
				m_fNowMp -= 10.f;
				CRM->Start_Shake(0.5f, 5.f, 0.06f);
				CRM->Start_Fov(40.f, 120.f);
				CRM->Set_FovDir(true);
			}
			if (pOther->Get_Damage() == 63.f || pOther->Get_Damage() == 73.f || pOther->Get_Damage() == 5.5f)
			{
				m_bPattern = false;
				m_bLHand = false;
				m_bRHand = false;
				GI->PlaySoundW(L"TheoGroggy.ogg", SD_MONSTERVOICE, 0.9f);
				Set_State(HITSTART);
				CRM->Start_Shake(0.5f, 5.f, 0.06f);
				CRM->Start_Fov(40.f, 120.f);
				CRM->Set_FovDir(true);
			}
			m_bCollision = false;
			m_bHit = true;
			if (m_eCurState == HITSTART || m_eCurState == HITLOOF)
				m_fNowHp -= pOther->Get_Damage() * 2;
			else
				m_fNowHp -= pOther->Get_Damage();
			UM->Set_ExGaugeTex(2);
		}
	}
}

HRESULT CTheo::Ready_Collider()
{
	CCollider::COLLIDERDESC		ColliderDesc;

	ColliderDesc.vSize = _float3(6.f, 6.f, 6.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 1.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Body";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Body"), (CComponent**)&m_pOBB[OBB_BODY], &ColliderDesc)))
		return E_FAIL;

	_float4x4 LHand = m_Sockets[SOCKET_LHAND]->Get_Transformation();

	//ColliderDesc.vSize = _float3(2.f, 1.f, 1.f);
	ColliderDesc.vSize = _float3(6.f, 6.f, 6.f);
	ColliderDesc.vCenter = _float3(LHand._41, LHand._42, LHand._43);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Attack";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_LHand"), (CComponent**)&m_pOBB[OBB_LHAND], &ColliderDesc)))
		return E_FAIL;


	_float4x4 RHand = m_Sockets[SOCKET_RHAND]->Get_Transformation();

	ColliderDesc.vSize = _float3(6.f, 6.f, 6.f);
	ColliderDesc.vCenter = _float3(RHand._41, RHand._42, RHand._43);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Attack";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_RHand"), (CComponent**)&m_pOBB[OBB_RHAND], &ColliderDesc)))
		return E_FAIL;


	return S_OK;
}

void CTheo::Set_NextMotion()
{
	if (!m_pAnimModel->GetChangeBool())
	{
		m_iPreMotion = m_iNextMotion;
		m_iNextMotion = GI->Get_Random(1, 3);

		while (m_iNextMotion == m_iPreMotion)
		{
			m_iNextMotion = GI->Get_Random(1, 3);
			if (m_iNextMotion != m_iPreMotion)
				break;
		}
		if (m_iNextMotion == 1)
		{
			Set_State(IDLE);
			return;
		}
		if (m_iNextMotion == 2)
		{
			Set_State(WALK);
			return;
		}
		if (m_iNextMotion == 3)
		{
			Set_State(RUN);
			return;
		}
		
	}
}

void CTheo::Set_NextAttack()
{
	if (!m_pAnimModel->GetChangeBool())
	{
		m_iPreAttack = m_iNextAttack;
		m_iNextAttack = GI->Get_Random(1, 5);

		while (m_iNextAttack == m_iPreAttack)
		{
			m_iNextAttack = GI->Get_Random(1, 5);
			if (m_iNextAttack != m_iPreAttack)
				break;
		}
		if (m_iNextAttack == 1)
		{
			Set_State(SKILL1);
			return;
		}
		if (m_iNextAttack == 2)
		{
			Set_State(SKILL2);
			return;
		}
		if (m_iNextAttack == 3)
		{
			Set_State(SKILL4);
			return;
		}
		if (m_iNextAttack == 4)
		{
			Set_State(SKILL5);
			return;
		}
		if (m_iNextAttack == 5)
		{
			Set_State(SKILL6);
			return;
		}
	}
}

void CTheo::DebugKeyInput()
{
	if (GI->Key_Down(DIK_NUMPAD0))
		Set_State(APPEAR);

	if (GI->Key_Down(DIK_NUMPAD1))
		Set_State(SKILL1);

	if (GI->Key_Down(DIK_NUMPAD2))
		Set_State(SKILL2);

	if (GI->Key_Down(DIK_NUMPAD3))
		Set_State(SKILL3);

	if (GI->Key_Down(DIK_NUMPAD4))
		Set_State(SKILL4);

	if (GI->Key_Down(DIK_NUMPAD5))
		Set_State(SKILL5);

	if (GI->Key_Down(DIK_NUMPAD6))
		Set_State(SKILL6);

	if (GI->Key_Down(DIK_NUMPAD7))
		Set_State(DOWN);

	if (GI->Key_Down(DIK_NUMPAD8))
		Set_State(APPEAR);
}

void CTheo::Set_State(STATE eState)
{
	if (m_eNextState == eState)
		return;

	m_eNextState = eState;

	switch (m_eNextState)
	{
	case Client::CTheo::DOWN:
		break;
	case Client::CTheo::HITEND:
		
		break;
	case Client::CTheo::HITLOOF:
		break;
	case Client::CTheo::HITSTART:
		m_bPattern = false;
		break;
	case Client::CTheo::RUN:
		break;
	case Client::CTheo::SKILL1:
		m_fDamage = 15.f;	
		break;
	case Client::CTheo::SKILL2:
			m_fDamage = 20.f;
		break;
	case Client::CTheo::SKILL3:
		m_pOBB[OBB_RHAND]->ChangeExtents(_float3{ 300.f, 300.f, 300.f });
		break;
	case Client::CTheo::SKILL4:
			m_fDamage = 30.f;
		break;
	case Client::CTheo::SKILL5:
			m_fDamage = 20.f;
			m_bRushRight = true;
			m_pOBB[OBB_RHAND]->ChangeExtents(_float3{ 6.f, 6.f, 6.f });
			GI->PlaySoundW(L"TheoSkill5.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CTheo::SKILL6:
		break;
	case Client::CTheo::APPEAR:
		break;
	case Client::CTheo::IDLE:
		break;
	case Client::CTheo::WALKBACK:
		break;
	case Client::CTheo::WALK:		
		break;
	case Client::CTheo::FINISH:
		m_eNextState = SKILL6;
		m_fDamage = 110.f;
		Set_Dir();
		break;
	}

	m_pAnimModel->SetNextIndex(m_eNextState);
	m_pAnimModel->SetChangeBool(true);
}

void CTheo::Set_Dir()
{
	XMStoreFloat3(&m_vTargetLook, XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

void CTheo::End_Animation()
{
	if (m_pAnimModel->GetAniEnd())
	{
		if (m_bFinishStart)
		{			
			Set_State(FINISH);
			return;
		}
		switch (m_eCurState)
		{
		case Client::CTheo::DOWN:
			
			//GI->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_STAGE3));
			break;
		case Client::CTheo::HITEND:
			Set_State(IDLE);
			break;
		case Client::CTheo::HITLOOF:
			if (m_bDie)
				Set_State(DOWN);
			else
				Set_State(HITEND);
			break;
		case Client::CTheo::HITSTART:
			Set_State(HITLOOF);
			break;
		case Client::CTheo::RUN:
			break;
		case Client::CTheo::SKILL1:
			Set_NextMotion();
			break;
		case Client::CTheo::SKILL2:
			Set_NextMotion();
			break;
		case Client::CTheo::SKILL3:
			m_fDamage = 10.f;
			m_bRHand = false;
			m_pOBB[OBB_RHAND]->ChangeExtents(_float3{ 6.f, 6.f, 6.f });
			Set_NextMotion();
			break;
		case Client::CTheo::SKILL4:
			Set_NextMotion();
			break;
		case Client::CTheo::SKILL5:
			Set_NextMotion();
			m_fRushShakeAcc = 0.25f;
			break;
		case Client::CTheo::SKILL6:
			Set_NextMotion();
			if (!m_bFinish)
				UM->Set_Count(2);
			break;
		case Client::CTheo::APPEAR:
			Set_State(RUN);
			break;
		case Client::CTheo::IDLE:
			Set_State(WALK);
			break;
		case Client::CTheo::WALKBACK:
			Set_NextMotion();
			break;
		case Client::CTheo::WALK:
			Set_State(RUN);
			break;
		}
	}
	
}

void CTheo::Update(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CTheo::DOWN:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			m_bDissolve = true;
		}
		break;
	case Client::CTheo::HITEND:
		break;
	case Client::CTheo::HITLOOF:
		break;
	case Client::CTheo::HITSTART:
		
		break;
	case Client::CTheo::RUN:
	{
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				CRM->Start_Shake(0.3f, 2.5f, 0.03f);
				GI->PlaySoundW(L"TheoRun.ogg", SD_MONSTER1, 1.f);
			}
		}
		m_fRunSpeed = 7.f;
		Set_Dir(); 
		_float Distance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
		if (Distance < 5.f)
			Set_NextAttack();
		else
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRunSpeed, m_pNavigation, fTimeDelta);
		break;
	}
	case Client::CTheo::SKILL1:
		m_bLHand = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
				m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 8.f, m_pNavigation, fTimeDelta);
			
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
				EffectInfo.WorldMatrix.r[3] += Look * 2.f;
				_float4 Pos;
				XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
				PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
				CRing::RINGINFO RingInfo;
				RingInfo.vSize = { 0.2f,0.3f,0.2f };
				RingInfo.vSpeed = _float3{ 0.6f, 0.f, 0.6f };
				RingInfo.fLifeTime = 0.3f;
				RingInfo.eColor = CRing::RING_BLUE;
				RingInfo.vWorldPos = Pos;
				RingInfo.vWorldPos.y += 0.5f;
				GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_TheoEffect", &RingInfo);
				EffectInfo.WorldMatrix.r[3] += Right * 2.f;
				EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
				GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
				m_bLHand = true;
				GI->PlaySoundW(L"TheoAttack1.ogg", SD_MONSTER1, 1.f);
				GI->PlaySoundW(L"TheoSkill1.ogg", SD_MONSTERVOICE, 0.9f);
				CRM->Start_Shake(0.3f, 3.f, 0.04f);
				return;
			}
		}

		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				m_bPattern = true;
			}
			else
				m_bPattern = false;
		}
		break;
	case Client::CTheo::SKILL2:
		m_bPattern = false;
		m_bRHand = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(6) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(7))
		{
			GI->PlaySoundW(L"TheoSkill2.ogg", SD_MONSTERVOICE, 0.9f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 4.f, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			Set_Dir();
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.5f;
			_float4 Pos;
			XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
			CRing::RINGINFO RingInfo;
			RingInfo.vSize = { 0.2f,0.3f,0.2f };
			RingInfo.vSpeed = _float3{ 0.6f, 0.f, 0.6f };
			RingInfo.fLifeTime = 0.3f;
			RingInfo.eColor = CRing::RING_BLUE;
			RingInfo.vWorldPos = Pos;
			RingInfo.vWorldPos.y += 0.5f;
			GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_TheoEffect", &RingInfo);
			EffectInfo.WorldMatrix.r[3] += Right * 2.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
			m_bRHand = true;
			GI->PlaySoundW(L"TheoAttack1.ogg", SD_MONSTER1, 1.f);
			CRM->Start_Shake(0.3f, 4.f, 0.04f);
			return;
		}

		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
				m_bPattern = true;
		}


		break;
	case Client::CTheo::SKILL3:
		m_bCollision = false;
		m_bRHand = false;
		m_bLHand = false;
		m_bPattern = false;
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			Set_Dir();
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.f;
			_float4 Pos;
			XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
			CRing::RINGINFO RingInfo;
			RingInfo.vSize = { 0.2f,0.6f,0.2f };
			RingInfo.vSpeed = _float3{ 1.5f, 0.f, 1.5f };
			RingInfo.fLifeTime = 0.5f;
			RingInfo.eColor = CRing::RING_BLUE;
			RingInfo.vWorldPos = Pos;
			RingInfo.vWorldPos.y += 0.5f;
			GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_TheoEffect", &RingInfo);
			EffectInfo.WorldMatrix.r[3] += Right * 2.5f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
		
			m_fDamage = 30.f;
			m_bRHand = true;
			GI->PlaySoundW(L"TheoAttack1.ogg", SD_MONSTER1, 1.f);
			GI->PlaySoundW(L"TheoAppearAttack.ogg", SD_MONSTERVOICE, 0.9f);
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 4.f, m_pNavigation, fTimeDelta);
			CRM->Start_Shake(0.3f, 3.f, 0.03f);
			m_fRushSpeed = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION))) * 0.65f;
			return;
		}		
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{		
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"TheoTrail", WorldPos, false, CAlphaParticle::DIR_END);
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRushSpeed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			EffectInfo.WorldMatrix.r[3] += Look * 3.f;
			EffectInfo.vScale = _float3{ 3.f,1.f,3.f };
			GI->Add_GameObjectToLayer(L"Rock", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
			m_fDamage = 110.f;
			m_bRHand = true;
			GI->PlaySoundW(L"TheoAttack3.ogg", SD_MONSTER1, 1.f);
			GI->PlaySoundW(L"TheoFinish.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.4f, 6.f, 0.05f);

			

			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, EffectInfo.WorldMatrix.r[3]);
			CreateLight(WorldPos);

			PTM->CreateParticle(L"TheoFinish", WorldPos, false, CAlphaParticle::DIR_END);

			PTM->CreateParticle(L"TheoFinish2", WorldPos, false, CAlphaParticle::DIR_END);
			CRing::RINGINFO RingInfo;
			RingInfo.vSize = { 0.3f,1.f,0.3f };
			RingInfo.vSpeed = _float3{ 1.5f, 0.f, 1.5f };
			RingInfo.fLifeTime = 0.2f;
			RingInfo.eColor = CRing::RING_BLUE;
			RingInfo.vWorldPos = WorldPos;
			RingInfo.vWorldPos.y += 1.f;
			GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_TheoEffect", &RingInfo);

			CWall::WALLINFO WallInfo;
			WallInfo.fMaxUVIndexX = 1.f;
			WallInfo.fMaxUVIndexY = 4.f;
			WallInfo.fUVSpeed = 0.05f;
			WallInfo.vSize = { 3.f,10.f,3.f };
			WallInfo.vSpeed = { 0.2f,0.f,0.2f };
			WallInfo.eColor = CWall::WALL_BLUE;
			WallInfo.fLifeTime = 0.5f;
			WallInfo.fEndSpeed = 0.5f;
			WallInfo.vWorldPos = WorldPos;
			GI->Add_GameObjectToLayer(L"Wall", PM->Get_NowLevel(), L"Layer_TheoEffect", &WallInfo);
			return;
		}
		break;
	case Client::CTheo::SKILL4:
		m_bRHand = false;
		m_bPattern = false;
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(7))
			Set_Dir();
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			GI->PlaySoundW(L"TheoAttack4.ogg", SD_MONSTER1, 1.f);
			GI->PlaySoundW(L"TheoSkill4.ogg", SD_MONSTERVOICE, 0.9f);
			m_bRHand = true;
			return;
		}
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				m_bPattern = true;
				return;
			}
		}

		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(5))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"TheoCharge", WorldPos, false, CAlphaParticle::DIR_THEO);
			return;
		}

		
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
		{
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			WorldPos.y += 3.f;
			PTM->CreateParticle(L"TheoSkill6", WorldPos, false, CAlphaParticle::DIR_END);
			GI->PlaySoundW(L"TheoSkill4_2.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.4f, 5.f, 0.05f);
			return;
		}
		break;
	case Client::CTheo::SKILL5:	
		m_bLHand = false;
		m_bRHand = false;
		m_bPattern = false;
		if (!m_pAnimModel->GetChangeBool())
		{			
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				m_bPattern = true;
				return;
			}
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			Set_Dir();
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			m_fRushShakeAcc += 1.f * fTimeDelta;
			if (m_fRushShakeAcc >= m_fRushTempo)
			{
				m_fRushShakeAcc = 0.f;
				GI->PlaySoundW(L"TheoRun.ogg", SD_MONSTER1, 1.f);
				CRM->Start_Shake(0.3f, 3.f, 0.04f);
				if (m_bRushRight)
				{
					CAnimMesh::EFFECTINFO EffectInfo;
					EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
					_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
					_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
					EffectInfo.WorldMatrix.r[3] += Look * 2.f;
					_float4 Pos;
					XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
					PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
					EffectInfo.WorldMatrix.r[3] += Right * 1.5f;
					EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
					GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
					m_bRushRight = false;
				}
				else
				{
					CAnimMesh::EFFECTINFO EffectInfo;
					EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
					_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
					_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
					EffectInfo.WorldMatrix.r[3] += Look * 2.f;
					_float4 Pos;
					XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
					PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
					EffectInfo.WorldMatrix.r[3] += Right * -1.f;
					EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
					GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
					m_bRushRight = true;
				}
			}
			if (m_pTransformCom->Go_NoSlide(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRushSpeed*1.8f, m_pNavigation, fTimeDelta))
				Set_Dir();
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"TheoTrail2", WorldPos, false, CAlphaParticle::DIR_END);
			m_bLHand = true;
			m_bRHand = true;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(4) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(5))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 2.f;
			_float4 Pos;
			XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
			EffectInfo.WorldMatrix.r[3] += Right * 1.5f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_TheoEffect", &EffectInfo);
			GI->PlaySoundW(L"TheoRun.ogg", SD_MONSTER1, 1.f);
			CRM->Start_Shake(0.3f, 3.f, 0.04f);
			if (m_pTransformCom->Go_NoSlide(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRushSpeed*1.3f, m_pNavigation, fTimeDelta))
				Set_Dir();
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			PTM->CreateParticle(L"TheoTrail2", WorldPos, false, CAlphaParticle::DIR_END);
			m_bLHand = true;
			m_bRHand = true;
			
		}

		break;
	case Client::CTheo::SKILL6:
		m_bPattern = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				if(!m_bFinishStart)
					m_bPattern = true;
			}
			
		}
		if (!m_bFinishStart)
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				PTM->CreateParticle(L"TheoCharge", WorldPos, false, CAlphaParticle::DIR_THEO);
				return;
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				m_fNowHp += 50.f;
				m_fNowMp += 3.f;
				if (m_fNowMp >= 100.f)
					m_bFinish = true;			
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				WorldPos.y += 3.f;
				PTM->CreateParticle(L"TheoSkill6", WorldPos, false, CAlphaParticle::DIR_END);
				GI->PlaySoundW(L"TheoSkill6.ogg", SD_MONSTERVOICE, 0.9f);
				CRM->Start_Shake(0.6f, 5.f, 0.05f);
			}
		}
		else
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				WorldPos.y += 2.f;
				PTM->CreateParticle(L"TheoCharge", WorldPos, false, CAlphaParticle::DIR_THEO);
				return;
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				WorldPos.y += 3.f;
				PTM->CreateParticle(L"TheoSkill6", WorldPos, false, CAlphaParticle::DIR_END);
				GI->PlaySoundW(L"TheoSkill6.ogg", SD_MONSTERVOICE, 0.9f);
				CRM->Start_Shake(0.6f, 5.f, 0.05f);
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3))
			{
			
				m_bFinishStart = false;
				Set_State(SKILL3);
			}
			
		}
		break;
	case Client::CTheo::APPEAR:
		if (!m_pAnimModel->GetChangeBool())
		{
			Set_Dir();
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
				EffectInfo.WorldMatrix.r[3] += Look * 2.f;
				EffectInfo.WorldMatrix.r[3] += Right * 2.f;
				_float4 Pos;
				XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
				PTM->CreateParticle(L"TheoSkill1", Pos, false, CAlphaParticle::DIR_END);
				CRing::RINGINFO RingInfo;
				RingInfo.vSize = { 0.2f,0.3f,0.2f };
				RingInfo.vSpeed = _float3{ 0.7f, 0.f, 0.7f };
				RingInfo.fLifeTime = 0.5f;
				RingInfo.eColor = CRing::RING_BLUE;
				RingInfo.vWorldPos = Pos;
				RingInfo.vWorldPos.y += 0.5f;
				GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_PlayerEffect", &RingInfo);
				EffectInfo.WorldMatrix.r[3] += Right * 2.f;
				EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
				GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
				GI->PlaySoundW(L"TheoAppearAttack.ogg", SD_MONSTERVOICE, 0.9f);
				//CRM->Start_Shake(0.2f, 3.f, 0.03f);
				return;
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
				GI->PlaySoundW(L"TheoAppear.ogg", SD_MONSTERVOICE, 0.9f);
		
		}
		break;
	case Client::CTheo::IDLE:
		break;
	case Client::CTheo::WALKBACK:
		break;
	case Client::CTheo::WALK:
	{
		if (!m_pAnimModel->GetChangeBool())
		{
			m_fWalkSpeed = 5.f;
			Set_Dir();
			_float Distance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
			if (Distance < 5.f)
				Set_State(RUN);
			else
				m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fWalkSpeed, m_pNavigation, fTimeDelta);
		}
		break;
	}
	}
}

HRESULT CTheo::Ready_Sockets()
{
	CHierarchyNode*		Bip001_L_Hand = m_pAnimModel->Get_HierarchyNode("Bip001-L-Hand");
	if (nullptr == Bip001_L_Hand)
		return E_FAIL;
	m_Sockets.push_back(Bip001_L_Hand);

	CHierarchyNode*		Bip001_R_Hand = m_pAnimModel->Get_HierarchyNode("Bip001-R-Hand");
	if (nullptr == Bip001_R_Hand)
		return E_FAIL;
	m_Sockets.push_back(Bip001_R_Hand);

	return S_OK;
}

HRESULT CTheo::Load_UI(char* DatName)
{
	string FileSave = DatName;

	string temp = "../Data/UIData/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return E_FAIL;
	}

	DWORD		dwByte = 0;

	while (true)
	{
		_tchar temp[256];
		_tchar temp2[256];
		ReadFile(hFile, temp, sizeof(_tchar) * 256, &dwByte, nullptr);
		ReadFile(hFile, temp2, sizeof(_tchar) * 256, &dwByte, nullptr);
		_tchar* UIPath = new _tchar[256];
		_tchar* UIName = new _tchar[256];
		for (int i = 0; i < 256; ++i)
		{
			UIPath[i] = temp[i];
			UIName[i] = temp2[i];
		}
		int UITexNum;
		int UIIndex;
		_float UIPosX;
		_float UIPosY;
		_float UISizeX;
		_float UISizeY;
		ReadFile(hFile, &UITexNum, sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &UIIndex, sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &UIPosX, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UIPosY, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UISizeX, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UISizeY, sizeof(_float), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete(UIPath);
			Safe_Delete(UIName);
			break;
		}

		CUI::UIINFO* UIInfo;
		UIInfo = new CUI::UIINFO;
		UIInfo->eLevel = LEVEL_STATIC;
		UIInfo->TexPath = UIPath;
		UIInfo->TexName = UIName;
		UIInfo->TexNum = UITexNum;
		UIInfo->UIIndex = UIIndex;
		UIInfo->UIPosX = UIPosX;
		UIInfo->UIPosY = UIPosY;
		UIInfo->UISizeX = UISizeX;
		UIInfo->UISizeY = UISizeY;

		if (FAILED(GI->Add_GameObjectToLayer(UIName, LEVEL_STATIC, L"Layer_UI", &UIInfo)))
		{
			wstring a = L"Please Load ProtoType";
			wstring b = a + UIName;
			const _tchar* c = b.c_str();
			MSG_BOX(c);

			Safe_Delete_Array(UIPath);
			Safe_Delete_Array(UIName);
			Safe_Delete(UIInfo);
			return E_FAIL;
		}

		Safe_Delete(UIInfo);
		Safe_Delete(UIPath);
		Safe_Delete(UIName);
	}

	CloseHandle(hFile);

	return S_OK;
}

void CTheo::CreateLight(_float4 WorldPos)
{
	CPlayerLight::PLAYERLIGHT PlayerLightInfo;
	PlayerLightInfo.vPos = WorldPos;
	PlayerLightInfo.fCloseSpeed = 1.f;
	PlayerLightInfo.vScale = { 5.f,10.f,5.f };
	PlayerLightInfo.vAngle = { 0.f,0.f,0.f };
	GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);


	for (int i = 0; i < 15; ++i)
	{
		PlayerLightInfo.vPos = WorldPos;
		PlayerLightInfo.fCloseSpeed = GI->Get_FloatRandom(0.05f, 0.1f);
		PlayerLightInfo.vScale = { GI->Get_FloatRandom(1.f,2.f),10.f, GI->Get_FloatRandom(1.f,2.f) };
		PlayerLightInfo.vAngle = { GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f) };
		GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);
	}
}

CTheo * CTheo::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTheo*		pInstance = new CTheo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTheo"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTheo::Clone(void * pArg)
{
	CTheo*		pInstance = new CTheo(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTheo"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTheo::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel);
	Safe_Release(m_pDissolveTexture);
	Safe_Release(m_pTarget);
	Safe_Release(m_pNavigation);

	for(auto& iter : m_pOBB)
	Safe_Release(iter);
}
