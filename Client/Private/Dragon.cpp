#include "stdafx.h"
#include "..\Public\Dragon.h"
#include "GameInstance.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "HierarchyNode.h"
#include "UI.h"
#include "UI_Manager.h"
#include "Pointer_Manager.h"
#include "Player.h"

CDragon::CDragon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	
}

CDragon::CDragon(const CDragon& rhs)
	:CAnimMesh(rhs)
{
}

HRESULT CDragon::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CDragon::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);
	sTag = m_MeshInfo->sTag;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Dragon"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	Ready_Sockets();

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	m_bColliderRender = true;

	m_eCurState = START;
	m_eNextState = START;

	m_vTargetLook = { 0.f,0.f,1.f };

	m_pAnimModel->Set_AnimIndex(m_eCurState);

	m_fMaxHp = 100;
	m_fMaxMp = 100.f;
	m_fNowHp = m_fMaxHp;
	m_fNowMp = 90.f;
	m_fDamage = 10.f;

	m_fColiisionTime = 0.1f;

	m_pTarget = PM->Get_PlayerPointer();
	Safe_AddRef(m_pTarget);
	PM->Add_Boss(this);

	Set_Dir();

	CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 1;
	if (FAILED(__super::Add_Component(LEVEL_STAGE1, L"NavigationStage1", TEXT("NavigationStage1"), (CComponent**)&m_pNavigation, &NaviDesc)))
	return E_FAIL;

	/*CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 172;
	if (FAILED(__super::Add_Component(LEVEL_STAGE4, L"NavigationStage4", TEXT("NavigationStage4"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;

	m_pNavigation->Set_BattleIndex(145);
	CRM->Start_Scene("Scene_Stage4Boss");*/

	UM->Add_Boss(this);
	Load_UI("BossBar");
	

	return S_OK;
}

void CDragon::Tick(_float fTimeDelta)
{
	if (m_fNowHp <= 0)
		m_fNowHp = 0;

	if (m_fNowMp <= 0)
		m_fNowMp = 0;

	if (m_fNowMp > 100.f)
		m_fNowMp = 100.f;

	if (m_fNowHp >= m_fMaxHp)
		m_fNowHp = m_fMaxHp;

	if (!m_pAnimModel->GetChangeBool())
		m_eCurState = m_eNextState;

	if (GI->Key_Down(DIK_7))
		Set_State(SKILL7_1);

	if (GI->Key_Down(DIK_0))
		m_bColliderRender = !m_bColliderRender;

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

void CDragon::LateTick(_float fTimeDelta)
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
		GI->PlaySoundW(L"DragonDie.ogg", SD_MONSTERVOICE, 0.9f);
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
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CDragon::Render()
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

		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(j), TEX_NORMALS, "g_NormalTexture")))
		{
			if (m_bPattern)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NPATTERN)))
					return E_FAIL;
			}

			else if (m_bHit)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NHIT)))
					return E_FAIL;
			}

			else if (m_bFinish)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NFINISH)))
					return E_FAIL;
			}

			else
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_NDEFAULT)))
					return E_FAIL;
			}
		}
		else
		{
			if (m_bPattern)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_PATTERN)))
					return E_FAIL;
			}

			else if (m_bHit)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_HIT)))
					return E_FAIL;
			}

			else if (m_bFinish)
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_FINISH)))
					return E_FAIL;
			}

			else
			{
				if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_DEFAULT)))
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

void CDragon::Collision(CGameObject * pOther, string sTag)
{
	if (sTag == "Player_Body")
	{
		if (m_bFinish)
		{
			if (UM->Get_Count() > 0)
				UM->Set_Count(-1);
			else
			{
				m_bFinishUpdate = true;
				m_bFinishStart = true;
				m_bFinish = false;
				m_fNowMp = 0.f;
			}
		}
		else if (m_eCurState != SKILL7_2)
			m_fNowMp += m_fDamage / 2.f;	

		if (m_fNowMp >= 100.f)		
			m_bFinish = true;
		
		else		
			m_bFinish = false;
		
	
	}

	if (sTag == "Player_Sword")
	{
		if (pOther->Can_Hit())
		{
			if (m_bPattern && pOther->Get_Damage() == 1.f || m_bPattern && pOther->Get_Damage() == 100.f)
			{
				m_bFinish = false;
				if(!m_bFinish)
					UM->Set_Count(2);
				GI->PlaySoundW(L"DragonGroggy.ogg", SD_MONSTERVOICE, 0.9f);
				m_bPattern = false;
				m_bLHand = false;
				m_bRHand = false;
				Set_State(GROGGYSTART);
				m_fNowMp -= 20.f;
				CRM->Start_Shake(0.5f, 5.f, 0.06f);
				CRM->Start_Fov(40.f, 120.f);
				CRM->Set_FovDir(true);
			}		
			if (m_eCurState == GROGGYSTART || m_eCurState == GROGGYLOOF)
				m_fNowHp -= pOther->Get_Damage() * 2.f;
			else
				m_fNowHp -= pOther->Get_Damage();

			m_bCollision = false;
			m_bHit = true;
			UM->Set_ExGaugeTex(1);
		}
	}
}

HRESULT CDragon::Ready_Collider()
{
	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(8.f, 6.f, 12.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
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

void CDragon::Set_NextMotion()
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
			m_iWalkCount = 0;
			return;
		}
		if (m_iNextMotion == 3)
		{
			Set_State(WALK);
			m_iWalkCount = 1;
			return;
		}
	}
}

void CDragon::Set_NextAttack()
{
	_float fDistance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	if (fDistance > 8.f)
		Range_Attack();
	else
		Close_Attack();
	
}

void CDragon::Range_Attack()
{
	if (!m_pAnimModel->GetChangeBool())
	{
		m_iPreRangeAttack = m_iNextRangeAttack;
		m_iNextRangeAttack = GI->Get_Random(1, 6);

		while (m_iNextRangeAttack == m_iPreRangeAttack)
		{
			m_iNextRangeAttack = GI->Get_Random(1, 6);
			if (m_iNextRangeAttack != m_iPreRangeAttack)
				break;
		}

		if (m_iNextRangeAttack == 1)
		{
			Set_State(SKILL6);
			return;
		}
		if (m_iNextRangeAttack == 2)
		{
			Set_State(SKILL9_1);
			return;
		}
		if (m_iNextRangeAttack == 3)
		{
			Set_State(SKILL4);
			return;
		}

		if (m_iNextRangeAttack == 4)
		{
			Set_State(SKILL10_1);
			return;
		}

		if (m_iNextRangeAttack == 5)
		{
			Set_State(SKILL8);
			return;
		}
	}

}

void CDragon::Close_Attack()
{
	if (!m_pAnimModel->GetChangeBool())
	{
		m_iPreCloseAttack = m_iNextCloseAttack;
		m_iNextCloseAttack = GI->Get_Random(1, 9);

		while (m_iNextCloseAttack == m_iPreCloseAttack)
		{
			m_iNextCloseAttack = GI->Get_Random(1, 9);
			if (m_iNextCloseAttack != m_iPreCloseAttack)
				break;
		}

		if (m_iNextCloseAttack == 1)
		{
			Set_State(SKILL3);
			return;
		}
		if (m_iNextCloseAttack == 2)
		{
			Set_State(SKILL4);
			return;
		}
		if (m_iNextCloseAttack == 3)
		{
			Set_State(SKILL5);
			return;
		}

		if (m_iNextCloseAttack == 4)
		{
			Set_State(SKILL10_1);
			return;
		}

		if (m_iNextCloseAttack == 5)
		{
			Set_State(SKILL8);
			return;
		}

		if (m_iNextCloseAttack == 6)
		{
			Set_State(SKILL13);
			return;
		}

		if (m_iNextCloseAttack == 7)
		{
			Set_State(SKILL14_1);
			return;
		}

		if (m_iNextCloseAttack == 8)
		{
			Set_State(SKILL15);
			return;
		}

		if (m_iNextCloseAttack == 9)
		{
			Set_State(BACKSTEP);
			return;
		}
	}

}

void CDragon::Set_State(STATE eState)
{
	if (m_eNextState == eState)
		return;

	m_eNextState = eState;

	switch (m_eNextState)
	{
	case Client::CDragon::BACKSTEP:
		GI->PlaySoundW(L"DragonBackStep.ogg", SD_MONSTERVOICE, 0.9f);
		XMStoreFloat3(&m_vBackStepLook, m_pTransformCom->Get_State(CTransform::STATE_POSITION) - XMLoadFloat3(&PM->Get_PlayerPointer()->Get_Pos()));
		break;
	case Client::CDragon::DIE:
		break;
	case Client::CDragon::GROGGYEND:
		break;
	case Client::CDragon::GROGGYLOOF:
		break;
	case Client::CDragon::GROGGYSTART:
		break;
	case Client::CDragon::SKILL1:
		break;
	case Client::CDragon::SKILL3:
		m_fDamage = 10.f;
		break;
	case Client::CDragon::SKILL4:
		m_fSkill4Speed = 40.f;
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 8.f, 6.f, 10.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,3.f,3.f });
		m_fDamage = 30.f;
		break;
	case Client::CDragon::SKILL5:
		m_fDamage = 20.f;
		break;
	case Client::CDragon::SKILL6:
		break;
	case Client::CDragon::SKILL7_1:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 300.f, 300.f, 300.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,3.f,6.f });
		break;
	case Client::CDragon::SKILL7_2:
		break;
	case Client::CDragon::SKILL7_3:
		break;
	case Client::CDragon::SKILL8:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 12.f, 6.f, 14.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,3.f,6.f });
		m_fDamage = 20.f;
		break;
	case Client::CDragon::SKILL9_1:
		m_fRushShakeAcc = 0.6f;
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 8.f, 6.f, 10.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,3.f,3.f });
		m_fDamage = 20.f;
		break;
	case Client::CDragon::SKILL9_2:
		break;
	case Client::CDragon::SKILL9_3:
		break;
	case Client::CDragon::SKILL10_1:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 5.f, 6.f, 6.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,3.f,2.f });
		m_fFlyAttackSpeed = 20.f;	
		m_fDamage = 20.f;
		break;
	case Client::CDragon::SKILL10_2:
		break;
	case Client::CDragon::SKILL13:
		GI->PlaySoundW(L"DragonSkill13.ogg", SD_MONSTERVOICE, 0.9f);
		m_fDamage = 30.f;
		break;
	case Client::CDragon::SKILL14_1:
		break;
	case Client::CDragon::SKILL14_2:
		break;
	case Client::CDragon::SKILL14_3:
		break;
	case Client::CDragon::SKILL15:
		m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 24.f, 10.f, 24.f });
		m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,5.f,0.f });
		m_fDamage = 60.f;
		break;
	case Client::CDragon::START:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			CRM->Start_Shake(0.5f, 6.f, 0.07f);
		break;
	case Client::CDragon::IDLE:
		GI->PlaySoundW(L"DragonWait.ogg", SD_MONSTERVOICE, 0.9f);
		break;
	case Client::CDragon::WALK:
		GI->PlaySoundW(L"DragonWait.ogg", SD_MONSTERVOICE, 0.9f);
		m_fWalkShakeAcc = 0.f;
		break;
	case Client::CDragon::FINISH:
		m_eNextState = SKILL1;
		CRM->Start_Scene("Scene_DragonFinish");
		m_fDamage = 110.f;
		XMStoreFloat3(&m_vTargetLook, m_pTransformCom->Get_State(CTransform::STATE_POSITION) - XMLoadFloat3(&m_pTarget->Get_Pos()));
		break;
	}
	m_pAnimModel->SetNextIndex(m_eNextState);
	m_pAnimModel->SetChangeBool(true);
}

void CDragon::Set_Dir()
{
	XMStoreFloat3(&m_vTargetLook, XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

void CDragon::End_Animation()
{
	if (m_pAnimModel->GetAniEnd())
	{
		if (m_bFinishStart)
		{
			if(m_eCurState != SKILL9_1 && m_eCurState != SKILL9_2 && m_eCurState != SKILL10_1)
			Set_State(FINISH);
			m_fNowMp = 0.f;
			m_bFinishStart = false;
			return;
		}
		switch (m_eCurState)
		{
		case Client::CDragon::BACKSTEP:
			Set_NextAttack();
			break;
		case Client::CDragon::DIE:
			PM->Delete_Boss();
			Set_Dead();
			break;
		case Client::CDragon::GROGGYEND:
			Set_NextMotion();
			break;
		case Client::CDragon::GROGGYLOOF:
			Set_State(GROGGYEND);
			break;
		case Client::CDragon::GROGGYSTART:
			Set_State(GROGGYLOOF);
			break;
		case Client::CDragon::SKILL1:
			if (m_bFinishUpdate)
				Set_State(SKILL7_1);
			else	
				Set_NextMotion();
			break;
		case Client::CDragon::SKILL3:		
			Set_NextMotion();		
			break;
		case Client::CDragon::SKILL4:	
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL5:
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL6:
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL7_1:
			m_bFinish = false;
			m_pAnimModel->Set_AnimIndex(SKILL7_2);
			m_eNextState = SKILL7_2;
			GI->PlaySoundW(L"DragonFinish.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.5f, 8.f, 0.04f);
			break;
		case Client::CDragon::SKILL7_2:
			m_pAnimModel->Set_AnimIndex(SKILL7_3);
			m_eNextState = SKILL7_3;
			break;
		case Client::CDragon::SKILL7_3:
			UM->Set_Count(2);
			m_fDamage = 20.f;
			m_bFinishUpdate = false;
			m_bAttack = false;
			m_bFinishStart = false;
			m_pOBB[OBB_ATTACK]->ChangeExtents(_float3{ 6.f, 6.f, 6.f });
			m_pOBB[OBB_ATTACK]->ChangeCenter(_float3{ 0.f,3.f,0.f });
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL8:
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL9_1:
			m_pAnimModel->Set_AnimIndex(SKILL9_2);
			m_eNextState = SKILL9_2;
			break;
		case Client::CDragon::SKILL9_2:			
			m_pAnimModel->Set_AnimIndex(SKILL9_3);
			m_eNextState = SKILL9_3;
			break;
		case Client::CDragon::SKILL9_3:
			m_bAttack = false;
			Set_NextMotion();		
			break;
		case Client::CDragon::SKILL10_1:
			m_pAnimModel->Set_AnimIndex(SKILL10_2);
			m_eNextState = SKILL10_2;
			m_bAttack = false;
			break;
		case Client::CDragon::SKILL10_2:		
			m_bAttack = false;
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL13:
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL14_1:
			Set_State(SKILL14_2);
			if (!m_bFinish)
				UM->Set_Count(2);
			break;
		case Client::CDragon::SKILL14_2:
			Set_State(SKILL14_3);
			break;
		case Client::CDragon::SKILL14_3:
			Set_NextMotion();
			break;
		case Client::CDragon::SKILL15:
			Set_NextMotion();
			break;
		case Client::CDragon::START:
			Set_State(WALK);
			break;
		case Client::CDragon::IDLE:
			Set_NextMotion();
			break;
		case Client::CDragon::WALK:
			if (m_iWalkCount < 2)
				m_iWalkCount += 1;
			
			if(m_iWalkCount >= 2)
				Set_NextAttack();
			break;
		}
	}
}

void CDragon::Update(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CDragon::BACKSTEP:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vBackStepLook), m_fBackStepSpeed, m_pNavigation, fTimeDelta);
			Set_Dir();
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			GI->PlaySoundW(L"DragonAttack13.ogg", SD_MONSTER1, 0.9f);
			CRM->Start_Shake(0.2f, 3.f, 0.03f);
		}
		break;
	case Client::CDragon::DIE:
		break;
	case Client::CDragon::GROGGYEND:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			GI->PlaySoundW(L"DragonFly.ogg", SD_MONSTERVOICE, 0.9f);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			m_bCollision = false;
		}
		break;
	case Client::CDragon::GROGGYLOOF:
		break;
	case Client::CDragon::GROGGYSTART:
		break;
	case Client::CDragon::SKILL1:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
		{
			GI->PlaySoundW(L"DragonFly.ogg", SD_MONSTERVOICE, 0.9f);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
		{
			GI->PlaySoundW(L"DragonFly.ogg", SD_MONSTERVOICE, 0.9f);
		}
		if (m_bFinishUpdate)
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				XMStoreFloat3(&m_vTargetLook, _vector{ 60.f, 2.f, 65.f } -m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				_float3 vDir;
				XMStoreFloat3(&vDir, _vector{ 60.f, 2.5f, 123.f } -m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				m_pTransformCom->Go_Dir(XMLoadFloat3(&vDir), m_fFinishSpeed, m_pNavigation, fTimeDelta);
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				CRM->Start_Shake(0.3f, 5.f, 0.04f);
				GI->PlaySoundW(L"DragonAttack13.ogg", SD_MONSTER1, 0.9f);
			}

			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			CRM->Start_Shake(0.3f, 5.f, 0.04f);
			GI->PlaySoundW(L"DragonAttack13.ogg", SD_MONSTER1, 0.9f);
		}
		break;
	case Client::CDragon::SKILL3:
		m_bRHand = false;
		m_bLHand = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(9) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(10))
		{
			GI->PlaySoundW(L"DragonAttack1.ogg", SD_MONSTER1, 0.9f);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(11) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(12))
		{
			GI->PlaySoundW(L"DragonAttack1_2.ogg", SD_MONSTER1, 0.9f);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fSkill3Speed, m_pNavigation, fTimeDelta);
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			m_bRHand = true;
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(7) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(8))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fSkill3Speed, m_pNavigation, fTimeDelta);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
		{
			m_bLHand = true;
			return;
		}

		break;
	case Client::CDragon::SKILL4:
		m_bPattern = false;
		m_bAttack = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(7) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(8))
		{
			GI->PlaySoundW(L"DragonBottom.ogg", SD_MONSTER1, 0.5f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(9) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(10))
		{
			GI->PlaySoundW(L"DragonBottom.ogg", SD_MONSTER1, 0.5f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
			GI->PlaySoundW(L"DragonSkill4.ogg", SD_MONSTERVOICE, 0.9f);
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			m_bPattern = true;
			return;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			m_bAttack = true;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			if (m_fSkill4Speed >= 1.f)
				m_fSkill4Speed -= 1.f;
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fSkill4Speed, m_pNavigation, fTimeDelta);
		}

		break;
	case Client::CDragon::SKILL5:
		m_bPattern = false;
		m_bLHand = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			m_bPattern = true;
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
		{
			GI->PlaySoundW(L"DragonAttack5.ogg", SD_MONSTERVOICE, 0.9f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
		{
			m_bLHand = true;
		}
		break;
	case Client::CDragon::SKILL6:
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				GI->PlaySoundW(L"DragonSkill6.ogg", SD_MONSTERVOICE, 0.9f);
			}
		}
		break;
	case Client::CDragon::SKILL7_1:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			GI->PlaySoundW(L"DragonBottom.ogg", SD_MONSTER1, 0.5f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			GI->PlaySoundW(L"DragonBottom.ogg", SD_MONSTER1, 0.5f);
		}
		break;
	case Client::CDragon::SKILL7_2:
		if (!m_pAnimModel->GetChangeBool())
			m_bAttack = true;
		break;
	case Client::CDragon::SKILL7_3:
		m_bAttack = false;
		break;
	case Client::CDragon::SKILL8:
		m_bPattern = false;
		m_bAttack = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				Set_Dir();
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				m_bPattern = true;
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				m_bAttack = true;
				GI->PlaySoundW(L"DragonSkill8.ogg", SD_MONSTERVOICE, 0.9f);
				return;
			}
		}
		break;
	case Client::CDragon::SKILL9_1:
	{
		m_bAttack = false;
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
		{
			Set_Dir();
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1))
		{
			m_fRushShakeAcc += 1.f * fTimeDelta;
			if (m_fRushShakeAcc >= m_fRushTempo)
			{
				m_fRushShakeAcc = 0.f;
				GI->PlaySoundW(L"DragonWalk.ogg", SD_MONSTERVOICE, 0.9f);
				CRM->Start_Shake(0.3f, 3.f, 0.04f);
			}
			if (m_pTransformCom->Go_NoSlide(XMLoadFloat3(&m_vTargetLook), m_fRushSpeed, m_pNavigation, fTimeDelta))
				Set_Dir();
			m_bAttack = true;
			return;
		}
		break;
	}
	case Client::CDragon::SKILL9_2:
	{
		m_fRushShakeAcc += 1.f * fTimeDelta;
		if (m_fRushShakeAcc >= m_fRushTempo)
		{
			m_fRushShakeAcc = 0.f;
			GI->PlaySoundW(L"DragonWalk.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 3.f, 0.04f);
		}
		if (m_pTransformCom->Go_NoSlide(XMLoadFloat3(&m_vTargetLook), m_fRushSpeed, m_pNavigation, fTimeDelta))
			Set_Dir();
		m_bAttack = true;
		break;
	}
	case Client::CDragon::SKILL9_3:
	{
		m_bAttack = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				m_fRushShakeAcc += 1.f * fTimeDelta;
				if (m_fRushShakeAcc >= m_fRushTempo)
				{
					m_fRushShakeAcc = 0.f;
					GI->PlaySoundW(L"DragonWalk.ogg", SD_MONSTERVOICE, 0.9f);
					CRM->Start_Shake(0.3f, 3.f, 0.04f);
				}
				if (m_pTransformCom->Go_NoSlide(XMLoadFloat3(&m_vTargetLook), m_fRushSpeed, m_pNavigation, fTimeDelta))
					Set_Dir();
				m_bAttack = true;
			}
		}
		break;
	}
	case Client::CDragon::SKILL10_1:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			Set_Dir();

		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
		{
			GI->PlaySoundW(L"DragonFly.ogg", SD_MONSTERVOICE, 0.9f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(4) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(5))
		{

			GI->PlaySoundW(L"DragonFly.ogg", SD_MONSTERVOICE, 0.9f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3))
		{
			m_bCollision = false;
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fFlyAttackSpeed, m_pNavigation, fTimeDelta);
			m_bAttack = true;
		}
		else
			m_bAttack = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
		{
			GI->PlaySoundW(L"DragonFly_2.ogg", SD_MONSTERVOICE, 0.9f);
		}
		
		break;
	case Client::CDragon::SKILL10_2:
		m_bPattern = true;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				m_bAttack = true;
				if (m_fFlyAttackSpeed >= 0.2f)
					m_fFlyAttackSpeed -= 0.2f;
				m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fFlyAttackSpeed, m_pNavigation, fTimeDelta);
			}
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(2) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(3))
			{
				GI->PlaySoundW(L"DragonAttack13.ogg", SD_MONSTER1, 0.9f);
				CRM->Start_Shake(0.3f, 4.f, 0.04f);
			}
			else
				m_bAttack = false;

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				m_bPattern = true;
			}
		}
		break;
	case Client::CDragon::SKILL13:
		m_bRHand = false;
		m_bPattern = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
				Set_Dir();
		}
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 6.f, m_pNavigation, fTimeDelta);
			return;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			GI->PlaySoundW(L"DragonAttack13.ogg", SD_MONSTER1, 0.9f);
			CRM->Start_Shake(0.4f, 4.f, 0.04f);
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
		{
			m_bPattern = true;
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			m_bRHand = true;
			return;
		}


		break;
	case Client::CDragon::SKILL14_1:
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
				m_bPattern = true;
			else
				m_bPattern = false;

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
				GI->PlaySoundW(L"DragonSkill14.ogg", SD_MONSTERVOICE, 0.9f);
		}
		break;
	case Client::CDragon::SKILL14_2:
		m_fNowHp += 1.f;
		m_fNowMp += 0.15f;
		if (m_fNowMp >= 100.f)
			m_bFinish = true;
		break;
	case Client::CDragon::SKILL14_3:
		break;
	case Client::CDragon::SKILL15:
		m_bPattern = false;
		m_bAttack = false;
		if (!m_pAnimModel->GetChangeBool())
		{
			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			{
				GI->PlaySoundW(L"DragonWing.ogg", SD_MONSTERVOICE, 0.9f);
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(5) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(6))
			{
				m_bCollision = false;
			}

			if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			{
				m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), 3.f, m_pNavigation, fTimeDelta);
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			{
				m_bAttack = true;
				GI->PlaySoundW(L"DragonAttack15.ogg", SD_MONSTER1, 0.9f);
				CRM->Start_Shake(0.4f, 6.f, 0.05f);
				return;
			}

			if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			{
				m_bPattern = true;
			}

			
		}
		break;
	case Client::CDragon::START:
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
		{
			GI->PlaySoundW(L"DragonStart.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.4f, 3.f, 0.03f);
		}
		break;
	case Client::CDragon::IDLE:
		break;
	case Client::CDragon::WALK:
	{
		m_fWalkShakeAcc += 1.f * fTimeDelta;
		if (m_fWalkShakeAcc >= m_fWalkTempo)
		{
			m_fWalkShakeAcc = 0.f;
			GI->PlaySoundW(L"DragonWalk.ogg", SD_MONSTERVOICE, 0.9f);
			CRM->Start_Shake(0.3f, 3.f, 0.04f);
		}
		if (!m_pAnimModel->GetChangeBool())
			Set_Dir();
		_float Distance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
		if (Distance > 8.f)
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRunSpeed, m_pNavigation, fTimeDelta);
		else
			Close_Attack();
		break;
	}

	}

	
	
}

HRESULT CDragon::Ready_Sockets()
{
	CHierarchyNode*		L_Finger0 = m_pAnimModel->Get_HierarchyNode("L_Finger0");
	if (nullptr == L_Finger0)
		return E_FAIL;
	m_Sockets.push_back(L_Finger0);

	CHierarchyNode*		R_Finger0 = m_pAnimModel->Get_HierarchyNode("R_Finger0");
	if (nullptr == R_Finger0)
		return E_FAIL;
	m_Sockets.push_back(R_Finger0);

	return S_OK;
}

HRESULT CDragon::Load_UI(char* DatName)
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

void CDragon::DebugKeyInput()
{
	if (GI->Key_Down(DIK_NUMPAD0))
		Set_State(BACKSTEP);

	if (GI->Key_Down(DIK_NUMPAD1))
		Set_State(SKILL1);

	if (GI->Key_Down(DIK_NUMPAD2))
		Set_State(SKILL3);

	if (GI->Key_Down(DIK_NUMPAD3))
		Set_State(SKILL4);

	if (GI->Key_Down(DIK_NUMPAD4))
		Set_State(SKILL5);

	if (GI->Key_Down(DIK_NUMPAD5))
		Set_State(SKILL6);

	if (GI->Key_Down(DIK_NUMPAD6))
		Set_State(SKILL7_1);

	if (GI->Key_Down(DIK_NUMPAD7))
		Set_State(SKILL8);

	if (GI->Key_Down(DIK_NUMPAD8))
		Set_State(SKILL15);

	if (GI->Key_Down(DIK_NUMPAD9))
		Set_State(SKILL13);

	if (GI->Key_Down(DIK_M))
		Set_State(SKILL10_1);
}

CDragon * CDragon::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CDragon*		pInstance = new CDragon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CDragon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CDragon::Clone(void * pArg)
{
	CDragon*		pInstance = new CDragon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CDragon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDragon::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel);

	Safe_Release(m_pTarget);
	Safe_Release(m_pNavigation);

	for(auto& iter : m_pOBB)
	Safe_Release(iter);
}

