#include "stdafx.h"
#include "..\Public\GolemSkillRock2.h"
#include "GameInstance.h"
#include "AnimMesh.h"
#include "OBB.h"
#include "Collider_Manager.h"
#include "Particle_Manager.h"
#include "Pointer_Manager.h"
CGolemSkillRock2::CGolemSkillRock2(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CGolemSkillRock2::CGolemSkillRock2(const CGolemSkillRock2& rhs)
	: CMesh(rhs)
{
}

HRESULT CGolemSkillRock2::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CGolemSkillRock2::Initialize(void * pArg)
{
	GOLEMROCK2INFO GolemRock2Info = (*(GOLEMROCK2INFO*)pArg);

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_Model"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;

	m_bDead = false;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"GolemSkillRock2", TEXT("GolemSkillRock2"), (CComponent**)&m_pModel)))
		return E_FAIL;

	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(GolemRock2Info.vScale.x * 2.5f, GolemRock2Info.vScale.y * 4.f, GolemRock2Info.vScale.z * 2.5f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Attack";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB"), (CComponent**)&m_pOBB, &ColliderDesc)))
		return E_FAIL;

	m_fDamage = 30.f;
	m_iMaxHit = 1;

	GolemRock2Info.vPos.w = 1.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&GolemRock2Info.vPos));

	m_pTransformCom->Set_Scale(XMLoadFloat3(&GolemRock2Info.vScale));

	m_pTransformCom->Set_TurnSpeed(15.f);
	return S_OK;
}

void CGolemSkillRock2::Tick(_float fTimeDelta)
{

	if (m_bSetDir)
	{
		if (m_fSpeed < 50.f)
			m_fSpeed += 1.f;
		m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetPos), m_fSpeed, fTimeDelta);
		m_pTransformCom->TurnY(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMLoadFloat3(&m_vTargetLook), 0.1f);
	}
	else
		m_pTransformCom->Turn(_vector{ GI->Get_FloatRandom(0.f,1.f),GI->Get_FloatRandom(0.f,1.f), GI->Get_FloatRandom(0.f,1.f) }, fTimeDelta);
	if (m_fUpAcc <= 1.f)
	{
		m_fUpAcc += 1.f * fTimeDelta;
		m_pTransformCom->Go_Dir(_vector{ 0.f,1.f,0.f }, 10.f, fTimeDelta);
	}
	
}

void CGolemSkillRock2::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	m_pOBB->Update(m_pTransformCom->Get_WorldMatrix());
	if (!m_bSetDir && PM->Get_GolemRockOn())
	{
		_float3 RandomPos = PM->Get_PlayerPos();
		RandomPos.x += GI->Get_FloatRandom(-6.f, 6.f);
		RandomPos.z += GI->Get_FloatRandom(-6.f, 6.f);
		XMStoreFloat3(&m_vTargetPos, XMVector3Normalize(XMLoadFloat3(&RandomPos) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
		_vector LookAt = { m_vTargetPos.x * -1, m_vTargetPos.y, m_vTargetPos.z * -1 };
		XMStoreFloat3(&m_vTargetLook, LookAt);
		m_bSetDir = true;	
	}

	if (m_bSetDir)
	{
		CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB);
		if (m_pTransformCom->Get_State(CTransform::STATE_POSITION).m128_f32[1] <= PM->Get_PlayerPos().y + 1.f)
		{
			if (m_pTransformCom->Get_ScaleAxis(CTransform::AXIS_X) >= 0.8f)
			{
				CAnimMesh::EFFECTINFO EffectInfo;
				EffectInfo.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				EffectInfo.vScale = { 1.f,1.f,1.f };
				EffectInfo.bObtion = true;
				_float4 Pos;
				XMStoreFloat4(&Pos, EffectInfo.WorldMatrix.r[3]);
				PTM->CreateParticle(L"GolemSkill3", Pos, false, CAlphaParticle::DIR_END);
				GI->Add_GameObjectToLayer(L"GolemRock3", PM->Get_NowLevel(), L"GolemEffect", &EffectInfo);
				CRM->Start_Shake(0.3f, 4.f, 0.04f);
			}
			Set_Dead();
		}
	}

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CGolemSkillRock2::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
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
			if (FAILED(m_pShaderCom->Begin(MODEL_NDEFAULT)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Begin(MODEL_DEFAULT)))
				return E_FAIL;
		}

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}

	if (m_bColliderRender)
		m_pOBB->Render();
	return S_OK;
}

CMesh * CGolemSkillRock2::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CGolemSkillRock2*		pInstance = new CGolemSkillRock2(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CGolemSkillRock2"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CGolemSkillRock2::Clone(void * pArg)
{
	CGolemSkillRock2*		pInstance = new CGolemSkillRock2(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CGolemSkillRock2"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGolemSkillRock2::Free()
{
	__super::Free();
	Safe_Release(m_pOBB);
	Safe_Release(m_pModel);

}
