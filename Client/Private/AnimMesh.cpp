#include "stdafx.h"
#include "..\Public\AnimMesh.h"
#include "AnimModel.h"
#include "GameInstance.h"


CAnimMesh::CAnimMesh(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CAnimMesh::CAnimMesh(const CAnimMesh & rhs)
	: CGameObject(rhs)
{
}

HRESULT CAnimMesh::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimMesh::Initialize(void * pArg)
{
	m_MeshInfo = ((MESHINFO*)pArg);
	sTag = m_MeshInfo->sTag;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, m_MeshInfo->sTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_bDead = false;
	m_MeshInfo->fPos.w = 1.f;
	Set_Pos(m_MeshInfo->fPos);
	m_pTransformCom->Set_Scale(XMLoadFloat3(&m_MeshInfo->fScale));
	m_fAngles = m_MeshInfo->fAngle;
	m_iPreAniIndex = 0;
	if(m_pModelCom->Get_NumAnimations() > 0)
	m_pModelCom->Set_AnimIndex(0);

	return S_OK;
}

void CAnimMesh::Tick(_float fTimeDelta)
{
	m_iPreAniIndex = m_iAniIndex;
	
}

void CAnimMesh::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	if (m_iPreAniIndex != m_iAniIndex)
	{
		m_pModelCom->SetNextIndex(m_iAniIndex);
		m_pModelCom->SetChangeBool(true);
	}
	m_pModelCom->Play_Animation(fTimeDelta, m_pModelCom);			
	
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CAnimMesh::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;


	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;



	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), TEX_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(m_pShaderCom, i)))
			return E_FAIL;
	}

	return S_OK;
}

void CAnimMesh::Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3)
{
	m_fAngles.x = fRadian;
	m_fAngles.y = fRadian2;
	m_fAngles.z = fRadian3;
	m_pTransformCom->RotationThree(vAxis, fRadian, vAxis2, fRadian2, vAxis3, fRadian3);
}

_float3 CAnimMesh::Get_Pos()
{
	_float3 Pos;
	XMStoreFloat3(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	return Pos;
}

void CAnimMesh::Set_Pos(_float4 Pos)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&Pos));
}

int CAnimMesh::Get_NumAnimations()
{
	return m_pModelCom->Get_NumAnimations(); 
}

const char * CAnimMesh::Get_AnimName(int AniIndex)
{
	return m_pModelCom->Get_Name(AniIndex); 
}

void CAnimMesh::Set_AnimName(const char * Name, int AniIndex)
{
	m_pModelCom->Set_Name(Name, AniIndex); 
}


void CAnimMesh::ChangeAni(int iAniIndex)
{
	m_pModelCom->SetNextIndex(iAniIndex);
	m_pModelCom->SetChangeBool(true);
}

_float CAnimMesh::Get_Duration()
{
	return m_pModelCom->GetDuration();
}

void CAnimMesh::Set_Duration(_float fDuration)
{
	m_pModelCom->SetDuration(fDuration);
}

_float CAnimMesh::Get_TickPerSecond()
{
	return m_pModelCom->GetTickPerSecond();
}

void CAnimMesh::Set_TickPerSecond(_float fTickPerSecond)
{
	m_pModelCom->SetTickPerSecond(fTickPerSecond);
}

_float CAnimMesh::Get_PlayTime()
{
	return m_pModelCom->GetPlayTime();
}

void CAnimMesh::Set_PlayTime(_float fPlayTime)
{
	m_pModelCom->SetPlayTime(fPlayTime);
}

_float CAnimMesh::Get_TimeLimit()
{
	return m_pModelCom->GetTimeLimit();
}

void CAnimMesh::Set_TimeLimit(_float fTimeLimit)
{
	m_pModelCom->SetTimeLimit(fTimeLimit);
}

void CAnimMesh::Change_AniIndex(int Index1, int Index2)
{
	m_pModelCom->ChangeAnimIndex(Index1, Index2);
}

void CAnimMesh::Reset_KeyFrame()
{
	m_pModelCom->ResetKeyFrame();
	m_pModelCom->Play_Animation(0.00001f, m_pModelCom);
}

void CAnimMesh::DeleteAnimation(int Index)
{
	m_pModelCom->DeleteAnimation(Index);
}

HRESULT CAnimMesh::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC Transform;
	Transform.fRotationPerSec = 90.f;
	Transform.fSpeedPerSec = 5.f;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &Transform)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

CAnimMesh * CAnimMesh::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CAnimMesh*		pInstance = new CAnimMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimMesh"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CAnimMesh::Clone(void * pArg)
{
	CAnimMesh*		pInstance = new CAnimMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CAnimMesh"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimMesh::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
}
