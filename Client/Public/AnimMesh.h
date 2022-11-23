#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Camera_Manager.h"

BEGIN(Engine)
class CShader;
class CRenderer;
class CTransform;
class CAnimModel;
END

BEGIN(Client)

class CAnimMesh : public CGameObject
{
public:
	typedef struct tagMeshInfo
	{
		_tchar* sTag;
		_float3 fScale = { 1.f,1.f,1.f };
		_float4 fPos = { 0.f,0.f,0.f,1.f };
		_float3 fAngle = { 0.f,0.f,0.f };

	}MESHINFO;

	typedef struct tagEffectInfo
	{
		_matrix WorldMatrix;
		_float3 vScale;
		_bool bObtion = false;

	}EFFECTINFO;

protected:
	CAnimMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimMesh(const CAnimMesh& rhs);
	virtual ~CAnimMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Collision(CGameObject* pOther, string sTag) {};

public:
	void Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3);
	_vector Get_vPos();
	_float3 Get_Pos();
	_float3 Get_Right();
	_float3 Get_Look();
	void Set_Pos(_float4 Pos);
	const _tchar* Get_Tag() { return sTag.c_str(); }
	int Get_AnimIndex() { return m_iAniIndex; }
	int Get_NumAnimations();
	void ChangeAni(int iAniIndex);
	_bool Get_Die() { return m_bDie; }
	_bool Get_Finish() { return m_bFinish; }
protected:
	wstring sTag;
	MESHINFO*				m_MeshInfo;
	_bool m_bHit = false;
	_float m_fHitAcc = 0.f;
	_bool m_bDie = false;
	_bool m_bFinish = false;
protected:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CAnimModel*				m_pModelCom = nullptr;
	
protected:
	HRESULT Ready_Components();
	int m_iAniIndex = 0;

public:
	static CAnimMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END