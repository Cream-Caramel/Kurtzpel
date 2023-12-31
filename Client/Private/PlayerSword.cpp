#include "stdafx.h"
#include "..\Public\PlayerSword.h"
#include "GameInstance.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "Pointer_Manager.h"
#include "Player.h"
#include "Trail.h"
#include "Particle_Manager.h"
#include "PlayerHit1.h"

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
	m_fMaxHp = 0.1f;
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

	m_bColliderRender = false;
	m_bCollision = false;
	
	//등 보정값
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ -0.3f,0.f,-0.4f,1.f });
	m_pTransformCom->Set_Scale(_vector{ 1.f,1.f,1.f });
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 95.f, _float3{ 0.f,1.f,0.f }, 27.f, _float3{ 0.f,0.f,1.f }, 20.f);

	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(0.3f, 3.6f, 0.3f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(45.f), 0.f);
	ColliderDesc.sTag = "Player_Sword";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Sword"), (CComponent**)&m_pOBB, &ColliderDesc)))
		return E_FAIL;

	m_fOutLinePower = 3.f;
	m_pPlayer = PM->Get_PlayerPointer();

	return S_OK;
}

void CPlayerSword::Tick(_float fTimeDelta)
{
	if (GI->Key_Down(DIK_0))
		m_bColliderRender = !m_bColliderRender;

	if (GI->Key_Down(DIK_P))
	{

		/*_float4 OriginPos;
		_float4 ParentPos;
		XMStoreFloat4(&OriginPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		XMStoreFloat4(&ParentPos, m_pParentTransformCom->Get_State(CTransform::STATE_POSITION));*/
		// * m_pParentTransformCom->Get_WorldMatrix()
		/*_matrix _WorldMatrix;
		_WorldMatrix = m_pTransformCom->Get_WorldMatrix();
		CPlayerHit1::PLAYERHIT1INFO PlayerHit1Info;
		PlayerHit1Info.vWorldMatrix = _WorldMatrix;
		GI->Add_GameObjectToLayer(L"PlayerHit1", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerHit1Info);*/

		/*_float3 Center = m_pOBB->Get_Center();
		
		XMStoreFloat3(&Center, XMVector3TransformCoord(XMLoadFloat3(&Center), _WorldMatrix));
	
		_float4 WorldPos;		
		WorldPos.x = Center.x;
		WorldPos.y = Center.y;
		WorldPos.z = Center.z;
		WorldPos.w = 1.f;*/



		//PTM->CreateParticle(L"Hit", WorldPos, true, CAlphaParticle::DIR_END);
	}


}

void CPlayerSword::LateTick(_float fTimeDelta)
{
	
	if (m_bCollision)
		CM->Add_OBBObject(CCollider_Manager::COLLIDER_PLAYERSWORD, this, m_pOBB);
		
	m_pOBB->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

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
		
		if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrixInverse", &XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&WorldMatrix))), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrixInverse", &GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
			return E_FAIL;

		m_pShaderCom->Set_RawValue("g_fOutLinePower", &m_fOutLinePower, sizeof(_float));

		if (FAILED(m_pShaderCom->Begin(MODEL_OUTLINE)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(MODEL_NDEFAULT)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}

	
	if (FAILED(m_pTexture->Set_SRV(m_pTexShader, "g_DiffuseTexture")))
		return E_FAIL;
	//m_pTrail->Render();
	return S_OK;
}



HRESULT CPlayerSword::Render_ShadowDepth()
{
	if (m_pModel != nullptr)
	{
		_uint		iNumMeshes = m_pModel->Get_NumMeshes();
		for (_uint j = 0; j < iNumMeshes; ++j)
		{
			
			_matrix		LightViewMatrix;

			LightViewMatrix = XMMatrixTranspose(GI->Get_LightMatrix());

			_float4x4		WorldMatrix;

			XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()));

			if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Set_RawValue("g_LightViewMatrix", &LightViewMatrix, sizeof(_float4x4))))
				return E_FAIL;

			_matrix Fov60 = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (_float)1280.f / 720.f, 0.2f, 300.f);

			if (FAILED(m_pShaderCom->Set_RawValue("g_LightProjMatrix", &XMMatrixTranspose(Fov60), sizeof(_float4x4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(MODEL_SHADOW)))
				return E_FAIL;			

			if (FAILED(m_pModel->Render(j)))
				return E_FAIL;					
		}
	}
	
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
		
	
		/*_matrix _WorldMatrix;
		_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix();
		CPlayerHit1::PLAYERHIT1INFO PlayerHit1Info;
		PlayerHit1Info.vWorldMatrix = _WorldMatrix;
		GI->Add_GameObjectToLayer(L"PlayerHit1", PM->Get_NowLevel(), L"Layer_PlayerEffect", &PlayerHit1Info);*/

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
