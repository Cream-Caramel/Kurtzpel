#include "..\Public\Cell.h"
#include "VIBuffer_Cell.h"
#include "Shader.h"
#include "PipeLine.h"
#include "Sphere.h"

CCell::CCell(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}



HRESULT CCell::Initialize(int iCellIndex, const _float3 * pPoints, const _float3 * pNormals, int * pNeighborIndex)
{
	m_iIndex = iCellIndex;
	memcpy(m_vPoints, pPoints, sizeof(_float3) * POINT_END);
	memcpy(m_vNormal, pNormals, sizeof(_float3) * LINE_END);
	memcpy(m_iNeighborIndex, pNeighborIndex, sizeof(int) * LINE_END);

#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_vPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXCOL_DECLARATION::Elements, VTXCOL_DECLARATION::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
#endif
	return S_OK;
}


_bool CCell::isIn(_fvector vPosition, _int * pNeighborIndex, _vector* vNormal)
{
	_uint _iOut = 0;
	for (_uint i = 0; i < LINE_END; ++i)
	{
		_vector      vDir = XMVectorSetW(vPosition - XMLoadFloat3(&m_vPoints[i]), 0.f);
		vDir = XMVector3Normalize(vDir);
		_vector _vTemp = XMVector3Dot(vDir, XMLoadFloat3(&m_vNormal[i]));
		_float  _fTemp = XMVectorGetX(_vTemp);

		if (0.f <= _fTemp)
		{
			if (XMVectorGetX(XMVector3Dot(vDir, XMLoadFloat3(&m_vNormal[i]))) != 0)
			{
				if (vNormal != nullptr)
					*vNormal = XMLoadFloat3(&m_vNormal[i]) * (-1.f);
				if (_iOut == 0)
					*pNeighborIndex = m_iNeighborIndex[i];
			}

			++_iOut;
		}
	}

	if (_iOut > 0)
	{
		if (_iOut > 1/* && *pNeighborIndex == -1*/)
		{
			*pNeighborIndex = -1;
			if (vNormal != nullptr)
				*vNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		}
		return false;
	}
	return true;
}

#ifdef _DEBUG
HRESULT CCell::Render_Cell(_float fHeight, _float4 vColor)
{
	CPipeLine*			pPipeLine = GET_INSTANCE(CPipeLine);

	_float4x4			WorldMatrix;
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

	//WorldMatrix.m[1][3] = fHeight;

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &pPipeLine->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &pPipeLine->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShader->Set_RawValue("g_vColor", &vColor, sizeof(_float4))))
			return E_FAIL;

	RELEASE_INSTANCE(CPipeLine);

	m_pShader->Begin(0);

	m_pVIBuffer->Render();

	return S_OK;
}
#endif // _DEBUG



CCell * CCell::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, int iCellIndex, const _float3 * pPoints, const _float3 * pNormals, int * pNeighborIndex)
{
	CCell*			pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iCellIndex, pPoints, pNormals, pNeighborIndex)))
	{
		MSG_BOX(TEXT("Failed To Created : CCell"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCell::Free()
{

#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
#endif // _DEBUG

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);


}
