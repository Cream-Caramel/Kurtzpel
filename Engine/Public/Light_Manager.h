#pragma once

#include "Base.h"

BEGIN(Engine)

/* 빛들을 보관한다. */

class CLight_Manager final : public CBase
{
	DECLARE_SINGLETON(CLight_Manager)
public:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHTDESC* Get_LightDesc(_uint iIndex);
	const LIGHTDESC* Get_StaticLightDesc(_uint iIndex);
	HRESULT Set_StaticLight(_float fTimeLimit, _float fRange, _float4 vPos, _uint iIndex);
public:
	HRESULT Reset_Lights();
	HRESULT Add_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const LIGHTDESC& LightDesc);
	HRESULT Add_StaticLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const LIGHTDESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	void Tick(_float fTimeDelta);
	_matrix Get_LightMatrix() { return m_LightMatrix; }
	void Set_LightMatrix(_matrix matrix) { m_LightMatrix = matrix; }
private:
	list<class CLight*>			m_Lights;
	typedef list<class CLight*>	LIGHTS;

	list <class CLight*>	m_StaticLights;
	_matrix m_LightMatrix;
public:
	virtual void Free() override;
};

END