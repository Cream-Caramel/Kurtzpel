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

class CMesh final : public CGameObject
{
public:
	typedef struct tagMeshInfo
	{
		_tchar* sTag;
		_float3 fScale;
		_float4 fPos;
		_float3 fAngle;

	}MESHINFO;

private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& rhs);
	virtual ~CMesh() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void Rotation(_float3 vAxis, _float fRadian, _float3 vAxis2, _float fRadian2, _float3 vAxis3, _float fRadian3);
	_float3 Get_Angles() { return m_fAngles; }
	_float3 Get_Pos();
	void Set_Pos(_float4 Pos);
	void Set_Scale(_float3 fScale) { m_pTransformCom->Set_Scale(XMLoadFloat3(&fScale)); }
	_float3 Get_Scale() { return m_pTransformCom->Get_Scale(); }
	const _tchar* Get_Tag() { return sTag.c_str(); }

private:
	_float3 m_fAngles;
	wstring sTag;
private:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CModel*					m_pModelCom = nullptr;
	MESHINFO*				m_MeshInfo;


private:
	HRESULT Ready_Components();

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END