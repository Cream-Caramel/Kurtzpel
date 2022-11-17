#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CRenderer;
class CTransform;
class CVIBuffer_Particle;
END

BEGIN(Client)

class CAlphaParticle : public CGameObject
{
public:
	enum DIRPOINT {DIR_PLAYER, DIR_PLAYERSOWRD, DIR_GOLEM, DIR_END};
public:
	typedef struct tagParticleInfo
	{
		_float4 vPosition;
		_float2 vSize;
		_float fSpeed;
		_float fLifeTime;
		_float3 vDirection;
		_float fFrameSpeed;
		_float fGravity;
		const _tchar* TexPath;
		const _tchar* TexName;
		int TexNum = 0;
		_float4 vWorldPos;
		_bool bLoof;
		DIRPOINT eDirPoint;
		_bool bUV = false;
		int iShaderPass;
		_float fUVSpeed;
		_float fMaxUVIndexX;
		_float fMaxUVIndexY;
		_bool bUVLoof = false;
	}PARTICLEINFO;
private:
	CAlphaParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAlphaParticle(const CAlphaParticle& rhs);
	virtual ~CAlphaParticle() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

private:
	void SetDirPoint(_float4 vWorldPos);
	void CheckDead();

private:
	PARTICLEINFO m_ParticleInfo;
	int m_iShaderPass = 0;
	_float m_fLifeTimeAcc = 0.f;
	_float m_fFrameAcc = 0.f;
	_float m_fGravityAcc = 0.f;
	_float m_fShaderUVAcc = 0.f;
	_float m_fShaderUVIndexX = 0;
	_float m_fShaderUVIndexY = 0;
	int m_iNowFrame = 0;
	const _tchar* TexPath;
	const _tchar* TexName;

private:
	CShader*				m_pShaderCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CVIBuffer_Particle*			m_pVIBufferCom = nullptr;

private:
	HRESULT Ready_Components();

public:
	static CAlphaParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END