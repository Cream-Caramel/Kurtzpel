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

	m_iAniIndex = 0;


	for (int i = 0; i < MODEL_END; ++i)
	{
		m_pAnimModel[i]->Set_AnimIndex(m_iAniIndex);
	}
		
	Ready_Sockets();
	Ready_PlayerParts();
	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_UP))
	{
		++m_iAniIndex;
		for (auto& iter : m_pAnimModel)
		{
			iter->SetNextIndex(m_iAniIndex);
			iter->SetChangeBool(true);
		}
	}
	Update_Parts();

	for (auto& pPart : m_Parts)
		pPart->Tick(fTimeDelta);
}

void CPlayer::LateTick(_float fTimeDelta)
{
	for(int i = 0; i < MODEL_END; ++i)
	{
		if(m_pAnimModel[i] != nullptr)
			m_pAnimModel[i]->Play_Animation(fTimeDelta, m_pAnimModel[i]);
	}

	for (auto& pPart : m_Parts)
		pPart->LateTick(fTimeDelta);


	for (auto& pPart : m_Parts)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, pPart);

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	/*m_pModelCom->SetNextIndex(m_iAniIndex);
	m_pModelCom->SetChangeBool(true);*/

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
