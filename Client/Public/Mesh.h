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
		_float3 fScale;
		_float4 fPos;
		_float3 fAngle;

	}MESHINFO;

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


public:
	void Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3);
	_float3 Get_Pos();
	void Set_Pos(_float4 Pos);

protected:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CModel*					m_pModelCom = nullptr;
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