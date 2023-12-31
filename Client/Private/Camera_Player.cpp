#include "stdafx.h"
#include "..\Public\Camera_Player.h"
#include "GameInstance.h"
#include "Player.h"
#include "Pointer_Manager.h"
#include "Camera_Manager.h"



CCamera_Player::CCamera_Player(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera(pDevice, pContext)
{
}

CCamera_Player::CCamera_Player(const CCamera_Player & rhs, CTransform::TRANSFORMDESC * pArg)
	: CCamera(rhs, pArg)
{

}

HRESULT CCamera_Player::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCamera_Player::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pLookTransform)))
		return E_FAIL;

	m_pPlayer = PM->Get_PlayerPointer();
	Safe_AddRef(m_pPlayer);

	PM->Add_CameraPlayer(this);
	CRM->Add_Camera_Player(this);
	
	m_vDistance = { 0.f,1.f,-7.f };
	m_pTransformCom->LookAt(m_pPlayer->Get_PlayerPos() + _vector{ 0.f,4.f,0.f,0.f });

	return S_OK;
}

void CCamera_Player::Tick(_float fTimeDelta)
{
	if (!CRM->Get_bScene())
	{

		_long	MouseMove = 0;

		if (MouseMove = GI->Get_DIMMoveState(DIMM_X))
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * fTimeDelta * 0.05f);
		}

		if (MouseMove = GI->Get_DIMMoveState(DIMM_Y))
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), MouseMove * fTimeDelta * 0.05f);
		}
		if (m_bShake)
			Shake(fTimeDelta);
		else
		{

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pPlayer->Get_PlayerPos());

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVector3TransformCoord(XMLoadFloat3(&m_vDistance), m_pTransformCom->Get_WorldMatrix()));
		}
		_float4 _vCurPos;
		XMStoreFloat4(&_vCurPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

		if (m_pTransformCom->Get_State(CTransform::STATE_POSITION).m128_f32[1] < m_pPlayer->Get_NaviPosY() + 0.1f)
		{
			_float4 Pos;
			XMStoreFloat4(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ Pos.x,m_pPlayer->Get_NaviPosY() + 0.1f,Pos.z,1.f });
			if (m_vDistance.z < -2.f)
			{
				m_vDistance.z += 0.1f;
			}
		}
		else if (m_vDistance.z > -6.f && (m_pTransformCom->Get_State(CTransform::STATE_POSITION).m128_f32[1] > m_pPlayer->Get_NaviPosY() + 0.2f))
		{
			m_vDistance.z -= 0.1f;
		}	
	}
	else
	{
		if (!m_bStopScene)
			PlayScene(fTimeDelta);
	}

	if (CRM->Get_bFov())
		Fov(fTimeDelta);

	m_CameraDesc.fFovy = XMConvertToRadians(m_fNowFOV);
	__super::Tick(fTimeDelta);
}

void CCamera_Player::LateTick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

HRESULT CCamera_Player::Render()
{
	return S_OK;
}

_float3 CCamera_Player::Get_Pos()
{
	_float3 vTempPos;
	XMStoreFloat3(&vTempPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	return vTempPos;
}
void CCamera_Player::Set_Pos(_float3 vPos)
{
	_vector temp = XMLoadFloat4(&_float4{ vPos.x,vPos.y,vPos.z,1.f });
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, temp);
}


void CCamera_Player::LookAt(_float3 TargetPos)
{
	_vector temp = XMLoadFloat4(&_float4{ TargetPos.x,TargetPos.y,TargetPos.z,1.f });
	m_pTransformCom->LookAt(temp);
}

void CCamera_Player::PlayScene(_float fTimeDelta)
{
	if (m_bPosPlay)
	{		
		m_pTransformCom->LookAt(m_pLookTransform->Get_State(CTransform::STATE_POSITION));
		if (m_PosInfo[m_iPosInfoIndex].fStopLimit > 0 && !m_bPosStop)
		{
			m_fPosStopLimit += 1.f * fTimeDelta;
			if (m_fPosStopLimit >= m_PosInfo[m_iPosInfoIndex].fStopLimit)
			{
				m_bPosStop = true;
				m_fPosStopLimit = 0.f;
			}			
		}
		else
		{
			_vector vTempPos = XMLoadFloat3(&m_PosInfo[m_iPosInfoIndex].vPos);
			vTempPos = XMVectorSetW(vTempPos, 1.f);
			if (m_pTransformCom->CameraMove(vTempPos, m_PosInfo[m_iPosInfoIndex].fCamSpeed, fTimeDelta, m_PosInfo[m_iPosInfoIndex].fCamSpeed * 0.02f))
			{
				++m_iPosInfoIndex;
				m_bPosStop = false;
				if (m_iPosInfoIndex == m_PosInfo.size())
				{
					m_bPosPlay = false;
					m_bPosStop = false;
					m_iPosInfoIndex = 0;
				}
			}
		}
	}

	if (m_bLookPlay)
	{
		if (m_LookInfo[m_iLookInfoIndex].fStopLimit > 0 && !m_bLookStop)
		{		
			m_fLookStopLimit += 1.f * fTimeDelta;
			if (m_fLookStopLimit >= m_LookInfo[m_iLookInfoIndex].fStopLimit)
			{
				m_bLookStop = true;
				m_fLookStopLimit = 0.f;
			}
		}
		else
		{
			_vector vTempPos = XMLoadFloat3(&m_LookInfo[m_iLookInfoIndex].vPos);
			vTempPos = XMVectorSetW(vTempPos, 1.f);
			if (m_pLookTransform->CameraMove(vTempPos, m_LookInfo[m_iLookInfoIndex].fCamSpeed, fTimeDelta, m_LookInfo[m_iLookInfoIndex].fCamSpeed * 0.02f))
			{
				++m_iLookInfoIndex;
				m_bLookStop = false;
				if (m_iLookInfoIndex == m_LookInfo.size())
				{
					m_bLookPlay = false;
					m_bLookStop = false;
					m_iLookInfoIndex = 0;
				}
			}
		}
	}

	if (!m_bPosPlay && !m_bLookPlay)
	{
		if (CRM->Get_bScene())
		{
			_vector test = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		
			if(fabs(XMVectorGetX(m_vStartLook - test)) < 0.05f)
			{
				CRM->End_Scene();
				CRM->Set_PlayerScene(false);
				m_pTransformCom->LookAt(m_pPlayer->Get_PlayerPos() + _vector{ 0.f,4.f,0.f,0.f });
			}
			else
			{
				m_pTransformCom->TurnY(m_vStartLook, test, 0.9f);				
			}
		}	
	}
	if (m_bShake)
	{
		m_fShakeTimeAcc += 1.f * fTimeDelta;
		if (m_fShakeTimeAcc >= m_fShakeTime)
		{
			End_Shake();
		}

		m_fShakeSpeedAcc += 1.f * fTimeDelta;
		if (m_fShakeSpeedAcc >= m_fShakeSpeed)
		{
			m_bDir = !m_bDir;
			m_fShakeSpeedAcc = 0.f;
			m_fShakePowerAcc = 0.f;
		}

		m_fShakePowerAcc += m_fShakePower * fTimeDelta;
		if (m_bDir)
		{
			_float3 Pos;
			XMStoreFloat3(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			Pos.x += m_fShakePowerAcc;
			Pos.y += m_fShakePowerAcc;
			Pos.z += m_fShakePowerAcc;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ Pos.x, Pos.y, Pos.z, 1.f });
			
		}
		else
		{
			_float3 Pos;
			XMStoreFloat3(&Pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			Pos.x -= m_fShakePowerAcc;
			Pos.y -= m_fShakePowerAcc;
			Pos.z -= m_fShakePowerAcc;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ Pos.x, Pos.y, Pos.z, 1.f });
		}
	}

}

void CCamera_Player::Set_ScenePosInfo(vector<POSINFO> PosInfos)
{
	m_PosInfo.clear();

	m_PosInfo = PosInfos;
	
	if (CRM->Get_bPlayerScene())
	{
		_vector PlayerPos = XMLoadFloat3(&PM->Get_PlayerPointer()->Get_Pos());
		for (auto& iter : m_PosInfo)
			XMStoreFloat3(&iter.vPos, XMLoadFloat3(&iter.vPos) + PlayerPos);
		_vector FixPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		POSINFO EndInfo;
		XMStoreFloat3(&EndInfo.vPos, _vector{ FixPos.m128_f32[0], FixPos.m128_f32[1] , FixPos.m128_f32[2] });
		EndInfo.fCamSpeed = 60.f;
		EndInfo.fStopLimit = 0.f;
		m_PosInfo.push_back(EndInfo);
		m_bPosPlay = true;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vector{ m_PosInfo.front().vPos.x, m_PosInfo.front().vPos.y, m_PosInfo.front().vPos.z, 1.f });
		return;
	}
	_vector FixPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		POSINFO StartInfo;
		XMStoreFloat3(&StartInfo.vPos, _vector{ FixPos.m128_f32[0], FixPos.m128_f32[1] , FixPos.m128_f32[2] });
		StartInfo.fCamSpeed = 60.f;
		StartInfo.fStopLimit = 0.f;
		m_PosInfo.insert(m_PosInfo.begin(), StartInfo);
	
	
		POSINFO EndInfo;
		XMStoreFloat3(&EndInfo.vPos, _vector{ FixPos.m128_f32[0], FixPos.m128_f32[1] , FixPos.m128_f32[2] });
		EndInfo.fCamSpeed = 100.f;
		EndInfo.fStopLimit = 0.f;
		m_PosInfo.push_back(EndInfo);
	

	m_bPosPlay = true;

	
}

void CCamera_Player::Set_SceneLookInfo(vector<LOOKINFO> LookInfos)
{
	m_LookInfo.clear();
	
	m_LookInfo = LookInfos;

	if (CRM->Get_bPlayerScene())
	{
		_vector PlayerPos = XMLoadFloat3(&PM->Get_PlayerPointer()->Get_Pos());
		for (auto& iter : m_LookInfo)
			XMStoreFloat3(&iter.vPos, XMLoadFloat3(&iter.vPos) + PlayerPos);
	}

	m_bLookPlay = true;

	m_vStartLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);

	_vector LookPos = XMLoadFloat3(&m_LookInfo[m_iLookInfoIndex].vPos);
	LookPos = XMVectorSetW(LookPos, 1.f);
	m_pLookTransform->Set_State(CTransform::STATE_POSITION, LookPos);
}

void CCamera_Player::Start_Shake(_float fShakeTime, _float fShakePower, _float fShakeSpeed)
{

	if (m_bShake && m_fShakePower > fShakePower)
		return;

	m_bShake = true;
	m_fShakeTime = fShakeTime;
	m_fShakePower = fShakePower;
	m_fShakeSpeed = fShakeSpeed;

	XMStoreFloat3(&m_vOriginPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	m_vRightPos = { m_vOriginPos.x + m_fShakeSpeed, m_vOriginPos.y + m_fShakeSpeed, m_vOriginPos.z + m_fShakeSpeed, 1.f };
	m_vLeftPos = { m_vOriginPos.x - m_fShakeSpeed, m_vOriginPos.y - m_fShakeSpeed, m_vOriginPos.z - m_fShakeSpeed, 1.f };
	
}

void CCamera_Player::Shake(_float fTimeDelta)
{
	m_fShakeTimeAcc += 1.f * fTimeDelta;
	if (m_fShakeTimeAcc >= m_fShakeTime)
	{
		End_Shake();
	}

	m_fShakeSpeedAcc += 1.f * fTimeDelta;
	if (m_fShakeSpeedAcc >= m_fShakeSpeed)
	{
		m_bDir = !m_bDir;
		m_fShakeSpeedAcc = 0.f;
		m_fShakePowerAcc = 0.f;
	}
	
	m_fShakePowerAcc += m_fShakePower * fTimeDelta;
	if (m_bDir)
	{		
	         
		m_vDistance.x += m_fShakePowerAcc;
		m_vDistance.y += m_fShakePowerAcc;
		m_vDistance.z += m_fShakePowerAcc;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pPlayer->Get_PlayerPos());

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVector3TransformCoord(XMLoadFloat3(&m_vDistance), m_pTransformCom->Get_WorldMatrix()));
	}
	else
	{		
		m_vDistance.x -= m_fShakePowerAcc;
		m_vDistance.y -= m_fShakePowerAcc;
		m_vDistance.z -= m_fShakePowerAcc;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pPlayer->Get_PlayerPos());

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVector3TransformCoord(XMLoadFloat3(&m_vDistance), m_pTransformCom->Get_WorldMatrix()));
	}


}

void CCamera_Player::End_Shake()
{
	m_bShake = false;
	m_fShakeSpeedAcc = 0.f;
	m_fShakeTimeAcc = 0.f;
	m_fShakePowerAcc = 0.f;
	m_vDistance = { 0.f,1.f,-7.f };
}

void CCamera_Player::Start_Fov(_float fFov, _float fFovSpeed)
{
	if (m_fTargetFOV == m_fOriginFOV)
		return;

	m_fTargetFOV = fFov;
	m_fFovSpeed = fFovSpeed;
	

	if (m_fTargetFOV > m_fOriginFOV)
		m_eFovDir = FOVUP;
	else
		m_eFovDir = FOVDOWN;
}

void CCamera_Player::Fov(_float fTimeDelta)
{
	if (m_iFovCount >= 2)
		End_Fov();

	if (m_eFovDir == FOVUP)
	{
		if (m_iFovCount == 0)
		{
			m_fNowFOV += m_fFovSpeed * fTimeDelta;
			if (m_fNowFOV >= m_fTargetFOV)
			{
				m_iFovCount += 1;
				m_fNowFOV = m_fTargetFOV;
			}
		}
		else if(CRM->Get_bFovDir())
		{
			m_fNowFOV -= m_fFovSpeed * fTimeDelta;
			if (m_fNowFOV <= m_fOriginFOV)
			{
				m_iFovCount += 1;
				m_fNowFOV = m_fOriginFOV;
			}
		}
	}

	if (m_eFovDir == FOVDOWN)
	{
		if (m_iFovCount == 0)
		{
			m_fNowFOV -= m_fFovSpeed * fTimeDelta;
			if (m_fNowFOV <= m_fTargetFOV)
			{
				m_iFovCount += 1;
				m_fNowFOV = m_fTargetFOV;
			}
		}
		else if (CRM->Get_bFovDir())
		{
			m_fNowFOV += m_fFovSpeed * fTimeDelta;
			if (m_fNowFOV >= m_fOriginFOV)
			{
				m_iFovCount += 1;
				m_fNowFOV = m_fOriginFOV;
			}
		}
	}
}

void CCamera_Player::FixFov(_float fFov, _float fFovSpeed)
{
	if (fFov == m_fTargetFOV)
		return;

	CRM->Set_Fov(true);

	if (fFov > m_fTargetFOV)
	{
		m_eFovDir = FOVUP;
		m_iFovCount = 0;
		m_fTargetFOV = fFov;
		m_fFovSpeed = fFovSpeed;
	}
	else
	{
		m_eFovDir = FOVDOWN;
		m_iFovCount = 0;
		m_fTargetFOV = fFov;
		m_fFovSpeed = fFovSpeed;
	}
}

void CCamera_Player::End_Fov()
{
	m_eFovDir = FOVEND;
	m_iFovCount = 0;
	CRM->EndFov();
}


void CCamera_Player::Stop_Scene()
{
	if (CRM->Get_bScene())
	{
		_vector test = m_pTransformCom->Get_State(CTransform::STATE_LOOK);

		if (fabs(XMVectorGetX(m_vStartLook - test)) < 0.05f)
		{
			CRM->End_Scene();
			CRM->Set_PlayerScene(false);
			m_bStopScene = false;
			m_pTransformCom->LookAt(m_pPlayer->Get_PlayerPos() + _vector{ 0.f,4.f,0.f,0.f });
		}
		else
		{
			m_pTransformCom->TurnY(m_vStartLook, test, 0.9f);
		}
	}
}

CCamera_Player * CCamera_Player::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Player*		pInstance = new CCamera_Player(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Player"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_Player::Clone(void* pArg)
{
	CCamera::CAMERADESC*		pCameraDesc = (CCamera::CAMERADESC*)pArg;

	CCamera_Player*		pInstance = new CCamera_Player(*this, &pCameraDesc->TransformDesc);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Player::Free()
{
	__super::Free();
	Safe_Release(m_pPlayer);
	Safe_Release(m_pLookTransform);

}
