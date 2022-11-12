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

class CPlayerGage : public CGameObject
{
public:
	typedef struct tagParticleInfo
	{
		_float4 vPosition;
		_float2 vSize;
		_float fLifeTime;
		const _tchar* TexPath;
		const _tchar* TexName;
		int TexNum = 0;
		_float4 vWorldPos;
	}GAGEINFO;
private:
	CPlayerGage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayerGage(const CPlayerGage& rhs);
	virtual ~CPlayerGage() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();


private:
	GAGEINFO m_ParticleInfo;
	int m_iNowFrame = 0;
	const _tchar* TexPath;
	const _tchar* TexName;

private:
	CShader*				m_pShaderCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	//CVIBuffer_Particle*			m_pVIBufferCom = nullptr;
	CVIBuffer_Rect*			m_pVIBufferCom = nullptr;

private:
	HRESULT Ready_Components();

public:
	static CPlayerGage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END