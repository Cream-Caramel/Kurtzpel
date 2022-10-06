#include "stdafx.h"
#include "..\Public\Player.h"
#include "GameInstance.h"
#include "HierarchyNode.h"
#include "Pointer_Manager.h"


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
	sTag = L"Player";

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Player", TEXT("Player"), (CComponent**)&m_pAnimModel[MODEL_PLAYER])))
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
	
	m_eCurState = IDLE;
	m_vTargetLook = { 0.f,0.f,1.f};

	Set_AniInfo();

	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->Set_AnimIndex(m_eCurState);
	}
		
	Ready_Sockets();
	Ready_PlayerParts();
	PM->Add_Player(this);	

	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
		
	m_bKeyInput = false;

	Get_KeyInput(fTimeDelta);

	Update(fTimeDelta);

	Update_Parts();
	for (auto& pPart : m_Parts)
		pPart->Tick(fTimeDelta);
}

void CPlayer::LateTick(_float fTimeDelta)
{

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

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	
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
	return S_OK;
}

_vector CPlayer::Get_PlayerPos()
{
	return m_pTransformCom->Get_State(CTransform::STATE_POSITION);
}

void CPlayer::Set_State(STATE eState)
{
	if (m_eCurState == eState)
		return;

	m_eCurState = eState;

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
		m_fDashSpeed = 20.f;
		break;
	case Client::CPlayer::DIE:
		
		break;
	case Client::CPlayer::RESPAWN:
		
		break;
	case Client::CPlayer::RUN:
		m_fRunSpeed = 8.f;
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
		m_fNC1Speed = 5.f;
		break;
	case Client::CPlayer::NOMALCOMBO2:
		m_fNC2Speed = 5.f;
		break;
	case Client::CPlayer::NOMALCOMBO3:
		m_fNC3Speed = 6.f;
		break;
	case Client::CPlayer::NOMALCOMBO4:
		m_fNC4Speed = 6.f;
		break;
	case Client::CPlayer::NOMALCOMBO5:
		m_fNC5Speed = 5.f;
		break;
	case Client::CPlayer::NOMALCOMBO6:
		m_fNC6Speed = 8.f;
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
	
	}
	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->SetNextIndex(m_eCurState);
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
	default:
		break;
	}
}

void CPlayer::End_Animation()
{
	if (m_pAnimModel[0]->GetAniEnd())
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
			Set_State(IDLE);
			m_fDashSpeed = 20.f;
			break;
		case Client::CPlayer::DIE:
			break;
		case Client::CPlayer::RESPAWN:
			break;
		case Client::CPlayer::RUN:
			break;
		case Client::CPlayer::RUNEND:
			Set_State(IDLE);
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
			Set_State(IDLE);
			break;
		case Client::CPlayer::NOMALCOMBO2:
			Set_State(IDLE);
			break;
		case Client::CPlayer::NOMALCOMBO3:
			Set_State(IDLE);
			break;
		case Client::CPlayer::NOMALCOMBO4:
			Set_State(IDLE);
			break;
		case Client::CPlayer::NOMALCOMBO5:
			Set_State(IDLE);
			break;
		case Client::CPlayer::NOMALCOMBO6:
			Set_State(IDLE);
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
}

void CPlayer::Get_KeyInput(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CPlayer::JUMP:
		Jump_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::JUMPDOWN:
		JumpDown_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::JUMPUP:
		JumpUp_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::IDLE:
		Idle_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::RUN:
		Run_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::RUNEND:
		RunEnd_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::SPINCOMBOLOOF:
		SpinComboLoof_KeyInput(fTimeDelta);
		break;
	case Client::CPlayer::FASTCOMBOSTART:
		FastComboStart_KeyInput(fTimeDelta);
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

bool CPlayer::Input_Direction()
{
	if (PM->Get_CameraPlayerPos().z < m_vPlayerPos.z && m_fCamDistanceZ > m_fCamDistanceX)
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
	return m_bKeyInput;
	
}

void CPlayer::Update(_float fTimeDelta)
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
		if (m_fDashSpeed > 0.5f)
			m_fDashSpeed -= 0.5f;
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fDashSpeed, fTimeDelta);
		break;
	case Client::CPlayer::DIE:
		break;
	case Client::CPlayer::RESPAWN:
		break;
	case Client::CPlayer::RUN:
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 8.f, fTimeDelta);
		break;
	case Client::CPlayer::RUNEND:
		if (m_fRunSpeed > 0.15f)
		{
			m_fRunSpeed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fRunSpeed, fTimeDelta);
		}
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
		if (m_fNC1Speed > 0.15f)
		{
			m_fNC1Speed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC1Speed, fTimeDelta);
		}
		break;
	case Client::CPlayer::NOMALCOMBO2:
		if (m_fNC2Speed > 0.15f)
		{
			m_fNC2Speed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC2Speed, fTimeDelta);
		}
		break;
	case Client::CPlayer::NOMALCOMBO3:
		if (m_fNC3Speed > 0.1f)
		{
			m_fNC3Speed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC3Speed, fTimeDelta);
		}
		break;
	case Client::CPlayer::NOMALCOMBO4:
		if (m_fNC4Speed > 0.1f)
		{
			m_fNC4Speed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC4Speed, fTimeDelta);
		}
		break;
	case Client::CPlayer::NOMALCOMBO5:
		if (m_fNC5Speed > 0.15f)
		{
			m_fNC5Speed -= 0.1f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC5Speed, fTimeDelta);
		}
		break;
	case Client::CPlayer::NOMALCOMBO6:
		if (m_fNC6Speed > 0.f)
		{		
			if (m_fNC6Speed >= 4.f)
			{
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION) + _vector{ 0.f,0.1f,0.f,0.f });
			}
			else
			{
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION) + _vector{ 0.f,-0.1f,0.f,0.f });
			}
			m_fNC6Speed -= 0.15f;
			m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fNC6Speed, fTimeDelta);
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

void CPlayer::Set_PlayerUseInfo()
{
	XMStoreFloat3(&m_vPlayerPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_fCamDistanceX = fabs(PM->Get_CameraPlayerPos().x - m_vPlayerPos.x);
	m_fCamDistanceZ = fabs(PM->Get_CameraPlayerPos().z - m_vPlayerPos.z);
}

void CPlayer::Jump_KeyInput(_float fTimeDelta)
{
}

void CPlayer::JumpDown_KeyInput(_float fTimeDelta)
{
}

void CPlayer::JumpUp_KeyInput(_float fTimeDelta)
{
}

void CPlayer::Idle_KeyInput(_float fTimeDelta)
{
	//Input_Direction();
	if (GI->Key_Down(DIK_W) || GI->Key_Down(DIK_A) || GI->Key_Down(DIK_S) || GI->Key_Down(DIK_D))
	{
		Set_State(RUN);
		return;
	}

	if (GI->Key_Down(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}

	if (GI->Mouse_Down(DIMK_LBUTTON))
	{
		Set_State(NOMALCOMBO1);
		return;
	}

		
}

void CPlayer::Run_KeyInput(_float fTimeDelta)
{
	if (!Input_Direction())
	{
		Set_State(RUNEND);
		return;
	}
	if (GI->Key_Down(DIK_LSHIFT))
	{
		Set_State(DASH);
		return;
	}

	if (GI->Mouse_Down(DIMK_LBUTTON))
	{
		Set_State(NOMALCOMBO1);
		return;
	}

}

void CPlayer::RunEnd_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() > 15.f)
	{
		if (GI->Key_Down(DIK_W) || GI->Key_Down(DIK_A) || GI->Key_Down(DIK_S) || GI->Key_Down(DIK_D))
		{
			Set_State(RUN);
			return;
		}
		else if (GI->Key_Down(DIK_LSHIFT))
		{
			Set_State(DASH);
			return;
		}
	}


}

void CPlayer::SpinComboLoof_KeyInput(_float fTimeDelta)
{
}

void CPlayer::FastComboStart_KeyInput(_float fTimeDelta)
{
}

void CPlayer::ChargeReady_KeyInput(_float fTimeDelta)
{
}

void CPlayer::AirCombo1_KeyInput(_float fTimeDelta)
{
}

void CPlayer::AirCombo2_KeyInput(_float fTimeDelta)
{
}

void CPlayer::AirCombo3_KeyInput(_float fTimeDelta)
{
}

void CPlayer::NomalCombo1_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit())
	{
		if (GI->Mouse_Down(DIMK_LBUTTON))
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
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit())
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
	}
}

void CPlayer::NomalCombo3_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit())
	{
		if (Input_Direction())
		{
			Set_State(RUN);
		}
	}
}

void CPlayer::NomalCombo4_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit())
	{
		if (Input_Direction())
		{
			Set_State(RUN);
		}
	}
}

void CPlayer::NomalCombo5_KeyInput(_float fTimeDelta)
{
	if (m_pAnimModel[0]->GetPlayTime() >= m_pAnimModel[0]->GetTimeLimit())
	{
		if (GI->Mouse_Down(DIMK_RBUTTON))
		{
			Set_State(NOMALCOMBO6);
			Input_Direction();
		}
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
