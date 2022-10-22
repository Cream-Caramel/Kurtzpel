#include "..\Public\ModelInstanceContainer.h"


CModelInstanceContainer::CModelInstanceContainer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CModelInstanceContainer::CModelInstanceContainer(const CModelInstanceContainer & rhs)
	: CVIBuffer(rhs)
	, m_iMaterialIndex(rhs.m_iMaterialIndex)
	, m_iNumInstance(rhs.m_iNumInstance)
	, m_iInstanceStride(rhs.m_iInstanceStride)
	, m_pVBInstance(rhs.m_pVBInstance)
{
}

HRESULT CModelInstanceContainer::Initialize_Prototype(_fmatrix PivotMatrix, CBinary * pBinary, vector<_float3*> MatrixInfo)
{
	m_iMaterialIndex = pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex[pBinary->MaterialIndex++];
	m_iNumInstance = MatrixInfo.size();
#pragma region VERTEXBUFFER
	m_iNumVertexBuffers = 2;
	m_iNumVertices = pBinary->m_BinaryVector->NumVertices.NumVertices[pBinary->NumVerticesIndex++];
	m_iStride = sizeof(VTXMODEL);

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumVertices * m_iStride;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXMODEL*		pVertices = new VTXMODEL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMODEL) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		pVertices[i] = pBinary->m_BinaryVector->NonAniVertexInfo.NonAniVerticesInfo[pBinary->NonVertexInfoIndex++];
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

#pragma endregion

#pragma region INDEXBUFFER
	m_iNumPrimitives = pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives[pBinary->NumPrimitivesIndex++];

	m_iIndexSizeofPrimitive = sizeof(FACEINDICES32);
	m_iNumIndicesofPrimitive = 3;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumPrimitives * m_iIndexSizeofPrimitive;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;


	FACEINDICES32*		pIndices = new FACEINDICES32[m_iNumPrimitives * m_iNumInstance];
	ZeroMemory(pIndices, sizeof(FACEINDICES32) * m_iNumPrimitives * m_iNumInstance);

	for (_uint i = 0; i < m_iNumInstance; ++i)
	{
		pBinary->pIndicesIndex = 0;
		for (_uint j = 0; j < m_iNumPrimitives; ++j)
		{
			pIndices[j + i * m_iNumPrimitives] = pBinary->m_BinaryVector->pIndices.pIndices[pBinary->pIndicesIndex++];
		}
	}


	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pIndices;

	if (FAILED(__super::Create_IndexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

#pragma region INSTANCE
	m_iInstanceStride = sizeof(MODELINSTANCE);

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iInstanceStride;

	MODELINSTANCE*		pInstanceVtx = new MODELINSTANCE[m_iNumInstance];
	ZeroMemory(pInstanceVtx, sizeof(MODELINSTANCE) * m_iNumInstance);

	for (_uint i = 0; i < m_iNumInstance; ++i)
	{
		pInstanceVtx[i].vRight = _float4{ MatrixInfo[i][0].x ,MatrixInfo[i][0].y, MatrixInfo[i][0].z, 0.f };
		pInstanceVtx[i].vUp = _float4{ MatrixInfo[i][1].x ,MatrixInfo[i][1].y, MatrixInfo[i][1].z, 0.f };
		pInstanceVtx[i].vLook = _float4{ MatrixInfo[i][2].x ,MatrixInfo[i][2].y, MatrixInfo[i][2].z, 0.f };
		pInstanceVtx[i].vPosition = _float4{ MatrixInfo[i][3].x ,MatrixInfo[i][3].y, MatrixInfo[i][3].z, 1.f };
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pInstanceVtx;

	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubResourceData, &m_pVBInstance)))
		return E_FAIL;

	Safe_Delete_Array(pInstanceVtx);
#pragma endregion INSTANCE

	return S_OK;
}

HRESULT CModelInstanceContainer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CModelInstanceContainer::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance,
	};

	_uint			iStrides[] = {
		m_iStride,
		m_iInstanceStride
	};

	_uint			iOffsets[] = {
		0,
		0,
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iStrides, iOffsets);

	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	m_pContext->IASetPrimitiveTopology(m_eTopology);

	m_pContext->DrawIndexedInstanced(m_iNumPrimitives * 3, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

CModelInstanceContainer * CModelInstanceContainer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _fmatrix PivotMatrix, CBinary * pBinary, vector<_float3*> MatrixInfo)
{
	CModelInstanceContainer*			pInstance = new CModelInstanceContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(PivotMatrix, pBinary, MatrixInfo)))
	{
		MSG_BOX(TEXT("Failed To Created : CModelInstanceContainer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CModelInstanceContainer::Clone(void * pArg)
{
	CModelInstanceContainer*			pInstance = new CModelInstanceContainer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CModelInstanceContainer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModelInstanceContainer::Free()
{
	__super::Free();
	Safe_Release(m_pVBInstance);

}
