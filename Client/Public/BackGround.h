#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CRenderer;
class CTransform;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CBackGround final : public CGameObject
{
private:
	CBackGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBackGround(const CBackGround& rhs);
	virtual ~CBackGround() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

private:
	CShader*				m_pShaderCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CVIBuffer_Rect*			m_pVIBufferCom = nullptr;

private:
	_float					m_fX, m_fY, m_fSizeX, m_fSizeY;	
	_float4x4				m_ViewMatrix, m_ProjMatrix;

private:
	HRESULT Ready_Components();

public:
	static CBackGround* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END


/*
Player
-------------------------------
public:
enum STATE {STATE_IDLE, STATE_DASH, STATE_RUN, STATE_RUNEND, STATE_AIRCOMBO1, STATE_AIRCOMBO2, STATE_AIRCOMBO3, STATE_AIRCOMBO4, STATE_AIRCOMBOLAND, STATE_END};


private:
void SetState(STATE eState);
void SetTickPerSecond();
void ChangeAni();
void GetKeyInput(_float fTimeDelta);
void Update(_float fTimeDelta);
private:
int m_iAniIndex = 0;
_vector m_vTargetLook = { 0.f,0.f,1.f,0.f };
bool m_bMotion = false;
_float m_fDashSpeed = 20.f;
STATE m_eCurState;


void CAnimMesh::GetKeyInput(_float fTimeDelta)
{
if (GI->Key_Pressing(DIK_UP))
{
SetState(STATE_RUN);
m_vTargetLook = { 0.f,0.f,1.f,0.f };
if (GI->Key_Pressing(DIK_RIGHT))
{
m_vTargetLook = { 1.f,0.f,1.f,0.f };
}
if (GI->Key_Pressing(DIK_LEFT))
{
m_vTargetLook = { -1.f,0.f,1.f,0.f };
}
}

else if (GI->Key_Pressing(DIK_DOWN))
{
SetState(STATE_RUN);
m_vTargetLook = { 0.f,0.f,-1.f,0.f };
if (GI->Key_Pressing(DIK_RIGHT))
{
m_vTargetLook = { 1.f,0.f,-1.f,0.f };
}
if (GI->Key_Pressing(DIK_LEFT))
{
m_vTargetLook = { -1.f,0.f,-1.f,0.f };
}

}

else if (GI->Key_Pressing(DIK_RIGHT))
{
SetState(STATE_RUN);
m_vTargetLook = { 1.f,0.f,0.f,0.f };

}

else if (GI->Key_Pressing(DIK_LEFT))
{
SetState(STATE_RUN);
m_vTargetLook = { -1.f,0.f,0.f,0.f };

}
else if (m_eCurState == STATE_RUN)
{
SetState(STATE_RUNEND);
}


if (GI->Key_Down(DIK_A))
{
SetState(STATE_AIRCOMBO1);
}

if (GI->Key_Down(DIK_S))
{
SetState(STATE_AIRCOMBO2);
}

if (GI->Key_Down(DIK_D))
{
SetState(STATE_AIRCOMBO3);
}

if (GI->Key_Down(DIK_F))
{
SetState(STATE_AIRCOMBO4);
}

if (GI->Key_Down(DIK_G))
{
SetState(STATE_AIRCOMBOLAND);
}

if (GI->Key_Down(DIK_SPACE))
{
SetState(STATE_DASH);
}

}

void CAnimMesh::Update(_float fTimeDelta)
{
SetTickPerSecond();

switch (m_eCurState)
{
case Client::CAnimMesh::STATE_IDLE:
break;
case Client::CAnimMesh::STATE_DASH:
if (m_fDashSpeed > 1.f)
m_fDashSpeed -= 1.f;
m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fDashSpeed , fTimeDelta);
break;
case Client::CAnimMesh::STATE_RUN:
m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_vTargetLook, 0.6f);
m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 8.f, fTimeDelta);
break;
case Client::CAnimMesh::STATE_RUNEND:

if (m_pModelCom->GetPlayTime() < m_pModelCom->GetDuration() - 13.f)
m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 5.f, fTimeDelta);
break;
case Client::CAnimMesh::STATE_AIRCOMBO1:
break;
case Client::CAnimMesh::STATE_AIRCOMBO2:
break;
case Client::CAnimMesh::STATE_AIRCOMBO3:
break;
case Client::CAnimMesh::STATE_AIRCOMBO4:
break;
case Client::CAnimMesh::STATE_AIRCOMBOLAND:
break;
case Client::CAnimMesh::STATE_END:
break;
default:
break;
}
}


void CAnimMesh::SetState(STATE eState)
{
if (m_eCurState == eState)
return;

m_eCurState = eState;

switch (m_eCurState)
{
case Client::CAnimMesh::STATE_IDLE:
m_iAniIndex = 0;
m_bMotion = false;
break;
case Client::CAnimMesh::STATE_DASH:
m_iAniIndex = 1;
m_bMotion = true;
m_fDashSpeed = 20.f;

break;
case Client::CAnimMesh::STATE_RUN:
m_iAniIndex = 2;

break;
case Client::CAnimMesh::STATE_RUNEND:
m_iAniIndex = 3;

m_bMotion = true;
break;
case Client::CAnimMesh::STATE_AIRCOMBO1:
m_iAniIndex = 4;
break;
case Client::CAnimMesh::STATE_AIRCOMBO2:
m_iAniIndex = 5;
break;
case Client::CAnimMesh::STATE_AIRCOMBO3:
m_iAniIndex = 6;
break;
case Client::CAnimMesh::STATE_AIRCOMBO4:
m_iAniIndex = 7;
break;
case Client::CAnimMesh::STATE_AIRCOMBOLAND:
m_iAniIndex = 8;
break;
case Client::CAnimMesh::STATE_END:
break;
default:
break;
}

m_pModelCom->SetNextIndex(m_iAniIndex);
m_pModelCom->SetChangeBool(true);


}

void CAnimMesh::SetTickPerSecond()
{
switch (m_eCurState)
{
case Client::CAnimMesh::STATE_IDLE:
break;
case Client::CAnimMesh::STATE_DASH:
m_pModelCom->SetTickPerSecond(100.f);
break;
case Client::CAnimMesh::STATE_RUN:
m_pModelCom->SetTickPerSecond(30.f);
break;
case Client::CAnimMesh::STATE_RUNEND:
m_pModelCom->SetTickPerSecond(70.f);
break;
case Client::CAnimMesh::STATE_AIRCOMBO1:
break;
case Client::CAnimMesh::STATE_AIRCOMBO2:
break;
case Client::CAnimMesh::STATE_AIRCOMBO3:
break;
case Client::CAnimMesh::STATE_AIRCOMBO4:
break;
case Client::CAnimMesh::STATE_AIRCOMBOLAND:
break;
case Client::CAnimMesh::STATE_END:
break;
default:
break;
}
}

void CAnimMesh::ChangeAni()
{
switch (m_eCurState)
{
case Client::CAnimMesh::STATE_IDLE:
break;
case Client::CAnimMesh::STATE_DASH:
SetState(STATE_IDLE);
break;
case Client::CAnimMesh::STATE_RUN:
break;
case Client::CAnimMesh::STATE_RUNEND:
SetState(STATE_IDLE);
break;
case Client::CAnimMesh::STATE_AIRCOMBO1:
SetState(STATE_AIRCOMBO2);
break;
case Client::CAnimMesh::STATE_AIRCOMBO2:
SetState(STATE_AIRCOMBO3);
break;
case Client::CAnimMesh::STATE_AIRCOMBO3:
SetState(STATE_AIRCOMBO4);
break;
case Client::CAnimMesh::STATE_AIRCOMBO4:
SetState(STATE_AIRCOMBOLAND);
break;
case Client::CAnimMesh::STATE_AIRCOMBOLAND:
SetState(STATE_IDLE);
break;
case Client::CAnimMesh::STATE_END:
break;
default:
break;
}
}

*/