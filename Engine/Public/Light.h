#pragma once

#include "Base.h"

BEGIN(Engine)
/* 빛 정보를 보관한다. */

class CLight final : public CBase
{
private:
	CLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLight() = default;

public:
	const LIGHTDESC* Get_LightDesc() const {
		return &m_LightDesc;
	}

	HRESULT Set_StaticLight(_float fTimeLimit, _float fRange, _float4 vPos, _uint iIndex);

public:
	HRESULT Initialize(const LIGHTDESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	void Tick(_float fTimeDelta);
private:
	ID3D11Device*			m_pDevice = nullptr;
	ID3D11DeviceContext*	m_pContext = nullptr;

private:
	LIGHTDESC				m_LightDesc;
	_float					m_fTimeLimitAcc = 0.f;
	_float					m_fTimeLimit = 0.f;

public:
	static CLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const LIGHTDESC& LightDesc);
	virtual void Free() override;
};

END