#include "stdafx.h"
#include "..\Public\Theo.h"
#include "GameInstance.h"
#include "OBB.h"

CTheo::CTheo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CAnimMesh(pDevice, pContext)
{
	
}

CTheo::CTheo(const CTheo& rhs)
	:CAnimMesh(rhs)
{
}

HRESULT CTheo::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CTheo::Initialize(void * pArg)
{
	__super::Initialize(pArg);

	m_MeshInfo = ((MESHINFO*)pArg);
	sTag = m_MeshInfo->sTag;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Theo"), (CComponent**)&m_pAnimModel)))
		return E_FAIL;

	m_eCurState = IDLE;
	m_eNextState = IDLE;
	m_vTargetLook = { 0.f,0.f,1.f };

	m_pAnimModel->Set_AnimIndex(m_eCurState);

	m_fMaxHp = 100;
	m_fMaxMp = 100.f;
	m_fNowHp = m_fMaxHp;
	m_fNowMp = m_fMaxMp;

	return S_OK;
}

void CTheo::Tick(_float fTimeDelta)
{
	if (!m_pAnimModel->GetChangeBool())
		m_eCurState = m_eNextState;

	if (GI->Key_Down(DIK_0))
		m_bCollider = !m_bCollider;

	Update(fTimeDelta);
}

void CTheo::LateTick(_float fTimeDelta)
{
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMLoadFloat3(&m_vTargetLook), 0.3f);

	m_pAnimModel->Play_Animation(fTimeDelta, m_pAnimModel);

	End_Animation();

	m_pOBB->Update(m_pTransformCom->Get_WorldMatrix());

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CTheo::Render()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	_uint		iNumMeshes = m_pAnimModel->Get_NumMeshes();
	for (_uint j = 0; j < iNumMeshes; ++j)
	{
		if (FAILED(m_pAnimModel->SetUp_OnShader(m_pShaderCom, m_pAnimModel->Get_MaterialIndex(j), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pAnimModel->Render(m_pShaderCom, j)))
			return E_FAIL;
	}

	m_pOBB->Render();

	return S_OK;
}

void CTheo::Collision(CGameObject * pOther, string sTag)
{
}

HRESULT CTheo::Ready_Collider()
{
	CCollider::COLLIDERDESC		ColliderDesc;

	ColliderDesc.vSize = _float3(0.7f, 2.f, 0.7f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.sTag = "Theo";
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), TEXT("OBB_Body"), (CComponent**)&m_pOBB, &ColliderDesc)))
		return E_FAIL;

	m_pOBBs.push_back(m_pOBB);
	Safe_AddRef(m_pOBB);


	return S_OK;
}

void CTheo::Set_State(STATE eState)
{
}

void CTheo::Set_Dir()
{
}

void CTheo::End_Animation()
{
}

void CTheo::Update(_float fTimeDelta)
{
}

CTheo * CTheo::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTheo*		pInstance = new CTheo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTheo"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTheo::Clone(void * pArg)
{
	CTheo*		pInstance = new CTheo(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTheo"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTheo::Free()
{
	__super::Free();
	Safe_Release(m_pAnimModel);
	Safe_Release(m_pOBB);
}
