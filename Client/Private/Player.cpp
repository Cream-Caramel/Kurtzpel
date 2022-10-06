#include "stdafx.h"
#include "..\Public\Player.h"
#include "GameInstance.h"
#include "HierarchyNode.h"

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

	
	
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Player"), (CComponent**)&m_pAnimModel[MODEL_PLAYER])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerTop", TEXT("Top"), (CComponent**)&m_pAnimModel[MODEL_TOP])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerBottom", TEXT("Bottom"), (CComponent**)&m_pAnimModel[MODEL_BOTTOM])))
		return E_FAIL;

	for (int i = 0; i < MODEL_END; ++i)
	{
		_matrix PivotMatrix = m_pAnimModel[i]->Get_PivotMatrix();
		PivotMatrix = XMMatrixRotationY(XMConvertToRadians(270.0f));
		m_pAnimModel[i]->Set_PivotMatrix(PivotMatrix);
	}
	m_AniIndex = IDLE;
	m_vTargetLook = { 0.f,0.f,1.f,0.f };

	Set_AniInfo();

	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->Set_AnimIndex(m_AniIndex);
	}
		
	Ready_Sockets();
	Ready_PlayerParts();
	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_UP))
	{
		
		
		for (auto& iter : m_pAnimModel)
		{
			iter->SetNextIndex(m_AniIndex);
			iter->SetChangeBool(true);
		}
	}
	Update_Parts();

	for (auto& pPart : m_Parts)
		pPart->Tick(fTimeDelta);
}

void CPlayer::LateTick(_float fTimeDelta)
{

	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_vTargetLook, 0.6f);

	for(int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->Play_Animation(fTimeDelta, m_pAnimModel[i]);
	}

	for (auto& pPart : m_Parts)
		pPart->LateTick(fTimeDelta);


	for (auto& pPart : m_Parts)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, pPart);

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	

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
	return S_OK;
}

void CPlayer::Set_State(STATE eState)
{
	if (m_eCurState == eState)
		return;

	m_eCurState = eState;

	switch (m_eCurState)
	{
	case Client::CPlayer::HITBACK:
		m_AniIndex = HITBACK;
		break;
	case Client::CPlayer::HITFRONT:
		m_AniIndex = HITFRONT;
		break;
	case Client::CPlayer::JUMP:
		m_AniIndex = JUMP;
		break;
	case Client::CPlayer::JUMPDOWN:
		m_AniIndex = JUMPDOWN;
		break;
	case Client::CPlayer::JUMPEND:
		m_AniIndex = JUMPEND;
		break;
	case Client::CPlayer::JUMPUP:
		m_AniIndex = JUMPUP;
		break;
	case Client::CPlayer::JUMPSTART:
		m_AniIndex = JUMPSTART;
		break;
	case Client::CPlayer::IDLE:
		m_AniIndex = IDLE;
		break;
	case Client::CPlayer::DASH:
		m_AniIndex = DASH;
		break;
	case Client::CPlayer::DIE:
		m_AniIndex = DIE;
		break;
	case Client::CPlayer::RESPAWN:
		m_AniIndex = RESPAWN;
		break;
	case Client::CPlayer::RUN:
		m_AniIndex = RUN;
		break;
	case Client::CPlayer::RUNEND:
		m_AniIndex = RUNEND;
		break;
	case Client::CPlayer::SPINCOMBOEND:
		m_AniIndex = SPINCOMBOEND;
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		m_AniIndex = SPINCOMBOLOOF;
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		m_AniIndex = SPINCOMBOSTART;
		break;
	case Client::CPlayer::FASTCOMBOEND:
		m_AniIndex = FASTCOMBOEND;
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		m_AniIndex = FASTCOMBOSTART;
		break;
	case Client::CPlayer::ROCKBREAK:
		m_AniIndex = ROCKBREAK;
		break;
	case Client::CPlayer::CHARGECRASH:
		m_AniIndex = CHARGECRASH;
		break;
	case Client::CPlayer::CHARGEREADY:
		m_AniIndex = CHARGEREADY;
		break;
	case Client::CPlayer::AIRCOMBO1:
		m_AniIndex = AIRCOMBO1;
		break;
	case Client::CPlayer::AIRCOMBO2:
		m_AniIndex = AIRCOMBO2;
		break;
	case Client::CPlayer::AIRCOMBO3:
		m_AniIndex = AIRCOMBO3;
		break;
	case Client::CPlayer::AIRCOMBO4:
		m_AniIndex = AIRCOMBO4;
		break;
	case Client::CPlayer::AIRCOMBOEND:
		m_AniIndex = AIRCOMBOEND;
		break;
	case Client::CPlayer::VOIDFRONTEND:
		m_AniIndex = VOIDFRONTEND;
		break;
	case Client::CPlayer::VOIDBACKEND:
		m_AniIndex = VOIDBACKEND;
		break;
	case Client::CPlayer::VOIDFRONT:
		m_AniIndex = VOIDFRONT;
		break;
	case Client::CPlayer::VOIDBACK:
		m_AniIndex = VOIDBACK;
		break;
	case Client::CPlayer::NOMALCOMBO1:
		m_AniIndex = NOMALCOMBO1;
		break;
	case Client::CPlayer::NOMALCOMBO2:
		m_AniIndex = NOMALCOMBO2;
		break;
	case Client::CPlayer::NOMALCOMBO3:
		m_AniIndex = NOMALCOMBO3;
		break;
	case Client::CPlayer::NOMALCOMBO4:
		m_AniIndex = NOMALCOMBO4;
		break;
	case Client::CPlayer::NOMALCOMBO5:
		m_AniIndex = NOMALCOMBO5;
		break;
	case Client::CPlayer::NOMALCOMBO6:
		m_AniIndex = NOMALCOMBO6;
		break;
	case Client::CPlayer::GROUNDCRASH:
		m_AniIndex = GROUNDCRASH;
		break;
	case Client::CPlayer::GROUNDREADY:
		m_AniIndex = GROUNDREADY;
		break;
	case Client::CPlayer::GROUNDRUN:
		m_AniIndex = GROUNDRUN;
		break;
	case Client::CPlayer::LEAPDOWN:
		m_AniIndex = LEAPDOWN;
		break;
	case Client::CPlayer::LEAPUP:
		m_AniIndex = LEAPUP;
		break;
	case Client::CPlayer::LEAPEND:
		m_AniIndex = LEAPEND;
		break;
	case Client::CPlayer::LEAPREADY:
		m_AniIndex = LEAPREADY;
		break;
	case Client::CPlayer::LEAPRUN:
		m_AniIndex = LEAPRUN;
		break;
	case Client::CPlayer::LEAPSTART:
		m_AniIndex = LEAPSTART;
		break;
	case Client::CPlayer::BLADEATTACK:
		m_AniIndex = BLADEATTACK;
		break;
	case Client::CPlayer::SLASHATTACK:
		m_AniIndex = SLASHATTACK;
		break;
	case Client::CPlayer::ROCKSHOT:
		m_AniIndex = ROCKSHOT;
		break;
	case Client::CPlayer::EX1ATTACK:
		m_AniIndex = EX1ATTACK;
		break;
	case Client::CPlayer::EX2ATTACK:
		m_AniIndex = EX2ATTACK;
		break;
	case Client::CPlayer::EX1READY:
		m_AniIndex = EX1READY;
		break;
	case Client::CPlayer::EX2READY:
		m_AniIndex = EX2READY;
		break;
	default:
		m_AniIndex = IDLE;
		break;
	}
	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->SetNextIndex(m_AniIndex);
		m_pAnimModel[i]->SetChangeBool(true);
	}
}

void CPlayer::Set_Dir(DIR eDir)
{
	if (m_eDir == eDir)
		return;

	m_eDir = eDir;

	switch (m_eDir)
	{
	case Client::CPlayer::DIR_UP:
		m_vTargetLook = { 0.f,0.f,1.f,0.f };
		break;
	case Client::CPlayer::DIR_DOWN:
		m_vTargetLook = { 0.f,0.f,-1.f,0.f };
		break;
	case Client::CPlayer::DIR_RIGHT:
		m_vTargetLook = { 1.f,0.f,0.f,0.f };
		break;
	case Client::CPlayer::DIR_LEFT:
		m_vTargetLook = { -1.f,0.f,0.f,0.f };
		break;
	case Client::CPlayer::DIR_LU:
		m_vTargetLook = { -1.f,0.f,1.f,0.f };
		break;
	case Client::CPlayer::DIR_RU:
		m_vTargetLook = { 1.f,0.f,1.f,0.f };
		break;
	case Client::CPlayer::DIR_LD:
		m_vTargetLook = { -1.f,0.f,-1.f,0.f };
		break;
	case Client::CPlayer::DIR_RD:
		m_vTargetLook = { 1.f,0.f,-1.f,0.f };
		break;
	default:
		break;
	}
}

void CPlayer::End_Animation()
{
	if (m_pAnimModel[0]->GetAniEnd())
	{

	}
}

void CPlayer::Get_KeyInput(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CPlayer::HITBACK:
		break;
	case Client::CPlayer::HITFRONT:
		break;
	case Client::CPlayer::JUMP:
		break;
	case Client::CPlayer::JUMPDOWN:
		break;
	case Client::CPlayer::JUMPEND:
		break;
	case Client::CPlayer::JUMPUP:
		break;
	case Client::CPlayer::JUMPSTART:
		break;
	case Client::CPlayer::IDLE:
		break;
	case Client::CPlayer::DASH:
		break;
	case Client::CPlayer::DIE:
		break;
	case Client::CPlayer::RESPAWN:
		break;
	case Client::CPlayer::RUN:
		break;
	case Client::CPlayer::RUNEND:
		break;
	case Client::CPlayer::SPINCOMBOEND:
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		break;
	case Client::CPlayer::SPINCOMBOSTART:
		break;
	case Client::CPlayer::FASTCOMBOEND:
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		break;
	case Client::CPlayer::ROCKBREAK:
		break;
	case Client::CPlayer::CHARGECRASH:
		break;
	case Client::CPlayer::CHARGEREADY:
		break;
	case Client::CPlayer::AIRCOMBO1:
		break;
	case Client::CPlayer::AIRCOMBO2:
		break;
	case Client::CPlayer::AIRCOMBO3:
		break;
	case Client::CPlayer::AIRCOMBO4:
		break;
	case Client::CPlayer::AIRCOMBOEND:
		break;
	case Client::CPlayer::VOIDFRONTEND:
		break;
	case Client::CPlayer::VOIDBACKEND:
		break;
	case Client::CPlayer::VOIDFRONT:
		break;
	case Client::CPlayer::VOIDBACK:
		break;
	case Client::CPlayer::NOMALCOMBO1:
		break;
	case Client::CPlayer::NOMALCOMBO2:
		break;
	case Client::CPlayer::NOMALCOMBO3:
		break;
	case Client::CPlayer::NOMALCOMBO4:
		break;
	case Client::CPlayer::NOMALCOMBO5:
		break;
	case Client::CPlayer::NOMALCOMBO6:
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
		break;
	case Client::CPlayer::SLASHATTACK:
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

void CPlayer::Input_Direction()
{
	if (GI->Key_Pressing(DIK_UP))
	{
		Set_Dir(DIR_UP);
		if (GI->Key_Pressing(DIK_RIGHT))
		{
			Set_Dir(DIR_RU);
		}
		if (GI->Key_Pressing(DIK_LEFT))
		{
			Set_Dir(DIR_LU);
		}
	}

	else if (GI->Key_Pressing(DIK_DOWN))
	{
		Set_Dir(DIR_DOWN);
		if (GI->Key_Pressing(DIK_RIGHT))
		{
			Set_Dir(DIR_RD);
		}
		if (GI->Key_Pressing(DIK_LEFT))
		{
			Set_Dir(DIR_LD);
		}

	}

	else if (GI->Key_Pressing(DIK_RIGHT))
	{
		Set_Dir(DIR_RIGHT);
	}

	else if (GI->Key_Pressing(DIK_LEFT))
	{
		Set_Dir(DIR_LEFT);
	}
}

void CPlayer::Update(_float fTimeDelta)
{
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
		m_pAnimModel[1]->SetTimeLimit(m_pAnimModel[0]->GetTimeLimit());
		m_pAnimModel[2]->SetDuration(m_pAnimModel[0]->GetDuration());
		m_pAnimModel[2]->SetTickPerSecond(m_pAnimModel[0]->GetTickPerSecond());
		m_pAnimModel[2]->SetTimeLimit(m_pAnimModel[0]->GetTimeLimit());
		++AniIndex;
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

CAnimMesh * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
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
	Safe_Release(m_pAnimModel[MODEL_PLAYER]);
	Safe_Release(m_pAnimModel[MODEL_TOP]);
	Safe_Release(m_pAnimModel[MODEL_BOTTOM]);

	for (auto& pPart : m_Parts)
		Safe_Release(pPart);

	m_Parts.clear();

}
