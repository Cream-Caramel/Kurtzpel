#include "stdafx.h"
#include "..\Public\DragonFire1.h"
#include "GameInstance.h"
#include "Pointer_Manager.h"
#include "OBB.h"
#include "Collider_Manager.h"

CDragonFire1::CDragonFire1(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CDragonFire1::CDragonFire1(const CDragonFire1 & rhs)
	: CGameObject(rhs)
{
}

HRESULT CDragonFire1::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CDragonFire1::Initialize(void * pArg)
{
	m_DragonFire1Info = (*(DRAGONFIRE1INFO*)pArg);
			
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"DragonFire1", L"DragonFire1", (CComponent**)&m_pTextureCom)))
		return E_FAIL;
	


	if (FAILED(Ready_Components()))
		return E_FAIL;

	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vSize = _float3(0.15f, 0.15f, 0.15f);
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Monster_Attack";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB"), (CComponent**)&m_pOBB, &ColliderDesc)))
		return E_FAIL;


	m_DragonFire1Info.vPosition.x += GI->Get_FloatRandom(-2.f, 2.f);
	m_DragonFire1Info.vPosition.y += GI->Get_FloatRandom(0.f,2.f);
	m_DragonFire1Info.vPosition.z += GI->Get_FloatRandom(-2.f, 2.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_DragonFire1Info.vPosition));
	m_pTransformCom->Set_Scale(_vector{ 15.f, 15.f, 15.f});
	m_fDamage = 30.f;
	m_iMaxHit = 1;
	
	m_vTargetLook = m_DragonFire1Info.vDirection;
	return S_OK;
}

void CDragonFire1::Tick(_float fTimeDelta)
{
	
	m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), GI->Get_FloatRandom(20.f, 30.f) , fTimeDelta);

	if (!m_bEnd)
	{
		m_fShaderUVAcc += 1.f * fTimeDelta;
		if (m_fShaderUVAcc >= 0.02f)
		{
			m_fShaderUVAcc = 0.f;
			m_fShaderUVIndexX += 1.f;
			if (m_fShaderUVIndexX >= m_fMaxUVIndexX)
			{
				m_fShaderUVIndexX = 0.f;
				m_fShaderUVIndexY += 1.f;
				if (m_fShaderUVIndexY >= m_fMaxUVIndexY)
				{
					m_fShaderUVIndexY = 0.f;					
				}
				else if (m_fShaderUVIndexY >= m_fMaxUVIndexY - 1.f)
					m_bEnd = true;
			}
		}
	}

	if (m_bEnd)
	{
		m_fEndAcc += 4.f * fTimeDelta;
		if (m_fEndAcc >= 1.f)
			Set_Dead();
	}
	m_pOBB->Update(m_pTransformCom->Get_WorldMatrix());
	
}

void CDragonFire1::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	CM->Add_OBBObject(CCollider_Manager::COLLIDER_MONSTERATTACK, this, m_pOBB);
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);
	//m_pRendererCom->Add_DebugGroup(m_pOBB);


}

HRESULT CDragonFire1::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	
	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4));

	m_pShaderCom->Set_RawValue("g_vCamPosition", &GI->Get_CamPosition(), sizeof(_float4));

	if (m_pTextureCom != nullptr)
	{
		if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;
	}

	
	m_pShaderCom->Set_RawValue("g_fMaxUVIndexX", &m_fMaxUVIndexX, sizeof(_float));
	m_pShaderCom->Set_RawValue("g_fMaxUVIndexY", &m_fMaxUVIndexY, sizeof(_float));
	m_pShaderCom->Set_RawValue("g_fUVIndexX", &m_fShaderUVIndexX, sizeof(_float));
	m_pShaderCom->Set_RawValue("g_fUVIndexY", &m_fShaderUVIndexY, sizeof(_float));
	if (m_bEnd)
	{
		m_pShaderCom->Set_RawValue("g_fEndAcc", &m_fEndAcc, sizeof(_float));
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
	}

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}



HRESULT CDragonFire1::Ready_Components()
{
	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_EffectPoint"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Particle"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

CDragonFire1 * CDragonFire1::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CDragonFire1*		pInstance = new CDragonFire1(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CDragonFire1"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CDragonFire1::Clone(void * pArg)
{
	CDragonFire1*		pInstance = new CDragonFire1(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CDragonFire1"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDragonFire1::Free()
{
	__super::Free();
	
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pOBB);

}
