#include "stdafx.h"
#include "..\Public\MeshInstance.h"
#include "GameInstance.h"

CMeshInstance::CMeshInstance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CMeshInstance::CMeshInstance(const CMeshInstance & rhs)
	: CGameObject(rhs)
{
}

HRESULT CMeshInstance::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeshInstance::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bDead = false;
	return S_OK;
}

void CMeshInstance::Tick(_float fTimeDelta)
{
}

void CMeshInstance::LateTick(_float fTimeDelta)
{
	
}


HRESULT CMeshInstance::Ready_Components()
{

	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(TransformDesc));

	TransformDesc.fSpeedPerSec = 25.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_ModelInstance"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;

	return S_OK;
}

CMeshInstance * CMeshInstance::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CMeshInstance*		pInstance = new CMeshInstance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CMeshInstance"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CMeshInstance::Clone(void * pArg)
{
	CMeshInstance*		pInstance = new CMeshInstance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMeshInstance"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshInstance::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pParentTransformCom);
}
