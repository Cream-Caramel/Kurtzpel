#include "stdafx.h"
#include "..\Public\Count.h"
#include "Pointer_Manager.h"
#include "UI_Manager.h"
CCount::CCount(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CCount::CCount(const CCount & rhs)
	: CUI(rhs)
{
}

HRESULT CCount::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCount::Initialize(void * pArg)
{
	__super::Initialize(pArg);
	m_iTextureNum = 2;
	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", m_iTextureNum)))
		return E_FAIL;

	UM->Add_Count(this);
	return S_OK;
}

void CCount::Tick(_float fTimeDelta)
{

}

void CCount::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	if(PM->Get_BossFinish())
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CCount::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));
	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", m_iTextureNum)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(PASS_SKILLICON)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

_uint CCount::Get_Count()
{
	return m_iTextureNum;
}

void CCount::Set_Count(_uint iCount)
{
	if (m_iTextureNum + iCount >= 2)
	{
		m_iTextureNum = 2;
	}

	else if (m_iTextureNum + iCount <= 0)
		m_iTextureNum = 0;

	else
	{
		m_iTextureNum += iCount;
	}

	
}

CCount * CCount::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CCount*		pInstance = new CCount(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCount"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCount::Clone(void * pArg)
{
	CCount*		pInstance = new CCount(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CCount"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCount::Free()
{
	__super::Free();
}
