#include "stdafx.h"
#include "..\Public\UI.h"
#include "GameInstance.h"

CUI::CUI(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext),m_UITag(nullptr)
{
}

CUI::CUI(const CUI & rhs)
	: CGameObject(rhs)
{
}

HRESULT CUI::Initialize_Prototype()
{
	m_UIInfo = new UIINFO;
	m_UIInfo->UIPosX = 10.f;
	_tchar* Path = new _tchar[256];
	_tchar* Name = new _tchar[256];
	TexPath = Path;
	TexName = Name;
	return S_OK;
}

HRESULT CUI::Initialize(void * pArg)
{	
	
	m_fSizeX = 300.f;
	m_fSizeY = 300.f;

	m_fX = 640.f;
	m_fY = 360.f;
	

	m_UIInfo = *(UIINFO**)pArg;
	_tchar* Path = new _tchar[256];
	_tchar* Name = new _tchar[256];
	for (int i = 0; i < 256; ++i)
	{
		Path[i] = m_UIInfo->TexPath[i];
		Name[i] = m_UIInfo->TexName[i];
	}
	TexPath = Path;
	TexName = Name;
	m_iTexNum = m_UIInfo->TexNum;
	m_fSizeX = m_UIInfo->UISizeX;
	m_fSizeY = m_UIInfo->UISizeY;
	m_fX = m_UIInfo->UIPosX;
	m_fY = m_UIInfo->UIPosY;

	m_UIInfo = new UIINFO;
	m_UIInfo->TexPath = TexPath;
	m_UIInfo->TexName = TexName;
	m_UIInfo->TexNum = m_iTexNum;
	m_UIInfo->UISizeX = m_fSizeX;
	m_UIInfo->UISizeY = m_fSizeY;
	m_UIInfo->UIPosX = m_fX;
	m_UIInfo->UIPosY = m_fY;

	m_bDead = false;
	
	int _TexNameSize = WideCharToMultiByte(CP_ACP, 0, m_UIInfo->TexName, -1, NULL, 0, NULL, NULL);
	
	m_UITag = new char[_TexNameSize];

	WideCharToMultiByte(CP_ACP, 0, m_UIInfo->TexName, -1, m_UITag, _TexNameSize, 0, 0);
	
	GI->Add_Prototype(LEVEL_GAMEPLAY, m_UIInfo->TexName,
		CTexture::Create(m_pDevice, m_pContext, m_UIInfo->TexPath, m_UIInfo->TexNum));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, m_UIInfo->TexName, m_UIInfo->TexName, (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	Safe_Delete(m_UITag);

	return S_OK;
}

void CUI::Tick(_float fTimeDelta)
{
	//if (m_bDead)
	m_Cool += 1.f * fTimeDelta;
	if (m_Cool > 5.f)
	{
		m_Cool = 0.f;
	}
	m_UIInfo->TexName;
	m_pTransformCom->Set_Scale(XMVectorSet(m_fSizeX, m_fSizeY, 1.f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.0f, 1.f));
}

void CUI::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	temp = m_Cool / 5.f;
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);
	m_pTextureCom->Get_Texture(0);
	
}

HRESULT CUI::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	int a = m_dwRefCnt;
	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4(CPipeLine::D3DTS_IDENTITY), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_UIPROJ), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_fPercent", &temp, sizeof(float));
	
	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (m_bSelect)
	{
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
	}
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

ID3D11ShaderResourceView * CUI::Get_SRV(int Texnum)
{
	return m_pTextureCom->Get_Texture(Texnum); 
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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
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

	Safe_Delete(m_UIInfo);
	Safe_Delete(TexPath);
	Safe_Delete(TexName);
		
	
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pTextureCom);

}
