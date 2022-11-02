#include "..\Public\Light_Manager.h"
#include "Light.h"

IMPLEMENT_SINGLETON(CLight_Manager)

CLight_Manager::CLight_Manager()
{
}

const LIGHTDESC * CLight_Manager::Get_LightDesc(_uint iIndex)
{
	auto	iter = m_Lights.begin();

	for (_uint i = 0; i < iIndex; ++i)
		++iter;

	return (*iter)->Get_LightDesc();	
}

const LIGHTDESC * CLight_Manager::Get_StaticLightDesc(_uint iIndex)
{
	auto	iter = m_StaticLights.begin();

	for (_uint i = 0; i < iIndex; ++i)
		++iter;

	return (*iter)->Get_LightDesc();
}

HRESULT CLight_Manager::Set_StaticLight(_float fTimeLimit, _float fRange, _float4 vPos, _uint iIndex)
{
	auto	iter = m_StaticLights.begin();

	for (_uint i = 0; i < iIndex; ++i)
		++iter;

	return (*iter)->Set_StaticLight(fTimeLimit, fRange, vPos, iIndex);
}

HRESULT CLight_Manager::Reset_Lights()
{
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();

	return S_OK;
}

HRESULT CLight_Manager::Add_Light(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const LIGHTDESC & LightDesc)
{
	CLight*		pLight = CLight::Create(pDevice, pContext, LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Add_StaticLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const LIGHTDESC & LightDesc)
{
	CLight*		pLight = CLight::Create(pDevice, pContext, LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_StaticLights.push_back(pLight);
	return S_OK;
}

HRESULT CLight_Manager::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	for (auto& pLight : m_Lights)
		pLight->Render(pShader, pVIBuffer);

	for (auto& pLight : m_StaticLights)
		pLight->Render(pShader, pVIBuffer);

	return S_OK;
}

void CLight_Manager::Tick(_float fTimeDelta)
{
	for (auto& pLight : m_StaticLights)
		pLight->Tick(fTimeDelta);
}

void CLight_Manager::Free()
{
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();

	for (auto& pLight : m_StaticLights)
		Safe_Release(pLight);

	m_StaticLights.clear();
}
