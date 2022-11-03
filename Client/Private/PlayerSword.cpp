#include "stdafx.h"
#include "..\Public\PlayerSword.h"
#include "GameInstance.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "Pointer_Manager.h"
#include "Player.h"
#include "Trail.h"

CPlayerSword::CPlayerSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerSword::CPlayerSword(const CPlayerSword& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerSword::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerSword::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerSword", TEXT("PlayerSword"), (CComponent**)&m_pModel)))
		return E_FAIL;

	CTrail::TRAILPOS TrailPos;
	TrailPos.vHigh = { 0.f,2.1f,0.f };
	TrailPos.vLow = { 0.f,0.f,0.f };
	
	CTrail::TRAILINFO TrailInfo;
	TrailInfo._HighAndLow = TrailPos;
	TrailInfo._Color = _float4{ 0.3f,0.3f,0.3f,1.f };
	
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Prototype_Component_Trail", TEXT("SwordTrail"), (CComponent**)&m_pTrail,&TrailInfo)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"SwordTrail01", TEXT("SwordTrail01"), (CComponent**)&m_pTexture)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"Prototype_Component_Shader_UI", TEXT("Shader_UI"), (CComponent**)&m_pTexShader)))
		return E_FAIL;

	m_bColliderRender = true;
	m_bCollision = false;
	
	//등 보정값
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ -0.3f,0.f,-0.4f,1.f });
	m_pTransformCom->Set_Scale(_vector{ 1.f,1.f,1.f });
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 95.f, _float3{ 0.f,1.f,0.f }, 27.f, _float3{ 0.f,0.f,1.f }, 20.f);

	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(0.3f, 2.2f, 0.3f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(45.f), 0.f);
	ColliderDesc.sTag = "Player_Sword";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Sword"), (CComponent**)&m_pOBB, &ColliderDesc)))
		return E_FAIL;

	m_pPlayer = PM->Get_PlayerPointer();

	return S_OK;
}

void CPlayerSword::Tick(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_0))
		m_bColliderRender = !m_bColliderRender;
}

void CPlayerSword::LateTick(_float fTimeDelta)
{
	_float4x4		WorldMatrix;
	if (m_bCollision)
		CM->Add_OBBObject(CCollider_Manager::COLLIDER_PLAYERSWORD, this, m_pOBB);
		
	m_pOBB->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	_matrix _WorldMatrix;
	_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix();

	if (!m_bTrail)
		m_pTrail->TrailOff();

	if (!m_pTrail->Get_On())
		m_pTrail->TrailOn(_WorldMatrix);
	m_pTrail->Tick(fTimeDelta, _WorldMatrix);
}

HRESULT CPlayerSword::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	_float4x4		WorldMatrix;

	XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()));

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);



	_uint		iNumMeshes = m_pModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_NORMALS, "g_NormalTexture")))
		{
			m_bNomalTex = false;
			m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNomalTex, sizeof(bool));
		}
		else
		{
			m_bNomalTex = true;
			m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNomalTex, sizeof(bool));
		}

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	if(m_bColliderRender)
		m_pOBB->Render();

	
	if (FAILED(m_pTexture->Set_SRV(m_pTexShader, "g_DiffuseTexture")))
		return E_FAIL;
	m_pTrail->Render();
	return S_OK;
}

void CPlayerSword::Collision(CGameObject * pOther, string sTag)
{
	if (sTag == "Monster_Body")
	{

		GI->PlaySoundW(L"SwordHit.ogg", SD_PLAYER1, 0.6f);
		_float4 vPos;
		vPos.x = m_pOBB->Get_Collider().Center.x;
		vPos.y = m_pOBB->Get_Collider().Center.y + 3.f;
		vPos.z = m_pOBB->Get_Collider().Center.z;
		vPos.w = 1.f;
		GI->Set_StaticLight(0.2f, 8.f, vPos, 0);
		CRM->Start_Shake(0.2f, 2.f, 0.03f);
	}
	

}

void CPlayerSword::Set_OBB(_float3 vSize)
{
	m_pOBB->ChangeExtents(_float3{ vSize.x, vSize.y, vSize.z });
}

void CPlayerSword::Set_RHand()
{
	//오른손 보정값
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ 0.05f,0.f,0.f,1.f });
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 90.f, _float3{ 0.f,1.f,0.f }, 180.f, _float3{ 0.f,0.f,1.f }, 0.f);
}

void CPlayerSword::Set_Spine()
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ -0.3f,0.f,-0.4f,1.f });
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 95.f, _float3{ 0.f,1.f,0.f }, 27.f, _float3{ 0.f,0.f,1.f }, 20.f);
}

void CPlayerSword::Set_Trail(_bool bTrail)
{
	m_bTrail = bTrail;
}



CPlayerSword * CPlayerSword::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerSword*		pInstance = new CPlayerSword(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerSword"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerSword::Clone(void * pArg)
{
	CPlayerSword*		pInstance = new CPlayerSword(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerSword"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerSword::Free()
{
	__super::Free();
	Safe_Release(m_pModel);
	Safe_Release(m_pOBB);
	Safe_Release(m_pTexShader);
	Safe_Release(m_pTrail);
	Safe_Release(m_pTexture);
}
