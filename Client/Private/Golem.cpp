#include "stdafx.h"
#include "..\Public\Golem.h"
#include "GameInstance.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "HierarchyNode.h"
#include "UI.h"
#include "UI_Manager.h"
#include "Pointer_Manager.h"
#include "Player.h"
#include "Level_Loading.h"
#include "Particle_Manager.h"
#include "PlayerLight.h"
#include "GolemSkillRock2.h"
#include "GolemSkillRock1.h"
#include "Wall.h"
#include "Ring.h"

CGolem::CGolem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	
}

CGolem::CGolem(const CGolem& rhs)
	:CAnimMesh(rhs)
{
}

HRESULT CGolem::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CGolem::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);
	sTag = m_MeshInfo->sTag;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Golem"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	Ready_Sockets();

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	m_bColliderRender = false;

	m_eCurState = START;
	m_eNextState = START;
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, _vector{ 0.f,0.f,1.f });
	m_pTransformCom->Rotation(_vector{ 0.f,1.f,0.f }, 180.f);
	m_vTargetLook = { 0.f,0.f,1.f };
	m_pAnimModel->Set_AnimIndex(m_eCurState);
	m_fOutLinePower = 3.f;

	m_fMaxHp = 500;
	m_fMaxMp = 100.f;
	m_fNowHp = m_fMaxHp;
	m_fNowMp = 90.f;
	m_fDamage = 10.f;

	m_fColiisionTime = 0.06f;

	m_pTarget = PM->Get_PlayerPointer();
	Safe_AddRef(m_pTarget);
	PM->Add_Boss(this);

	CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 1;
	if (FAILED(__super::Add_Component(LEVEL_STAGE1, L"NavigationStage1", TEXT("NavigationStage1"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;
	
	/*CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 478;
	if (FAILED(__super::Add_Component(LEVEL_STAGE3, L"NavigationStage3", TEXT("NavigationStage3"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;

	m_pNavigation->Set_BattleIndex(473);

	Set_Dir();

	CRM->Start_Scene("Scene_Stage3Boss");*/

	UM->Add_Boss(this);
	Load_UI("BossBar");
	

	return S_OK;
}

void CGolem::Tick(_float fTimeDelta)
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
	if (GI->Key_Down(DIK_I))
	{

	}

	if (!m_bCollision)
	{
		m_fCollisionAcc += 1.f * fTimeDelta;
		if (m_fCollisionAcc >= m_fColiisionTime)
		{
			m_fCollisionAcc = 0.f;
			m_bCollision = true;
		}
	}

	DebugKeyInput();

	if (m_bHit)
	{
		m_fHitAcc += 1.f * fTimeDelta;
		if (m_fHitAcc >= 0.2f)
		{
			m_fHitAcc = 0.f;
			m_bHit = false;
		}
	}

	m_bPattern = false;
	
	Update(fTimeDelta);
}

void CGolem::LateTick(_float fTimeDelta)
{
	if (m_fNowHp <= 0 && !m_bDie)
	{
		Set_State(DIE);
		m_bDie = true;
		m_bPattern = false;
		m_bCollision = false;
		m_bRHand = false;
		m_bLHand = false;
		m_bAttack = false;
		
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

	m_pOBB[OBB_ATTACK]->Update(m_pTransformCom->Get_WorldMatrix());

	if (!m_bDie)
	{
		if (m_bCollision)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTER, this, m_pOBB[OBB_BODY]);

		if (m_bLHand)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB[OBB_LHAND]);

		if (m_bRHand)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB[OBB_RHAND]);

		if (m_bAttack)
			CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB[OBB_ATTACK]);
	}
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CGolem::Render()
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

		if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrixInverse", &m_pTransformCom->Get_WorldMatrixInverse(), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrixInverse", &GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
			return E_FAIL;

		m_pShaderCom->Set_RawValue("g_fOutLinePower", &m_fOutLinePower, sizeof(_float));

		if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, OUTLINEPASS)))
			return E_FAIL;

		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(j), TEX_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		if (m_bPattern)
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

		else if (m_bFinishCharge)
		{
			if (FAILED(m_pShaderCom->Set_RawValue("g_fGolemPattern", &m_fGolemPattern, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_GOLEMPATTERN)))
				return E_FAIL;
		}

		else
		{
			if(FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_DEFAULT)))
				return E_FAIL;

			if (m_bHit)
			{
				m_pShaderCom->Set_RawValue("g_vCamPos", &GI->Get_CamPosition(), sizeof(_float4));

				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NHIT)))
					return E_FAIL;
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

HRESULT CGolem::Render_ShadowDepth()
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

void CGolem::Collision(CGameObject * pOther, string sTag)
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
				m_fNowMp = 0.f;
			}
		}

		else if (m_eCurState != SKILL10_1)
			m_fNowMp += m_fDamage / 2.f;

		if (m_fNowMp >= 100.f)
		{
			m_bFinish = true;
		}
		else
		{
			m_bFinish = false;
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
				m_bPattern = false;
				m_bLHand = false;
				m_bRHand = false;
				Set_State(RTDOWN);
				m_fNowMp -= 10.f;
				CRM->Start_Shake(0.5f, 5.f, 0.06f);
				CRM->Start_Fov(40.f, 120.f);
				CRM->Set_FovDir(true);
			}		

			if (m_eCurState != SKILL8 && pOther->Get_Damage() == 63.f || pOther->Get_Damage() == 73.f || pOther->Get_Damage() == 5.5f)
			{
				m_bPattern = false;
				m_bLHand = false;
				m_bRHand = false;
				Set_State(RTDOWN);
				CRM->Start_Shake(0.5f, 5.f, 0.06f);
				CRM->Start_Fov(40.f, 120.f);
				CRM->Set_FovDir(true);
			}

			if (m_eCurState == SKILL5_2)
				m_fNowHp -= pOther->Get_Damage() * 0.2f;
			else
				m_fNowHp -= pOther->Get_Damage();

			m_bCollision = false;
			m_bHit = true;
			UM->Set_ExGaugeTex(1);
		}
	}
}

HRESULT CGolem::Ready_Collider()
{
	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(6.f, 6.f, 6.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 1.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Body";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Body"), (CComponent**)&m_pOBB[OBB_BODY], &ColliderDesc)))
		return E_FAIL;

	_float4x4 LHand = m_Sockets[SOCKET_LHAND]->Get_Transformation();
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

	ColliderDesc.vSize = _float3(24.f, 10.f, 24.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 1.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Attack";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Attack"), (CComponent**)&m_pOBB[OBB_ATTACK], &ColliderDesc)))
		return E_FAIL;


	return S_OK;
}

void CGolem::Set_NextMotion()
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
			Set_State(RUN);		
			return;
		}
		if (m_iNextMotion == 3)
		{
			Set_State(RUN);		
			return;
		}
	}
}

void CGolem::Set_NextAttack()
{
	_float fDistance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	if (fDistance >= 6.f)
		Range_Attack();	
	else
		Close_Attack();
}

void CGolem::Range_Attack()
{
	if (!m_pAnimModel->GetChangeBool())
	{
		m_iPreRangeAttack = m_iNextRangeAttack;
		m_iNextRangeAttack = GI->Get_Random(1, 3);

		while (m_iNextRangeAttack == m_iPreRangeAttack)
		{
			m_iNextRangeAttack = GI->Get_Random(1, 3);
			if (m_iNextRangeAttack != m_iPreRangeAttack)
				break;
		}

		if (m_iNextRangeAttack == 1)
		{
			Set_State(SKILL4_1);
			return;
		}
		if (m_iNextRangeAttack == 2)
		{
			Set_State(SKILL8);
			return;
		}
		if (m_iNextRangeAttack == 3)
		{
			Set_State(SKILL2);
			return;
		}
	}
}

void CGolem::Close_Attack()
{
	if (!m_pAnimModel->GetChangeBool())
	{
		m_iPreCloseAttack = m_iNextCloseAttack;
		m_iNextCloseAttack = GI->Get_Random(1, 7);

		while (m_iNextCloseAttack == m_iPreCloseAttack)
		{
			m_iNextCloseAttack = GI->Get_Random(1, 7);
			if (m_iNextCloseAttack != m_iPreCloseAttack)
				break;
		}

		if (m_iNextCloseAttack == 1)
		{
			Set_State(SKILL1);
			return;
		}
		if (m_iNextCloseAttack == 2)
		{
			Set_State(SKILL3);
			return;
		}
		if (m_iNextCloseAttack == 3)
		{
			Set_State(SKILL9);
			return;
		}

		if (m_iNextCloseAttack == 4)
		{
			Set_State(SKILL1);
			return;
		}
		if (m_iNextCloseAttack == 5)
		{
			Set_State(SKILL3);
			return;
		}
		if (m_iNextCloseAttack == 6)
		{
			Set_State(SKILL9);
			return;
		}

		if (m_iNextCloseAttack == 7)
		{
			Set_State(SKILL5_1);
			return;
		}
	}
}

void CGolem::CreateSkillRock2()
{
	
	CGolemSkillRock2::GOLEMROCK2INFO GolemInfo;
	XMStoreFloat4(&GolemInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	GolemInfo.vPos.x += -10.f;
	GolemInfo.vPos.z += 10.f;
	GolemInfo.vScale = { 3.f, 1.5f, 3.f };
	GI->Add_GameObjectToLayer(L"GolemSkillRock2", PM->Get_NowLevel(), L"GolemEffect", &GolemInfo);
	

}

void CGolem::Set_State(STATE eState)
{
	if (m_eNextState == eState)
		return;

	m_eNextState = eState;

	switch (m_eNextState)
	{
	case Client::CGolem::DOWN:
		break;
	case Client::CGolem::DIE:
		GI->PlaySoundW(L"GolemDie.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::RTDOWN:
		GI->PlaySoundW(L"GolemGroggy.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::RUN:
		m_fRunTempo = 1.7f;
		m_fRunTempoAcc = 0.f;
		break;
	case Client::CGolem::SKILL1:
			m_fDamage = 10.f;
		break;
	case Client::CGolem::SKILL2:
		break;
	case Client::CGolem::SKILL3:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 12.f, 1.f, 12.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,1.f,6.f });
			m_fDamage = 15.f;
		break;
	case Client::CGolem::SKILL4_1:
		GI->PlaySoundW(L"GolemSkill4.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::SKILL4_2:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 8.f, 8.f, 8.f });
		GI->PlaySoundW(L"GolemSkill4_2.ogg", SD_MONSTERVOICE, 0.9f);
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 10.f, 8.f, 10.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,4.f,0.f });
			m_fDamage = 25.f;
		break;
	case Client::CGolem::SKILL4_3:
		break;
	case Client::CGolem::SKILL5_1:
		GI->PlaySoundW(L"GolemSkill5.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::SKILL5_2:
		break;
	case Client::CGolem::SKILL5_3:
		break;
	case Client::CGolem::SKILL8:
		m_bRockOn = false;
		break;
	case Client::CGolem::SKILL9:
		m_fDamage = 15.f;
		GI->PlaySoundW(L"GolemSkill9.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::SKILL10_1:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 300.f, 300.f, 300.f });
		m_fDamage = 110.f;
		GI->PlaySoundW(L"GolemSkill10.ogg", SD_MONSTERVOICE, 0.9f);
		m_bFinishStart = false;
		m_fGolemPattern = 0.f;
		m_bFinishCharge = true;
		break;
	case Client::CGolem::SKILL10_2:
		break;
	case Client::CGolem::SKILL10_3:
		m_bFinishTime = false;
		break;
	case Client::CGolem::STANDUP:
		GI->PlaySoundW(L"GolemStand.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::START:
		break;
	case Client::CGolem::IDLE:
		GI->PlaySoundW(L"GolemWait.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CGolem::STATE_END:
		break;
	default:
		break;
	}

	m_pAnimModel->SetNextIndex(m_eNextState);
	m_pAnimModel->SetChangeBool(true);
}

void CGolem::Set_Dir()
{
	XMStoreFloat3(&m_vTargetLook, XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

void CGolem::End_Animation()
{
	if (m_pAnimModel->GetAniEnd())
	{
		if (m_bFinishStart)
		{
			Set_State(SKILL10_1);
			return;
		}
		switch (m_eCurState)
		{
		case Client::CGolem::DOWN:
			Set_State(STANDUP);
			break;
		case Client::CGolem::DIE:
			PM->Delete_Boss();
			Set_Dead();
			GI->StopAll();
			GI->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_STAGE4));
			break;
		case Client::CGolem::RTDOWN:
			Set_State(DOWN);
			break;
		case Client::CGolem::RUN:
			Set_NextAttack();
			break;
		case Client::CGolem::SKILL1:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL2:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL3:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL4_1:
			Set_State(SKILL4_2);
			break;
		case Client::CGolem::SKILL4_2:
			m_bAttack = false;
			Set_State(SKILL4_3);
			break;
		case Client::CGolem::SKILL4_3:
			m_bPattern = false;
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL5_1:
			if (!m_bFinish)
				UM->Set_Count(2);
			Set_State(SKILL5_2);
			break;
		case Client::CGolem::SKILL5_2:
			Set_NextMotion();		
			Set_State(SKILL5_3);
			break;
		case Client::CGolem::SKILL5_3:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL8:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL9:
			m_bLHand = false;
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL10_1:
			Set_State(SKILL10_2);
			break;
		case Client::CGolem::SKILL10_2:
			Set_State(SKILL10_3);
			break;
		case Client::CGolem::SKILL10_3:
			Set_NextMotion();
			break;
		case Client::CGolem::STANDUP:
			Set_NextMotion();
			break;
		case Client::CGolem::START:
			Set_State(RUN);
			break;
		case Client::CGolem::IDLE:
			Set_NextAttack();
			break;
		case Client::CGolem::STATE_END:
			break;
		default:
			break;
		}
	}
}

void CGolem::Update(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CGolem::DOWN:
		break;
	case Client::CGolem::DIE:
		break;
	case Client::CGolem::RTDOWN:
		break;
	case Client::CGolem::RUN:
	{
		m_fRunTempoAcc += 1.f * fTimeDelta;
		if (m_fRunTempoAcc >= m_fRunTempo)
		{
			m_fRunTempoAcc = 0.f;
			GI->PlaySoundW(L"GolemRun.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 3.5f, 0.04f);
		}

		Set_Dir();
		_float Distance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
		if (Distance > 4.f)
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRunSpeed, m_pNavigation, fTimeDelta);
		else
			Set_NextAttack();
		break;
	}
	case Client::CGolem::SKILL1:
		m_bPattern = false;
		m_bLHand = false;
		m_bRHand = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			m_bPattern = true;
		

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(7) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(8))
		{
			GI->PlaySoundW(L"GolemSkill1.ogg", SD_MONSTERVOICE, 0.9f);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.3f;
			EffectInfo.WorldMatrix.r[3] += Right * 2.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			m_bRHand = true;
			GI->PlaySoundW(L"GolemAttack1.ogg", SD_MONSTER1, 0.9f);
			CRM->Start_Shake(0.3f, 3.f, 0.03f);
			_float4 WorldPos;
			XMStoreFloat4(&WorldPos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemSkill1", WorldPos, false, CAlphaParticle::DIR_END);
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 5.f, m_pNavigation, fTimeDelta);
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
		{			
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.3f;
			EffectInfo.WorldMatrix.r[3] += Right * 1.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			m_fDamage = 15.f;
			m_bLHand = true;
			GI->PlaySoundW(L"GolemAttack1_1.ogg", SD_MONSTER1, 0.9f);
			GI->PlaySoundW(L"GolemSkill1_2.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.4f, 4.f, 0.04f);
			_float4 WorldPos;
			EffectInfo.WorldMatrix.r[3] -= Right * 2.f;
			XMStoreFloat4(&WorldPos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemSkill1_2", WorldPos, false, CAlphaParticle::DIR_END);
		}
		break;
	case Client::CGolem::SKILL2:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			Set_Dir();

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			GI->PlaySoundW(L"GolemSkill2.ogg", SD_MONSTERVOICE, 0.9f);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(4) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(5))
		{
			CGolemSkillRock1::GOLEMROCK1INFO GolemInfo;
			_matrix RHandMatrix = m_Sockets[SOCKET_RHAND]->Get_CombinedTransformation()* m_pAnimModel->Get_PivotMatrix()* m_pTransformCom->Get_WorldMatrix();
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			RHandMatrix.r[3] += Right * 2.f;
			RHandMatrix.r[3].m128_f32[1] += 2.f;
			XMStoreFloat4(&GolemInfo.vPos, RHandMatrix.r[3]);
			GolemInfo.vScale = { 1.f,1.f,1.f };
			GI->Add_GameObjectToLayer(L"GolemSkillRock1", PM->Get_NowLevel(), L"GolemEffect", &GolemInfo);
			GI->PlaySoundW(L"GolemSkill2_2.ogg", SD_MONSTERVOICE, 0.9f);
		}
		break;
	case Client::CGolem::SKILL3:
		m_bPattern = false;
		m_bAttack = false;
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
		{
			Set_Dir();
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			m_bPattern = true;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
		{
			GI->PlaySoundW(L"GolemSkill3.ogg", SD_MONSTERVOICE, 0.9f);
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 2.f;
			EffectInfo.WorldMatrix.r[3] += Right * 3.5f;
			EffectInfo.vScale = _float3{ 2.f,1.f,2.f };
			GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			m_bAttack = true;
			GI->PlaySoundW(L"GolemAttack3.ogg", SD_MONSTER1, 0.9f);
			CRM->Start_Shake(0.3f, 5.f, 0.04f);

			_float4 WorldPos;
			EffectInfo.WorldMatrix.r[3];
			EffectInfo.WorldMatrix.r[3] += Right * -3.f;
			XMStoreFloat4(&WorldPos, EffectInfo.WorldMatrix.r[3]);
			WorldPos.y += 0.5f;
			PTM->CreateParticle(L"GolemSkill3", WorldPos, false, CAlphaParticle::DIR_END);

			CRing::RINGINFO RingInfo;
			RingInfo.vSize = { 0.2f,0.3f,0.2f };
			RingInfo.vSpeed = _float3{ 0.8f, 0.f, 0.8f };
			RingInfo.fLifeTime = 0.3f;
			RingInfo.eColor = CRing::RING_GREEN;
			XMStoreFloat4(&RingInfo.vWorldPos, EffectInfo.WorldMatrix.r[3]);
			RingInfo.vWorldPos.y += 0.5f;
			GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_PlayerEffect", &RingInfo);

		}
		break;
	case Client::CGolem::SKILL4_1:
		Set_Dir();
		break;
	case Client::CGolem::SKILL4_2:
		m_bAttack = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				m_fRushSpeed = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION))) * 0.65f;
				Set_Dir();
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
				GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
				GI->PlaySoundW(L"GolemRun.ogg", SD_MONSTERVOICE, 0.9f);
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRushSpeed, m_pNavigation, fTimeDelta);		
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				PTM->CreateParticle(L"GolemTrail", WorldPos, false, CAlphaParticle::DIR_END);
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
			{
				m_bAttack = true;
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
				EffectInfo.WorldMatrix.r[3] += Look * 1.f;
				EffectInfo.WorldMatrix.r[3] += Right * 3.f;
				EffectInfo.vScale = _float3{ 1.5f,1.5f,1.5f };
				GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
				GI->PlaySoundW(L"GolemAttack4.ogg", SD_MONSTER1, 0.9f);
				CRM->Start_Shake(0.4f, 5.f, 0.05f);
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				PTM->CreateParticle(L"GolemSkill4", WorldPos, false, CAlphaParticle::DIR_END);

			}
		}
		break;
	case Client::CGolem::SKILL4_3:
		if(!m_pAnimModel->GetChangeBool())
			m_bPattern = true;
		break;
	case Client::CGolem::SKILL5_1:
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
				m_bPattern = true;
			else
				m_bPattern = false;
		}
		break;
	case Client::CGolem::SKILL5_2:
	{
		_float4 WorldPos;
		XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		PTM->CreateParticle(L"GolemCharge", WorldPos, false, CAlphaParticle::DIR_GOLEM);
		m_fNowHp += 0.2f;
		m_fNowMp += 0.1f;
		if (m_fNowMp >= 100.f)
			m_bFinish = true;
	}
		break;
	case Client::CGolem::SKILL5_3:
		break;
	case Client::CGolem::SKILL8:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{


			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();			
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.f;
			EffectInfo.WorldMatrix.r[3] += Right * -1.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			_float4 ParticlePos;
			XMStoreFloat4(&ParticlePos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemRock", ParticlePos, false, CAlphaParticle::DIR_END);
			GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			GI->PlaySoundW(L"GolemAttack1_1.ogg", SD_MONSTER1, 0.9f);
			GI->PlaySoundW(L"GolemSkill8.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 4.f, 0.04f);
			CGolemSkillRock2::GOLEMROCK2INFO GolemInfo;
			XMStoreFloat4(&GolemInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			GolemInfo.vPos.x += -5.f;
			GolemInfo.vPos.z += 5.f;
			GolemInfo.vScale = { 2.5f, 1.f, 2.5f };
			GI->Add_GameObjectToLayer(L"GolemSkillRock2", PM->Get_NowLevel(), L"GolemEffect", &GolemInfo);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.3f;
			EffectInfo.WorldMatrix.r[3] += Right * 2.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			_float4 ParticlePos;
			XMStoreFloat4(&ParticlePos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemRock", ParticlePos, false, CAlphaParticle::DIR_END);
			GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			GI->PlaySoundW(L"GolemAttack1_1.ogg", SD_MONSTER1, 0.9f);
			GI->PlaySoundW(L"GolemSkill8.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 4.f, 0.04f);
			CGolemSkillRock2::GOLEMROCK2INFO GolemInfo;
			XMStoreFloat4(&GolemInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			GolemInfo.vPos.x += 5.f;
			GolemInfo.vPos.z += 5.f;
			GolemInfo.vScale = { 2.f, 1.f, 2.f };
			GI->Add_GameObjectToLayer(L"GolemSkillRock2", PM->Get_NowLevel(), L"GolemEffect", &GolemInfo);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(4) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(5))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.f;
			EffectInfo.WorldMatrix.r[3] += Right * -1.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			_float4 ParticlePos;
			XMStoreFloat4(&ParticlePos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemRock", ParticlePos, false, CAlphaParticle::DIR_END);
			GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			GI->PlaySoundW(L"GolemAttack1_1.ogg", SD_MONSTER1, 0.9f);
			GI->PlaySoundW(L"GolemSkill8.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 4.f, 0.04f);
			CGolemSkillRock2::GOLEMROCK2INFO GolemInfo;
			XMStoreFloat4(&GolemInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			GolemInfo.vPos.x += 5.f;
			GolemInfo.vPos.z += -5.f;
			GolemInfo.vScale = { 2.f, 1.f, 2.f };
			GI->Add_GameObjectToLayer(L"GolemSkillRock2", PM->Get_NowLevel(), L"GolemEffect", &GolemInfo);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(6) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(7))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.3f;
			EffectInfo.WorldMatrix.r[3] += Right * 2.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			_float4 ParticlePos;
			XMStoreFloat4(&ParticlePos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemRock", ParticlePos, false, CAlphaParticle::DIR_END);
			GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			GI->PlaySoundW(L"GolemAttack1_1.ogg", SD_MONSTER1, 0.9f);
			GI->PlaySoundW(L"GolemSkill8.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 4.f, 0.04f);
			CGolemSkillRock2::GOLEMROCK2INFO GolemInfo;
			XMStoreFloat4(&GolemInfo.vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			GolemInfo.vPos.x += -5.f;
			GolemInfo.vPos.z += -5.f;
			GolemInfo.vScale = { 2.f, 1.f, 2.f };
			GI->Add_GameObjectToLayer(L"GolemSkillRock2", PM->Get_NowLevel(), L"GolemEffect", &GolemInfo);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(10) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(11))
		{
			GI->PlaySoundW(L"GolemSkill8_2.ogg", SD_MONSTERVOICE, 0.9f);
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(8) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(9))
		{
			CAnimMesh::EFFECTINFO EffectInfo;
			EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
			_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_vector Right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
			EffectInfo.WorldMatrix.r[3] += Look * 3.f;
			EffectInfo.WorldMatrix.r[3] += Right * 2.f;
			EffectInfo.vScale = _float3{ 1.f,1.f,1.f };
			_float4 ParticlePos;
			XMStoreFloat4(&ParticlePos, EffectInfo.WorldMatrix.r[3]);
			PTM->CreateParticle(L"GolemSkill8", ParticlePos, false, CAlphaParticle::DIR_END);
			GI->Add_GameObjectToLayer(L"GolemRock1", PM->Get_NowLevel(), L"Layer_GolemEffect", &EffectInfo);
			CRM->Start_Shake(0.4f, 6.f, 0.05f);
			GI->PlaySoundW(L"GolemAttack1.ogg", SD_MONSTER1, 0.9f);
			m_bRockOn = true;	
			return;
		}
		break;
	case Client::CGolem::SKILL9:
		m_bPattern = false;
		m_bLHand = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				m_bPattern = true;
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				WorldPos.y += 2.5f;
				_vector Look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				XMStoreFloat4(&WorldPos, XMLoadFloat4(&WorldPos) + Look * 3.f);
				PTM->CreateParticle(L"GolemSkill7", WorldPos, false, CAlphaParticle::DIR_END);

				GI->PlaySoundW(L"GolemSkill9_2.ogg", SD_MONSTERVOICE, 0.9f);
				m_bLHand = true;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 10.f, m_pNavigation, fTimeDelta);
			}
		}
		break;
	case Client::CGolem::SKILL10_1:
		m_bAttack = false;
		m_fGolemPattern += 0.08f * fTimeDelta;
		if (!m_pAnimModel->GetChangeBool())
		{
			
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
			{
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				PTM->CreateParticle(L"GolemCharge", WorldPos, false, CAlphaParticle::DIR_GOLEM);
				
			}
			
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				GI->Set_Speed(L"Timer_Main", 0.2f);
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				if (!m_bFinishTime)
				{
					m_bFinishTime = true;
					GI->PlaySoundW(L"GolemSkill10_1.ogg", SD_MONSTERVOICE, 0.9f);
					
				}
				m_bFinishCharge = false;
				GI->Set_Speed(L"Timer_Main", 1.f);
				CreateLight();
				_float4 WorldPos;
				XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				PTM->CreateParticle(L"GolemFinish", WorldPos, false, CAlphaParticle::DIR_END);
				PTM->CreateParticle(L"GolemFinish2", WorldPos, false, CAlphaParticle::DIR_END);

				CRing::RINGINFO RingInfo;
				RingInfo.vSize = { 0.3f,1.f,0.3f };
				RingInfo.vSpeed = _float3{ 1.5f, 0.f, 1.5f };
				RingInfo.fLifeTime = 0.2f;
				RingInfo.eColor = CRing::RING_GREEN;
				RingInfo.vWorldPos = WorldPos;
				RingInfo.vWorldPos.y += 1.f;
				GI->Add_GameObjectToLayer(L"Ring", PM->Get_NowLevel(), L"Layer_GolemEffect", &RingInfo);

				CWall::WALLINFO WallInfo;
				WallInfo.fMaxUVIndexX = 1.f;
				WallInfo.fMaxUVIndexY = 4.f;
				WallInfo.fUVSpeed = 0.05f;
				WallInfo.vSize = { 0.2f,10.f,0.2f };
				WallInfo.vSpeed = { -0.3f,0.f,-0.3f };
				WallInfo.vWorldPos = WorldPos;
				WallInfo.fEndSpeed = 0.5f;
				WallInfo.fLifeTime = 1.f;
				WallInfo.eColor = CWall::WALL_GREEN;
				GI->Add_GameObjectToLayer(L"Wall", PM->Get_NowLevel(), L"Layer_PlayerEffect", &WallInfo);
			
				m_bAttack = true;
			}
					
		}
		break;
	case Client::CGolem::SKILL10_2:
		m_bCollision = false;
		break;
	case Client::CGolem::SKILL10_3:
		m_bCollision = false;
		break;
	case Client::CGolem::STANDUP:
		break;
	case Client::CGolem::START:
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				GI->PlaySoundW(L"GolemStart.ogg", SD_MONSTERVOICE, 0.9f);
				CRM->Start_Shake(0.4f, 3.f, 0.03f);
			}
		}
		break;
	case Client::CGolem::IDLE:
	{
		/*_float4 WorldPos;
		XMStoreFloat4(&WorldPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		PTM->CreateParticle(L"GolemCharge", WorldPos, false, false, CAlphaParticle::DIR_GOLEM);*/
	}
		break;
	case Client::CGolem::STATE_END:
		break;
	default:
		break;
	}
}

HRESULT CGolem::Ready_Sockets()
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

HRESULT CGolem::Load_UI(char* DatName)
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

void CGolem::DebugKeyInput()
{
	if (GI->Key_Down(DIK_NUMPAD0))
		Set_State(START);

	if (GI->Key_Down(DIK_NUMPAD1))
		Set_State(SKILL1);

	if (GI->Key_Down(DIK_NUMPAD2))
		Set_State(SKILL2);

	if (GI->Key_Down(DIK_NUMPAD3))
		Set_State(SKILL3);

	if (GI->Key_Down(DIK_NUMPAD4))
		Set_State(SKILL4_1);

	if (GI->Key_Down(DIK_NUMPAD5))
		Set_State(SKILL5_1);

	if (GI->Key_Down(DIK_NUMPAD6))
		Set_State(SKILL8);

	if (GI->Key_Down(DIK_NUMPAD7))
		Set_State(SKILL9);

	if (GI->Key_Down(DIK_NUMPAD8))
		Set_State(SKILL10_1);

	if (GI->Key_Down(DIK_NUMPAD9))
		Set_State(RTDOWN);

	if (GI->Key_Down(DIK_M))
		Set_State(STANDUP);
}

void CGolem::CreateLight()
{
	CPlayerLight::PLAYERLIGHT PlayerLightInfo;
	_vector Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	XMStoreFloat4(&PlayerLightInfo.vPos, Pos);
	PlayerLightInfo.fCloseSpeed = 1.f;
	PlayerLightInfo.vScale = { 5.f,10.f,5.f };
	PlayerLightInfo.vAngle = { 0.f,0.f,0.f };
	GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);


	for (int i = 0; i < 15; ++i)
	{
		_vector Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		XMStoreFloat4(&PlayerLightInfo.vPos, Pos);
		PlayerLightInfo.fCloseSpeed = GI->Get_FloatRandom(0.05f, 0.1f);
		PlayerLightInfo.vScale = { GI->Get_FloatRandom(1.f,2.f),10.f, GI->Get_FloatRandom(1.f,2.f) };
		PlayerLightInfo.vAngle = { GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f),GI->Get_FloatRandom(0.f,360.f) };
		GI->Add_GameObjectToLayer(L"PlayerLight", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerLightInfo);
	}
}



CGolem * CGolem::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CGolem*		pInstance = new CGolem(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CGolem"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CGolem::Clone(void * pArg)
{
	CGolem*		pInstance = new CGolem(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CGolem"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGolem::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel);

	Safe_Release(m_pTarget);
	Safe_Release(m_pNavigation);

	for(auto& iter : m_pOBB)
	Safe_Release(iter);
}
