#include "stdafx.h"
#include "..\Public\TRSky.h"
#include "GameInstance.h"
#include "ModelsInstance.h"
#include "Pointer_Manager.h"
CTRSky::CTRSky(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMeshInstance(pDevice, pContext)
{
	m_pModel = nullptr;
}

CTRSky::CTRSky(const CTRSky& rhs)
	: CMeshInstance(rhs)
{
}

HRESULT CTRSky::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CTRSky::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STAGE1, m_MeshInfo->sTag, m_MeshInfo->sTag, (CComponent**)&m_pModel)))
		return E_FAIL;

	return S_OK;
}

void CTRSky::Tick(_float fTimeDelta)
{
}

void CTRSky::LateTick(_float fTimeDelta)
{
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_PRIORITY, this);
}

HRESULT CTRSky::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	_matrix		WorldMatrix;
	WorldMatrix = XMMatrixIdentity();

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &XMMatrixTranspose(WorldMatrix), sizeof(_float4x4))))
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

CMeshInstance * CTRSky::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTRSky*		pInstance = new CTRSky(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTRSky"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTRSky::Clone(void * pArg)
{
	CTRSky*		pInstance = new CTRSky(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTRSky"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTRSky::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

	

}
