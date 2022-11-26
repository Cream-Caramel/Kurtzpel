#include "stdafx.h"
#include "..\Public\TRFloor.h"
#include "GameInstance.h"
#include "ModelsInstance.h"
CTRFloor::CTRFloor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMeshInstance(pDevice, pContext)
{
	m_pModel = nullptr;
}

CTRFloor::CTRFloor(const CTRFloor& rhs)
	: CMeshInstance(rhs)
{
}

HRESULT CTRFloor::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CTRFloor::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STAGE1, m_MeshInfo->sTag, m_MeshInfo->sTag, (CComponent**)&m_pModel)))
		return E_FAIL;


	return S_OK;
}

void CTRFloor::Tick(_float fTimeDelta)
{
}

void CTRFloor::LateTick(_float fTimeDelta)
{
	//m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CTRFloor::Render()
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
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pModel->SetUp_OnShader(m_pShaderCom, m_pModel->Get_MaterialIndex(i), TEX_NORMALS, "g_NormalTexture")))
		{
			if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_NDEFAULT)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Begin(INSTANCEMODEL_DEFAULT)))
				return E_FAIL;
		}

	

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CTRFloor::Render_ShadowDepth()
{
	_matrix		LightViewMatrix;
	LightViewMatrix = XMMatrixTranspose(GI->Get_LightMatrix());

	_float4x4		WorldMatrix;
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_LightViewMatrix", &LightViewMatrix, sizeof(_float4x4))))
		return E_FAIL;

	_matrix		LightProjMatrix;
	LightProjMatrix = XMMatrixTranspose(GI->Get_TransformMatrix(CPipeLine::D3DTS_PROJ));

	if (FAILED(m_pShaderCom->Set_RawValue("g_LightProjMatrix", &LightProjMatrix, sizeof(_float4x4))))
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

CMeshInstance * CTRFloor::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTRFloor*		pInstance = new CTRFloor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTRFloor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTRFloor::Clone(void * pArg)
{
	CTRFloor*		pInstance = new CTRFloor(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTRFloor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTRFloor::Free()
{
	__super::Free();
	Safe_Release(m_pModel);
}
