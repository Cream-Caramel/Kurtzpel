#include "..\Public\VIBuffer_Tile.h"
#include "Picking.h"
#include "Transform.h"

CVIBuffer_Tile::CVIBuffer_Tile(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Tile::CVIBuffer_Tile(const CVIBuffer_Tile & rhs)
	: CVIBuffer(rhs)
{

}

HRESULT CVIBuffer_Tile::Initialize_Prototype()
{
#pragma region VERTEXBUFFER
	m_iNumVertexBuffers = 1;
	m_iStride = sizeof(VTX);

#pragma endregion

#pragma region INDEXBUFFER
	
	m_iIndexSizeofPrimitive = sizeof(FACEINDICES16);
	m_iNumIndicesofPrimitive = 3;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	

#pragma endregion


	return S_OK;
}

HRESULT CVIBuffer_Tile::Initialize(void * pArg)
{
	CreateTile();
	return S_OK;
}

void CVIBuffer_Tile::Render()
{
	if (m_pVB != nullptr)
		__super::Render();

}

HRESULT CVIBuffer_Tile::CreateTile()
{
	if (m_pVB != nullptr)
		Safe_Release(m_pVB);
	if (m_pIB != nullptr)
		Safe_Release(m_pIB);

	m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;;
	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumVertices * m_iStride;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	Safe_Delete_Array(m_pVerticesPos);

	m_pVerticesPos = new _float3[m_iNumVertices];
	VTX*		pVertices = new VTX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTX) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVerticesZ; ++i)
	{
		for (_uint j = 0; j < m_iNumVerticesX; ++j)
		{
			/* 루프가 하나씩 진행될때마다 1씩증가하는 인덱스를 얻어오기위한.  */
			_uint		iIndex = i * m_iNumVerticesX + j;
			pVertices[iIndex].vPosition = m_pVerticesPos[iIndex] = _float3(float(j), 0.0f, float(i));	
		}
	}



	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	m_iNumPrimitives = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2;;
	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumPrimitives * m_iIndexSizeofPrimitive;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;


	FACEINDICES16*		pIndices = new FACEINDICES16[m_iNumPrimitives];
	ZeroMemory(pIndices, sizeof(FACEINDICES16) * m_iNumPrimitives);

	_uint			iNumFaces = 0;

	for (_uint i = 0; i < m_iNumVerticesZ - 1; ++i)
	{
		for (_uint j = 0; j < m_iNumVerticesX - 1; ++j)
		{
			/* 루프가 하나씩 진행될때마다 1씩증가하는 인덱스를 얻어오기위한.  */
			_uint		iIndex = i * m_iNumVerticesX + j;

			_uint		iIndices[4] = {
				iIndex + m_iNumVerticesX,
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1,
				iIndex
			};

			pIndices[iNumFaces]._0 = iIndices[0];
			pIndices[iNumFaces]._1 = iIndices[1];
			pIndices[iNumFaces]._2 = iIndices[2];
			++iNumFaces;

			pIndices[iNumFaces]._0 = iIndices[0];
			pIndices[iNumFaces]._1 = iIndices[2];
			pIndices[iNumFaces]._2 = iIndices[3];
			++iNumFaces;
		}
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pIndices;

	if (FAILED(__super::Create_IndexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CVIBuffer_Tile::Map(D3D11_MAPPED_SUBRESOURCE * pSubResource)
{
	if (pSubResource == nullptr)
		return E_FAIL;

	m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, pSubResource);
	return S_OK;
}

HRESULT CVIBuffer_Tile::UnMap()
{
	m_pContext->Unmap(m_pVB, 0);
	return S_OK;
}

_bool CVIBuffer_Tile::Picking(CTransform * pTransform, _float3 * pOut, VTXINDEX& VtxIndex)
{
	CPicking*		pPicking = CPicking::Get_Instance();

	Safe_AddRef(pPicking);


	_float4x4		WorldMatrixInv;
	XMStoreFloat4x4(&WorldMatrixInv, pTransform->Get_WorldMatrixInverse());

	_float3			vRayDir , vRayPos;

	pPicking->Compute_LocalRayInfo(&vRayDir, &vRayPos, pTransform);

	//XMVector3Normalize(XMLoadFloat3(&vRayDir));
	int Index = 0;
	for (_uint i = 0; i < m_iNumVerticesZ - 1; ++i)
	{
		for (_uint j = 0; j < m_iNumVerticesX - 1; ++j)
		{
			_uint		iIndex = i * m_iNumVerticesX + j;

			_uint		iIndices[] = {
				iIndex + m_iNumVerticesX,
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1,
				iIndex
			};
			_matrix	WorldMatrix = pTransform->Get_WorldMatrix();
			_float vDist = { 0.f };			
			_vector RayPos = XMLoadFloat3(&vRayPos);			
			_vector RayDir = XMLoadFloat3(&vRayDir);
			_vector Index0 = XMLoadFloat3(&m_pVerticesPos[iIndices[0]]);
			_vector Index1 = XMLoadFloat3(&m_pVerticesPos[iIndices[1]]);
			_vector Index2 = XMLoadFloat3(&m_pVerticesPos[iIndices[2]]);
			_vector Index3 = XMLoadFloat3(&m_pVerticesPos[iIndices[3]]);

			RayDir = XMVector3Normalize(RayDir);

			if (true == TriangleTests::Intersects (RayPos, RayDir, Index0,
				Index1, Index2, vDist))
			{
				
				_vector	vPickPos = XMLoadFloat3(&vRayPos) + XMLoadFloat3(&vRayDir) * vDist;

				XMStoreFloat3(pOut, XMVector3TransformCoord(vPickPos, WorldMatrix));
				VtxIndex.Index[0] = iIndices[0];
				VtxIndex.Index[1] = iIndices[1];
				VtxIndex.Index[2] = iIndices[2];
				Safe_Release(pPicking);
				return true;
			}

			if (true == TriangleTests::Intersects(RayPos, RayDir, Index0,
				Index2, Index3, vDist))
			{
				_vector	vPickPos = XMLoadFloat3(&vRayPos) + XMLoadFloat3(&vRayDir) * vDist;

				XMStoreFloat3(pOut, XMVector3TransformCoord(vPickPos, WorldMatrix));
				VtxIndex.Index[0] = iIndices[0];
				VtxIndex.Index[1] = iIndices[2];
				VtxIndex.Index[2] = iIndices[3];
				Safe_Release(pPicking);
				return true;
			}
		}
	}

	Safe_Release(pPicking);
	return false;
}

CVIBuffer_Tile * CVIBuffer_Tile::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CVIBuffer_Tile*			pInstance = new CVIBuffer_Tile(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CVIBuffer_Tile"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Tile::Clone(void * pArg)
{
	CVIBuffer_Tile*			pInstance = new CVIBuffer_Tile(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVIBuffer_Tile"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Tile::Free()
{
	Safe_Delete_Array(m_pVerticesPos);
	__super::Free();
}
