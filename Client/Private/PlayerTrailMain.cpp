#include "stdafx.h"
#include "..\Public\PlayerTrailMain.h"
#include "GameInstance.h"

CPlayerTrailMain::CPlayerTrailMain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMesh(pDevice, pContext)
{
	m_pModel = nullptr;
}

CPlayerTrailMain::CPlayerTrailMain(const CPlayerTrailMain& rhs)
	: CMesh(rhs)
{
}

HRESULT CPlayerTrailMain::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CPlayerTrailMain::Initialize(void * pArg)
{
	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(TransformDesc));

	TransformDesc.fSpeedPerSec = 25.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(1.0f);

	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_EffectModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;

	m_bDead = false;
	
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, L"PlayerTrailMain", TEXT("PlayerTrailMain"), (CComponent**)&m_pModel)))
		return E_FAIL;

	m_fShaderUVAcc = 0.f;
	m_fUVCount = 0.1f;
	m_iShaderUVIndex = 0;
	m_iMaxUVIndex = 3;

	//m_pTransformCom->Set_State(CTransform::STATE_RIGHT, )

	/*m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ 0.f,0.f,0.f,1.f });

	m_pTransformCom->RotationThree(_float3{ 1.f,0.f,0.f }, 130.f, _float3{ 0.f,1.f,0.f }, 200.f, _float3{ 0.f,0.f,1.f },0.f);*/

	return S_OK;
}

void CPlayerTrailMain::Tick(_float fTimeDelta)
{

}

void CPlayerTrailMain::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_bRenderObj)
	{
		m_fRenderLimitAcc += 1.f * fTimeDelta;
		if (m_fRenderLimitAcc >= m_fRenderLimit)
		{
			m_bRenderObj = false;
			m_fRenderLimitAcc = 0.f;
		}
		m_fShaderUVAcc += 1.f * fTimeDelta;
		if (m_fShaderUVAcc >= m_fUVCount)
		{
			m_fShaderUVAcc = 0.f;
			m_iShaderUVIndex += 1;
			if (m_iShaderUVIndex > m_iMaxUVIndex)
				m_iShaderUVIndex = 0;
		}

		if (m_eTurnDir == TURN_FRONT)
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_UP), fTimeDelta);
		else
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_UP) * -1, fTimeDelta);

		if (m_fMoveSpeed != 0.f)
		{
			if (m_fMoveSpeed >= m_fMoveSpeedTempo)
				m_fMoveSpeed -= m_fMoveSpeedTempo;
			m_pTransformCom->Go_Dir(XMLoadFloat3(&m_vTargetLook), m_fMoveSpeed, fTimeDelta);
		}


		Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);
	}

}

HRESULT CPlayerTrailMain::Render()
{
	if (nullptr == m_pModel ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	_float4x4		WorldMatrix;

	XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()));

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
			m_bNormalTex = false;
			m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
		}
		else
		{
			m_bNormalTex = true;
			m_pShaderCom->Set_RawValue("g_bNormalTex", &m_bNormalTex, sizeof(bool));
		}

		m_pShaderCom->Set_RawValue("g_iUVIndex", &m_iShaderUVIndex, sizeof(_uint));

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

CMesh * CPlayerTrailMain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayerTrailMain*		pInstance = new CPlayerTrailMain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayerTrailMain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayerTrailMain::Clone(void * pArg)
{
	CPlayerTrailMain*		pInstance = new CPlayerTrailMain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayerTrailMain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayerTrailMain::Free()
{
	__super::Free();
	Safe_Release(m_pModel);

}
