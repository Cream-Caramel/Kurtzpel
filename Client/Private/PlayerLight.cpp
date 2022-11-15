#include "stdafx.h"
#include "..\Public\PlayerLight.h"
#include "GameInstance.h"

CPlayerLight::CPlayerLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerLight::CPlayerLight(const CPlayerLight& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerLight::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerLight::Initialize(void * pArg)
{
	PLAYERLIGHT PlayerLightInfo = (*(PLAYERLIGHT*)pArg);

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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerLight", TEXT("PlayerLight"), (CComponent**)&m_pModel)))
		return E_FAIL;
	PlayerLightInfo.vPos.w = 1.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&PlayerLightInfo.vPos));
	
	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, PlayerLightInfo.vAngle.x, _float3{ 0.f,1.f,0.f }, PlayerLightInfo.vAngle.y, _float3{ 0.f,0.f,1.f }, PlayerLightInfo.vAngle.z);

	m_pTransformCom->Set_Scale(XMLoadFloat3(&PlayerLightInfo.vScale));

	m_fCloseSpeed = PlayerLightInfo.fCloseSpeed;
	return S_OK;
}

void CPlayerLight::Tick(_float fTimeDelta)
{
	m_pTransformCom->Set_ScaleAxis(CTransform::AXIS_X, m_pTransformCom->Get_ScaleAxis(CTransform::AXIS_X) - m_fCloseSpeed);
	m_pTransformCom->Set_ScaleAxis(CTransform::AXIS_Z, m_pTransformCom->Get_ScaleAxis(CTransform::AXIS_Z) - m_fCloseSpeed);
}

void CPlayerLight::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_pTransformCom->Get_ScaleAxis(CTransform::AXIS_X) <= m_fCloseSpeed || m_pTransformCom->Get_ScaleAxis(CTransform::AXIS_Z) <= m_fCloseSpeed)
		Set_Dead();

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CPlayerLight::Render()
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
	
		if (FAILED(m_pShaderCom->Begin(MODEL_NDEFAULT)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

CMesh * CPlayerLight::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerLight*		pInstance = new CPlayerLight(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerLight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerLight::Clone(void * pArg)
{
	CPlayerLight*		pInstance = new CPlayerLight(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerLight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerLight::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

}
