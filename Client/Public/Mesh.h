#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CRenderer;
class CTransform;
class CModel;
END

BEGIN(Client)

class CMesh : public CGameObject
{
public:
	typedef struct tagMeshInfo
	{
		_tchar* sTag;
		_float3 vScale{ 1.f,1.f, 1.f };
		_float4 vPos;
		_float3 vAngle{ 0.f,0.f,0.f };

	}MESHINFO;
public:
	enum TURNDIR {TURN_FRONT, TURN_BACK, TURN_END};
protected:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& rhs);
	virtual ~CMesh() = default;

public:
	virtual HRESULT Initialize_Prototype()override;
	virtual HRESULT Initialize(void* pArg)override;
	virtual void Tick(_float fTimeDelta)override;
	virtual void LateTick(_float fTimeDelta)override;
	virtual HRESULT Render()override;
	virtual void Collision(CGameObject* pOther, string sTag) {};

public:
	HRESULT SetUp_State(_fmatrix StateMatrix);
	void Set_EffectMatrix(_matrix Matrix);
	void Set_EffectInfo(_float fTurnSpeed, _float fRenderLimit, _float fMoveSpeed, _float fMoveSpeedTempo, _float3 vTargetLook, TURNDIR eTurnDir);

public:
	void Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3);
	_float3 Get_Pos();
	void Set_Pos(_float4 Pos);

protected:
	_float m_fShaderUVAcc = 0.f;
	_float m_fUVSpeed = 0.1f;
	_float m_fShaderUVIndexX = 0;
	_float m_fShaderUVIndexY = 0;
	_float m_fMaxUVIndexX = 0;
	_float m_fMaxUVIndexY = 0;
	_float m_fRenderLimitAcc = 0.f;
	_float m_fRenderLimit;
	_float m_fMoveSpeed;
	_float m_fMoveSpeedTempo;
	_float m_fEndAcc = 0.f;
	_float3 m_vTargetLook = { 0.f,0.f,0.f };	
	TURNDIR m_eTurnDir = TURN_FRONT;

protected:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	MESHINFO*				m_MeshInfo;
	CTransform*				m_pParentTransformCom = nullptr;


protected:
	HRESULT Ready_Components();

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END