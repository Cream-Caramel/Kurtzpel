#include "stdafx.h"
#include "..\Public\PlayerGage.h"
#include "GameInstance.h"
#include "Pointer_Manager.h"
#include "Player.h"

CPlayerGage::CPlayerGage(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CPlayerGage::CPlayerGage(const CPlayerGage & rhs)
	: CGameObject(rhs)
{
}

HRESULT CPlayerGage::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CPlayerGage::Initialize(void * pArg)
{

	memcpy(&m_ParticleInfo, pArg, sizeof(GAGEINFO));
	
	if (m_ParticleInfo.TexNum > 0)
	{
		_tchar* Path = new _tchar[256];
		_tchar* Name = new _tchar[256];
		for (int i = 0; i < 256; ++i)
		{
			Path[i] = m_ParticleInfo.TexPath[i];
			Name[i] = m_ParticleInfo.TexName[i];
		}
		TexPath = Path;
		TexName = Name;

		m_ParticleInfo.TexPath = TexPath;
		m_ParticleInfo.TexName = TexName;
		
		if (FAILED(__super::Add_Component(LEVEL_STATIC, m_ParticleInfo.TexName, m_ParticleInfo.TexName, (CComponent**)&m_pTextureCom)))
			return E_FAIL;
	}
	_float4 WorldPos{ m_ParticleInfo.vPosition.x + m_ParticleInfo.vWorldPos.x, m_ParticleInfo.vPosition.y + m_ParticleInfo.vWorldPos.y, m_ParticleInfo.vPosition.z + m_ParticleInfo.vWorldPos.z, 1.f };
	
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_iNowFrame = 0;
	m_pTransformCom->Rotation(_vector{ 1.f,0.f,0.f }, 90.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&WorldPos));
	m_pTransformCom->Set_Scale(_vector{ m_ParticleInfo.vSize.x, m_ParticleInfo.vSize.y });
	
	return S_OK;
}

void CPlayerGage::Tick(_float fTimeDelta)
{
	m_pTransformCom->Turn(_vector{ 0.f,1.f,0.f }, fTimeDelta);
	if (!PM->Get_PlayerGage())
	{
		m_pTransformCom->Set_Scale(XMLoadFloat3(&m_pTransformCom->Get_Scale()) - _vector{ 0.2f,0.2f,0.2f });
	}
}

void CPlayerGage::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	
	if(m_pTransformCom->Get_Scale().x <= 0.2f)
		Set_Dead();
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

}

HRESULT CPlayerGage::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4));

	//m_pShaderCom->Set_RawValue("g_vCamPosition", &GI->Get_CamPosition(), sizeof(_float4));

	if (m_pTextureCom != nullptr)
	{
		if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", m_iNowFrame)))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Begin(PASS_SKILLICON)))
		return E_FAIL;
	

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}


HRESULT CPlayerGage::Ready_Components()
{
	CTransform::TRANSFORMDESC TransformInfo;
	TransformInfo.fRotationPerSec = 6.f;

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &TransformInfo)))
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

CPlayerGage * CPlayerGage::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerGage*		pInstance = new CPlayerGage(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerGage"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerGage::Clone(void * pArg)
{
	CPlayerGage*		pInstance = new CPlayerGage(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerGage"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerGage::Free()
{
	__super::Free();
	if (m_ParticleInfo.TexNum > 0)
	{
		delete m_ParticleInfo.TexPath;
		delete m_ParticleInfo.TexName;
	}
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pTextureCom);

}
