#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
BEGIN(Engine)
class CShader;
class CRenderer;
class CTransform;
class CModelsInstance;
END

BEGIN(Client)

class CMeshInstance : public CGameObject
{
public:
	typedef struct tagMeshInfo
	{
		_tchar* sTag;
		
	}MESHINFO;

protected:
	CMeshInstance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshInstance(const CMeshInstance& rhs);
	virtual ~CMeshInstance() = default;

public:
	virtual HRESULT Initialize_Prototype()override;
	virtual HRESULT Initialize(void* pArg)override;
	virtual void Tick(_float fTimeDelta)override;
	virtual void LateTick(_float fTimeDelta)override;
	virtual void Collision(CGameObject* pOther, string sTag) {};


protected:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	MESHINFO*				m_MeshInfo;
	CTransform*				m_pParentTransformCom = nullptr;


protected:
	HRESULT Ready_Components();

public:
	static CMeshInstance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END