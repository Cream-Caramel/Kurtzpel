#include "stdafx.h"
#include "..\Public\BaseTile.h"
#include "GameInstance.h"
#include "ImGui_Manager.h"
#include "VIBuffer_Tile.h"

CBaseTile::CBaseTile(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CBaseTile::CBaseTile(const CBaseTile & rhs)
	: CGameObject(rhs)
{
}

HRESULT CBaseTile::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBaseTile::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	IG->AddBaseTile(this);

	//D3D11_RASTERIZER_DESC temp;
	//ZeroMemory(&temp, sizeof(D3D11_RASTERIZER_DESC));
	//temp.FillMode = D3D11_FILL_WIREFRAME;
	//temp.CullMode = D3D11_CULL_NONE;
	//temp.DepthClipEnable = true;
	//m_pDevice->CreateRasterizerState(&temp, &m_WireFrame);

	return S_OK;
}

void CBaseTile::Tick(_float fTimeDelta)
{

}

void CBaseTile::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (IG->Get_bMapToolWindow())
	{
		_float3			vPickPos;

		if (GI->Mouse_Pressing(DIMK_LBUTTON))
		{
			CVIBuffer_Tile::VTXINDEX VtxIndex;
			if (m_pVIBufferCom->Picking(m_pTransformCom, &vPickPos, VtxIndex))
			{
				D3D11_MAPPED_SUBRESOURCE		SubResource;
				int itemp = m_pVIBufferCom->GetVtxSize();
				m_pVIBufferCom->Map(&SubResource);
				_float3* temp = m_pVIBufferCom->GetVtxPos();
				temp[VtxIndex.Index[0]].y += 0.1f;
				temp[VtxIndex.Index[1]].y += 0.1f;
				temp[VtxIndex.Index[2]].y += 0.1f;
				memcpy(SubResource.pData, temp, sizeof(_float3)*itemp);
				m_pVIBufferCom->UnMap();
			}
		}		
	}
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CBaseTile::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	ID3D11RasterizerState* m_WireFrame;
	D3D11_RASTERIZER_DESC temp;
	ZeroMemory(&temp, sizeof(D3D11_RASTERIZER_DESC));
	temp.FillMode = D3D11_FILL_WIREFRAME;
	temp.CullMode = D3D11_CULL_NONE;
	temp.DepthClipEnable = true;
	m_pDevice->CreateRasterizerState(&temp, &m_WireFrame);
	
	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;	

	if (FAILED(m_pShaderCom->Begin(2)))
		return E_FAIL;

	m_pContext->RSSetState(m_WireFrame);

	m_pVIBufferCom->Render();

	m_pContext->RSSetState(nullptr);

	m_WireFrame->Release();

	return S_OK;
}

void CBaseTile::Create_Tile(int VtxX, int VtxZ)
{
	m_pVIBufferCom->SetVtxNum(VtxX, VtxZ);
	m_pVIBufferCom->CreateTile();
}

void CBaseTile::SetTileColor(_float4 TileColor)
{
	m_pShaderCom->Set_RawValue("g_fTileColor", &TileColor, sizeof(_float4));
}

HRESULT CBaseTile::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC		TransformDesc;
	TransformDesc.fSpeedPerSec = 5.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(10.0f);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Tile"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBaseTile::SetUp_ShaderResources()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}



CBaseTile * CBaseTile::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBaseTile*		pInstance = new CBaseTile(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBaseTile"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBaseTile::Clone(void * pArg)
{
	CBaseTile*		pInstance = new CBaseTile(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBaseTile"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBaseTile::Free()
{
	__super::Free();
	
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
}
