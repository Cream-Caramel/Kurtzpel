#include "stdafx.h"
#include "..\Public\UI.h"


CUI::CUI(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CUI::CUI(const CUI & rhs)
	: CGameObject(rhs)
{
}

HRESULT CUI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI::Initialize(void * pArg)
{	
	m_UIInfo = *(UIINFO**)pArg;
	
	m_iUIIndex = m_UIInfo->UIIndex;
	m_fSizeX = m_UIInfo->UISizeX;
	m_fSizeY = m_UIInfo->UISizeY;
	m_fX = m_UIInfo->UIPosX;
	m_fY = m_UIInfo->UIPosY;

	m_bDead = false;

	if (FAILED(__super::Add_Component(m_UIInfo->eLevel, m_UIInfo->TexName, m_UIInfo->TexName, (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scale(XMVectorSet(m_fSizeX, m_fSizeY, 1.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.0f, 1.f));

	return S_OK;
}

void CUI::Tick(_float fTimeDelta)
{

}

void CUI::LateTick(_float fTimeDelta)
{
	
}

HRESULT CUI::Render()
{
	return S_OK;
	
}


HRESULT CUI::Ready_Components()
{
	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_UI"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

CUI * CUI::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CUI*		pInstance = new CUI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CUI::Clone(void * pArg)
{
	CUI*		pInstance = new CUI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CUI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI::Free()
{	
	
	__super::Free();
		
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pTextureCom);

}
