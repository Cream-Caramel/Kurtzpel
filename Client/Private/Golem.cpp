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

	m_bColliderRender = true;

	m_eCurState = SKILL4_2;
	m_eNextState = SKILL4_2;
	m_vTargetLook = { 0.f,0.f,1.f };

	m_pAnimModel->Set_AnimIndex(m_eCurState);

	m_fMaxHp = 500;
	m_fMaxMp = 100.f;
	m_fNowHp = m_fMaxHp;
	m_fNowMp = 10.f;
	m_fDamage = 10.f;

	m_fColiisionTime = 0.1f;

	m_pTarget = PM->Get_PlayerPointer();
	Safe_AddRef(m_pTarget);

	CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 1;
	if (FAILED(__super::Add_Component(LEVEL_STAGE1, L"NavigationStage1", TEXT("NavigationStage1"), (CComponent**)&m_pNavigation, &NaviDesc)))
	return E_FAIL;

	

	
	/*CNavigation::NAVIGATIONDESC NaviDesc;
	NaviDesc.iCurrentIndex = 478;
	if (FAILED(__super::Add_Component(LEVEL_STAGE3, L"NavigationStage3", TEXT("NavigationStage3"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;

	m_pNavigation->Set_BattleIndex(473);*/

	

	UM->Add_Boss(this);
	Load_UI("BossBar");
	

	return S_OK;
}

void CGolem::Tick(_float fTimeDelta)
{
	if (m_fNowHp <= 0)
		m_fNowHp = 0;

	if (m_fNowMp <= 0)
		m_fNowMp = 0;

	if (m_fNowMp >= 100.f)
		m_fNowMp = 100.f;

	if (m_fNowHp >= m_fMaxHp)
		m_fNowHp = m_fMaxHp;

	if (!m_pAnimModel->GetChangeBool())
		m_eCurState = m_eNextState;

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
	}

	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMLoadFloat3(&m_vTargetLook), 0.3f);

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
		else
		{
			if (FAILED(m_pAnimModel->Render(m_pShaderCom, j, ANIM_DEFAULT)))
				return E_FAIL;
		}
	}

	for (int i = 0; i < OBB_END; ++i)
	{
		if(m_bColliderRender)
			m_pOBB[i]->Render();
	}
	return S_OK;
}

void CGolem::Collision(CGameObject * pOther, string sTag)
{
	if (sTag == "Player_Body")
	{
		m_fNowMp += 10.f;	
	}

	if (sTag == "Player_Sword")
	{
		if (pOther->Can_Hit())
		{
			if (m_bPattern && pOther->Get_Damage() == 1.f)
			{
				m_bPattern = false;
				m_bLHand = false;
				m_bRHand = false;
				Set_State(RTDOWN);
				m_fNowMp -= 10.f;
			}		
			if (m_eCurState == RTDOWN || m_eCurState == DOWN)
				m_fNowHp -= pOther->Get_Damage() * 2.f;

			else if (m_eCurState == SKILL5_2)
				m_fNowHp -= pOther->Get_Damage() * 0.2f;
			else
				m_fNowHp -= pOther->Get_Damage();

			m_bCollision = false;
			m_bHit = true;
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
	ColliderDesc.vSize = _float3(4.f, 4.f, 4.f);
	ColliderDesc.vCenter = _float3(LHand._41, LHand._42, LHand._43);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Attack";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_LHand"), (CComponent**)&m_pOBB[OBB_LHAND], &ColliderDesc)))
		return E_FAIL;


	_float4x4 RHand = m_Sockets[SOCKET_RHAND]->Get_Transformation();	
	ColliderDesc.vSize = _float3(4.f, 4.f, 4.f);
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
	int random = GI->Get_Random(1, 3);
	if (random == 1)
	{
		Set_State(IDLE);
		return;
	}
	if (random == 2)
	{
		Set_State(RUN);
		return;
	}
	if (random == 3)
	{
		Set_State(RUN);
		return;
	}
}

void CGolem::Set_NextAttack()
{
	_float fDistance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
	if (fDistance >= 4.f)
		Range_Attack();	
	else
		Close_Attack();
}

void CGolem::Range_Attack()
{
	int random = GI->Get_Random(1, 3);
	if (random == 1)
	{
		Set_State(SKILL4_1);
		return;
	}
	if (random == 2)
	{
		Set_State(SKILL8);
		return;
	}
	if (random == 3)
	{
		Set_State(SKILL2);
		return;
	}
}

void CGolem::Close_Attack()
{
	int random = GI->Get_Random(1, 5);
	if (random == 1)
	{
		Set_State(SKILL1);
		return;
	}
	if (random == 2)
	{
		Set_State(SKILL3);
		return;
	}
	if (random == 3)
	{
		Set_State(SKILL9);
		return;
	}
	if (random == 4)
	{
		Set_State(SKILL10_1);
		return;
	}
	if (random == 5)
	{
		Set_State(SKILL5_1);
		return;
	}

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
		break;
	case Client::CGolem::RTDOWN:
		break;
	case Client::CGolem::RUN:
		break;
	case Client::CGolem::SKILL1:
		if (m_fNowMp >= 100.f)
			m_fDamage = 20.f;
		else
			m_fDamage = 10.f;
		break;
	case Client::CGolem::SKILL2:
		break;
	case Client::CGolem::SKILL3:
		if (m_fNowMp >= 100.f)
			m_fDamage = 30.f;
		else
			m_fDamage = 15.f;
		break;
	case Client::CGolem::SKILL4_1:
		break;
	case Client::CGolem::SKILL4_2:
		if (m_fNowMp >= 100.f)
			m_fDamage = 50.f;
		else
			m_fDamage = 25.f;
		break;
	case Client::CGolem::SKILL4_3:
		break;
	case Client::CGolem::SKILL5_1:
		break;
	case Client::CGolem::SKILL5_2:
		break;
	case Client::CGolem::SKILL5_3:
		break;
	case Client::CGolem::SKILL8:
		break;
	case Client::CGolem::SKILL9:
		if (m_fNowMp >= 100.f)
			m_fDamage = 20.f;
		else
			m_fDamage = 10.f;
		break;
	case Client::CGolem::SKILL10_1:
		if (m_fNowMp >= 100.f)
			m_fDamage = 100.f;
		else
			m_fDamage = 50.f;
		break;
	case Client::CGolem::SKILL10_2:
		break;
	case Client::CGolem::SKILL10_3:
		break;
	case Client::CGolem::STANDUP:
		break;
	case Client::CGolem::START:
		break;
	case Client::CGolem::IDLE:
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
		switch (m_eCurState)
		{
		case Client::CGolem::DOWN:
			Set_State(STANDUP);
			break;
		case Client::CGolem::DIE:
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
			Set_State(SKILL4_3);
			break;
		case Client::CGolem::SKILL4_3:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL5_1:
			Set_State(SKILL5_2);
			break;
		case Client::CGolem::SKILL5_2:
			Set_State(SKILL5_3);
			break;
		case Client::CGolem::SKILL5_3:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL8:
			Set_NextMotion();
			break;
		case Client::CGolem::SKILL9:
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
		Set_Dir();
		_float Distance = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
		if (Distance > 2.f)
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRunSpeed, m_pNavigation, fTimeDelta);
		else
			Set_NextAttack();
		break;
	}
	case Client::CGolem::SKILL1:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			Set_Dir();		

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			m_bPattern = true;
		else
			m_bPattern = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			m_bLHand = true;
		else
			m_bLHand = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(3) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(4))
			m_bRHand = true;
		else
			m_bRHand = false;	
		break;
	case Client::CGolem::SKILL2:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			Set_Dir();
		break;
	case Client::CGolem::SKILL3:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))			
			Set_Dir();

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			m_bPattern = true;
		else
			m_bPattern = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			m_bLHand = true;
			m_bRHand = true;
		}
		else
		{
			m_bLHand = false;
			m_bRHand = false;
		}
		break;
	case Client::CGolem::SKILL4_1:
		Set_Dir();
		break;
	case Client::CGolem::SKILL4_2:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
		{
			m_fRushSpeed = XMVectorGetX(XMVector4Length(XMLoadFloat3(&m_pTarget->Get_Pos()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION))) * 0.65f;
			Set_Dir();
		}

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fRushSpeed, m_pNavigation, fTimeDelta);
			m_bLHand = true;
			m_bRHand = true;
		}
		else
		{
			m_bLHand = false;
			m_bRHand = false;
		}
		break;
	case Client::CGolem::SKILL4_3:
		break;
	case Client::CGolem::SKILL5_1:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			Set_Dir();
		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			m_bPattern = true;
		else
			m_bPattern = false;
		break;
	case Client::CGolem::SKILL5_2:
			m_fNowHp += 0.5f;
			m_fNowMp += 0.1f;
		break;
	case Client::CGolem::SKILL5_3:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			Set_Dir();
		break;
	case Client::CGolem::SKILL8:
		break;
	case Client::CGolem::SKILL9:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			Set_Dir();

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			m_bPattern = true;
		else
			m_bPattern = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
		{
			m_bLHand = true;
			m_bRHand = true;
		}
		else
		{
			m_bLHand = false;
			m_bRHand = false;
		}
		break;
	case Client::CGolem::SKILL10_1:
		if (m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(0))
			Set_Dir();

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(0) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(1))
			m_bPattern = true;
		else
			m_bPattern = false;

		if (m_pAnimModel->GetPlayTime() >= m_pAnimModel->GetTimeLimit(1) && m_pAnimModel->GetPlayTime() <= m_pAnimModel->GetTimeLimit(2))
			m_bAttack = true;
		else
			m_bAttack = false;
		break;
	case Client::CGolem::SKILL10_2:
		m_bCollision = false;
		break;
	case Client::CGolem::SKILL10_3:
		break;
	case Client::CGolem::STANDUP:
		break;
	case Client::CGolem::START:
		break;
	case Client::CGolem::IDLE:
		Set_Dir();
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

		if (0 == dwByte)	// ���̻� ���� �����Ͱ� ���� ���
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