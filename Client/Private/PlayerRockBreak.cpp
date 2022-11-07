#include "stdafx.h"
#include "..\Public\PlayerRockBreak.h"
#include "GameInstance.h"

CPlayerRockBreak::CPlayerRockBreak(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	m_pAnimModel = nullptr;
}

CPlayerRockBreak::CPlayerRockBreak(const CPlayerRockBreak& rhs)
	: CAnimMesh(rhs)
{
}

HRESULT CPlayerRockBreak::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerRockBreak::Initialize(void * pArg)
{
	EFFECTINFO* EffectInfo = ((EFFECTINFO*)pArg);

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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	m_bDead = false;
	
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerRockBreak", TEXT("PlayerRockBreak"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(EffectInfo->WorldMatrix);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&EffectInfo->vScale));

	return S_OK;
}

void CPlayerRockBreak::Tick(_float fTimeDelta)
{
	m_pAnimModel->Play_Animation(fTimeDelta, m_pAnimModel);
	
}

void CPlayerRockBreak::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_pAnimModel->GetAniEnd())
		this->Set_Dead();
	if(!m_bDead)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	

}

HRESULT CPlayerRockBreak::Render()
{
	if (nullptr == m_pAnimModel ||
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

	_uint		iNumMeshes = m_pAnimModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(i), TEX_NORMALS, "g_NormalTexture")))
		{
			m_bNormalTex = false;
			m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
		}
		else
		{
			m_bNormalTex = true;
			m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
		}

		if (FAILED(m_pAnimModel->Render(m_pShaderCom, i)))
			return E_FAIL;
	}
	return S_OK;
}

CAnimMesh * CPlayerRockBreak::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerRockBreak*		pInstance = new CPlayerRockBreak(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerRockBreak"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerRockBreak::Clone(void * pArg)
{
	CPlayerRockBreak*		pInstance = new CPlayerRockBreak(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerRockBreak"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerRockBreak::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel);

}
