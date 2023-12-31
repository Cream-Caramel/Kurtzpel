#include "stdafx.h"
#include "..\Public\InstanceModel.h"
#include "GameInstance.h"
#include "ModelsInstance.h"
CInstanceModel::CInstanceModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMeshInstance(pDevice, pContext)
{
	m_pModel = nullptr;
}

CInstanceModel::CInstanceModel(const CInstanceModel& rhs)
	: CMeshInstance(rhs)
{
}

HRESULT CInstanceModel::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CInstanceModel::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_InstanceInfo = ((INSTANCEINFO*)pArg);
	m_eRenderGroup = m_InstanceInfo->eRenderGroup;
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(m_InstanceInfo->eLevel, m_InstanceInfo->sTag, m_InstanceInfo->sTag, (CComponent**)&m_pModel)))
		return E_FAIL;


	return S_OK;
}

void CInstanceModel::Tick(_float fTimeDelta)
{
}

void CInstanceModel::LateTick(_float fTimeDelta)
{
	//if(m_eRenderGroup != CRenderer::RENDER_PRIORITY)
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	m_pRendererCom->Add_RenderGroup(m_eRenderGroup, this);
}

HRESULT CInstanceModel::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	_float4x4		WorldMatrix;
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

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
		if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrixInverse", &XMMatrixInverse(nullptr, XMLoadFloat4x4(&WorldMatrix)), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrixInverse", &GI->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_NORMALS, "g_NormalTexture")))
		{
		/*	if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_OUTLINE)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(i)))
				return E_FAIL;*/

			if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_DEFAULT)))
				return E_FAIL;


		}
		else
		{
		/*	if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_OUTLINE)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(i)))
				return E_FAIL;*/

			if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_NDEFAULT)))
				return E_FAIL;
		}	

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CInstanceModel::Render_ShadowDepth()
{
	_matrix		LightViewMatrix;
	LightViewMatrix = XMMatrixTranspose(GI->Get_LightMatrix());

	_float4x4		WorldMatrix;
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_LightViewMatrix", &LightViewMatrix, sizeof(_float4x4))))
		return E_FAIL;

	/*_matrix		LightProjMatrix;
	LightProjMatrix = XMMatrixTranspose(GI->Get_TransformMatrix(CPipeLine::D3DTS_PROJ));*/

	_matrix Fov60 = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (_float)1280.f / 720.f, 0.2f, 300.f);
	//XMStoreFloat4x4(&ProjMatrixInv, XMMatrixTranspose(Fov60));

	if (FAILED(m_pShaderCom->Set_RawValue("g_LightProjMatrix", &XMMatrixTranspose(Fov60), sizeof(_float4x4))))
		return E_FAIL;


	if (m_pModel != nullptr)
	{
		_uint		iNumMeshes = m_pModel->Get_NumMeshes();
		for (_uint j = 0; j < iNumMeshes; ++j)
		{
			if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_SHADOW)))
				return E_FAIL;

			if (FAILED(m_pModel->Render(j)))
				return E_FAIL;
		}
	}

	return S_OK;
}

CMeshInstance * CInstanceModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CInstanceModel*		pInstance = new CInstanceModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CInstanceModel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CInstanceModel::Clone(void * pArg)
{
	CInstanceModel*		pInstance = new CInstanceModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CInstanceModel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInstanceModel::Free()
{
	__super::Free();
	Safe_Release(m_pModel);
}
