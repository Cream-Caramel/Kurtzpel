#include "stdafx.h"
#include "..\Public\keyF.h"


CKeyF::CKeyF(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	:CUI(pDevice, pContext)
{
}

CKeyF::CKeyF(const CKeyF & rhs)
	: CUI(rhs)
{
}

HRESULT CKeyF::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKeyF::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	return S_OK;
}

void CKeyF::Tick(_float fTimeDelta)
{

}

void CKeyF::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CKeyF::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));

	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CKeyF * CKeyF::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CKeyF*		pInstance = new CKeyF(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CKeyF"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CKeyF::Clone(void * pArg)
{
	CKeyF*		pInstance = new CKeyF(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CKeyF"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKeyF::Free()
{
	__super::Free();
}
