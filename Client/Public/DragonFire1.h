#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CRenderer;
class CTransform;
class CVIBuffer_Particle;
class COBB;
END

BEGIN(Client)

class CDragonFire1 : public CGameObject
{

public:
	typedef struct tagDragonFire1Info
	{
		_float4 vPosition;
		_float3 vDirection;
	}DRAGONFIRE1INFO;
private:
	CDragonFire1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDragonFire1(const CDragonFire1& rhs);
	virtual ~CDragonFire1() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();


private:	
	DRAGONFIRE1INFO m_DragonFire1Info;
	_bool m_bEnd = false;
	_float m_fEndAcc = 0.f;
	_float m_fShaderUVAcc = 0.f;
	_float m_fMaxUVIndexX = 8.f;
	_float m_fMaxUVIndexY = 8.f;
	_float m_fShaderUVIndexX = 0.f;
	_float m_fShaderUVIndexY = 0.f;

private:
	_float3 m_vTargetLook;
	CShader*				m_pShaderCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CVIBuffer_Particle*			m_pVIBufferCom = nullptr;
	COBB* m_pOBB = nullptr;

private:
	HRESULT Ready_Components();

public:
	static CDragonFire1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END