#include "stdafx.h"
#include "..\Public\AlphaParticle.h"
#include "GameInstance.h"
#include "Pointer_Manager.h"
#include "Player.h"

CAlphaParticle::CAlphaParticle(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CAlphaParticle::CAlphaParticle(const CAlphaParticle & rhs)
	: CGameObject(rhs)
{
}

HRESULT CAlphaParticle::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CAlphaParticle::Initialize(void * pArg)
{

	memcpy(&m_ParticleInfo, pArg, sizeof(PARTICLEINFO));
	
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

	SetDirPoint(WorldPos);

	if (FAILED(Ready_Components()))
		return E_FAIL;
	
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&WorldPos));
	m_pTransformCom->Set_Scale(_vector{ m_ParticleInfo.vSize.x, m_ParticleInfo.vSize.y });
	return S_OK;
}

void CAlphaParticle::Tick(_float fTimeDelta)
{
	m_fLifeTimeAcc += 1.f * fTimeDelta;
	if (m_fLifeTimeAcc >= m_ParticleInfo.fLifeTime)
		Set_Dead();
	m_pTransformCom->Go_Dir(XMLoadFloat3(&m_ParticleInfo.vDirection), m_ParticleInfo.fSpeed, fTimeDelta);

	if (m_ParticleInfo.fGravity > 0.f)
	{
		m_fGravityAcc += m_ParticleInfo.fGravity * fTimeDelta;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION) - _vector{ 0.f,m_fGravityAcc, 0.f,0.f });
	}
	if (m_ParticleInfo.TexNum > 1)
	{
		m_fFrameAcc += 1.f * fTimeDelta;
		if (m_fFrameAcc >= m_ParticleInfo.fFrameSpeed)
		{
			m_fFrameAcc = 0.f;
			m_iNowFrame += 1;
			if (m_iNowFrame > m_ParticleInfo.TexNum)
			{
				if (m_ParticleInfo.bLoof)
					Set_Dead();
				else
					m_iNowFrame = 0;
			}
		}
	}
}

void CAlphaParticle::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	if (m_ParticleInfo.eDirPoint != DIR_END)	
		CheckDead();
		
	

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

}

HRESULT CAlphaParticle::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ViewMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &GI->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4));

	m_pShaderCom->Set_RawValue("g_vCamPosition", &GI->Get_CamPosition(), sizeof(_float4));

	if (m_pTextureCom != nullptr)
	{
		if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", m_iNowFrame)))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;
	

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CAlphaParticle::SetDirPoint(_float4 vWorldPos)
{
	
	switch (m_ParticleInfo.eDirPoint)
	{
	case DIR_PLAYER:
	{
		_float3 DirPos = PM->Get_PlayerPointer()->Get_Pos();
		DirPos.y += 0.5f;
		_vector PlayerDir = XMLoadFloat3(&DirPos) - _vector{ vWorldPos.x,vWorldPos.y,vWorldPos.z };
		XMStoreFloat3(&m_ParticleInfo.vDirection, PlayerDir);
	}
		break;
	case DIR_END:
		if (m_ParticleInfo.vDirection.x == 0 && m_ParticleInfo.vDirection.y == 0 && m_ParticleInfo.vDirection.z == 0)
			XMStoreFloat3(&m_ParticleInfo.vDirection, _vector{ m_ParticleInfo.vPosition.x, m_ParticleInfo.vPosition.y , m_ParticleInfo.vPosition.z } -_vector{ 0.f,0.f,0.f });
		break;
	case DIR_GOLEM:
	{
		_float3 DirPos = PM->Get_BossPos();
		DirPos.y += 0.5f;
		_vector BossDir = XMLoadFloat3(&DirPos) - _vector{ vWorldPos.x,vWorldPos.y,vWorldPos.z };
		XMStoreFloat3(&m_ParticleInfo.vDirection, BossDir);
	}
	}
	
}

void CAlphaParticle::CheckDead()
{
	switch (m_ParticleInfo.eDirPoint)
	{
	case DIR_PLAYER:
	{
		_float3 DirPos = PM->Get_PlayerPointer()->Get_Pos();
		DirPos.y += 0.5f;
		if (0.1f >= XMVector3Length(XMLoadFloat3(&DirPos) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)).m128_f32[0])
			Set_Dead();
	}
		break;

	case DIR_GOLEM:
	{
		_float3 DirPos = PM->Get_BossPos();
		DirPos.y += 0.5f;
		if (0.1f >= XMVector3Length(XMLoadFloat3(&DirPos) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)).m128_f32[0])
			Set_Dead();
	}
	break;

	}

	
}


HRESULT CAlphaParticle::Ready_Components()
{
	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPoint"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Particle"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

CAlphaParticle * CAlphaParticle::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CAlphaParticle*		pInstance = new CAlphaParticle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CAlphaParticle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CAlphaParticle::Clone(void * pArg)
{
	CAlphaParticle*		pInstance = new CAlphaParticle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CAlphaParticle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAlphaParticle::Free()
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
