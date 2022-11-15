#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "AlphaParticle.h"
BEGIN(Client)
class CAlphaParticle;

class CParticle_Manager final : public CBase
{
public:
	typedef struct tagParticleCreateInfo
	{
		_float4 vMinPos;
		_float4 vMaxPos;
		_float2 vMinSize;
		_float2 vMaxSize;
		_float fMinSpeed = 0.f;
		_float fMaxSpeed = 0.f;
		_float fMinLifeTime = 0.f;
		_float fMaxLifeTime = 0.f;
		_float fGravity = 0.f;
		_float3 vDir;
		int iParticleNum;
		const _tchar* TexPath;
		const _tchar* TexName;
		_float fFrameSpeed = 0.f;
		int TexNums;
	}PARTICLECREATE;

	DECLARE_SINGLETON(CParticle_Manager)
private:
	CParticle_Manager();
	virtual ~CParticle_Manager() = default;

public:
	void Initalize();
	void LoadParticle();
	void CreateParticle(_tchar* ParticleName, _float4 vPos, _bool bFollow, _bool bLoof, CAlphaParticle::DIRPOINT ePoint);
	PARTICLECREATE SearchParticle(_tchar* ParticleName);
private:
	_float4 m_vPos = { 0.f,0.f,0.f,1.f };
	_float2 m_vSize;
	_float m_fLifeTime;
	_float m_fSpeed;
	map<_tchar*, PARTICLECREATE> m_ParticleCreateInfo;

public:
	virtual void Free() override;
};
END

