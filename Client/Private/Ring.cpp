#include "stdafx.h"
#include "..\Public\Ring.h"
#include "GameInstance.h"

CRing::CRing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CRing::CRing(const CRing& rhs)
	: CMesh(rhs)
{
}

HRESULT CRing::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CRing::Initialize(void * pArg)
{
	m_RingInfo = (*(RINGINFO*)pArg);

	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(TransformDesc));

	TransformDesc.fSpeedPerSec = 25.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(1.0f);

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_EffectModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;

	m_bDead = false;
	
	/* For.Com_Model */

	m_fMaxUVIndexX = 0.f;

	m_pTransformCom->Set_Scale(_vector{ m_RingInfo.vSize.x, m_RingInfo.vSize.y, m_RingInfo.vSize.z });
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"RingOrange", TEXT("RingOrange"), (CComponent**)&m_pModel)))
			return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_RingInfo.vWorldPos));

	return S_OK;
}

void CRing::Tick(_float fTimeDelta)
{
	m_fLifeTimeAcc += 1.f * fTimeDelta;
	m_pTransformCom->Set_Scale(XMLoadFloat3(&m_pTransformCom->Get_Scale()) + _vector{ m_RingInfo.fSpeed, m_RingInfo.vSize.y,m_RingInfo.fSpeed });
	if (m_fLifeTimeAcc >= 0.3f)
		Set_Dead();
}

void CRing::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_FRONTALPHA, this);
	

}

HRESULT CRing::Render()
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


		m_pShaderCom->Set_RawValue("g_fMaxUVIndexX", &m_fMaxUVIndexX, sizeof(_float));

		if (FAILED(m_pShaderCom->Begin(EFFECT_NDEFAULT)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

CMesh * CRing::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CRing*		pInstance = new CRing(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CRing"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CRing::Clone(void * pArg)
{
	CRing*		pInstance = new CRing(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CRing"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRing::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

}
