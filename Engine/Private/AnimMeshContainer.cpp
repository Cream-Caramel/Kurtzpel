#include "..\Public\AnimMeshContainer.h"
#include "AnimModel.h"
#include "HierarchyNode.h"

CAnimMeshContainer::CAnimMeshContainer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CAnimMeshContainer::CAnimMeshContainer(const CAnimMeshContainer & rhs)
	: CVIBuffer(rhs)
	, m_iMaterialIndex(rhs.m_iMaterialIndex)
	, m_iNumBones(rhs.m_iNumBones)
{
	strcpy_s(m_szName, rhs.m_szName);
}

HRESULT CAnimMeshContainer::Initialize_Prototype(const aiMesh * pAIMesh, CAnimModel* pModel, _fmatrix PivotMatrix, CBinary* pBinary)
{
	strcpy_s(m_szName, pAIMesh->mName.data);
	m_iMaterialIndex = pAIMesh->mMaterialIndex;

	char* temp = new char[256];

	for (int i = 0; i < 256; ++i)
	{
		temp[i] = m_szName[i];
	}

	pBinary->m_BinaryVector->MeshName.MeshName.push_back(temp);

	pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex.push_back(m_iMaterialIndex);

#pragma region VERTEXBUFFER

	HRESULT		hr = 0;

	hr = Ready_AnimVertices(pAIMesh, pModel, pBinary);

	if (FAILED(hr))
		return E_FAIL;

#pragma endregion

#pragma region INDEXBUFFER
	m_iNumPrimitives = pAIMesh->mNumFaces;
	pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives.push_back(m_iNumPrimitives);
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


	FACEINDICES32*		pIndices = new FACEINDICES32[m_iNumPrimitives];
	ZeroMemory(pIndices, sizeof(FACEINDICES32) * m_iNumPrimitives);

	for (_uint i = 0; i < m_iNumPrimitives; ++i)
	{
		pIndices[i]._0 = pAIMesh->mFaces[i].mIndices[0];
		pIndices[i]._1 = pAIMesh->mFaces[i].mIndices[1];
		pIndices[i]._2 = pAIMesh->mFaces[i].mIndices[2];
		pBinary->m_BinaryVector->pIndices.pIndices.push_back(pIndices[i]);
	}


	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pIndices;

	if (FAILED(__super::Create_IndexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

	return S_OK;
}

HRESULT CAnimMeshContainer::Initialize_Prototype(CAnimModel * pModel, _fmatrix PivotMatrix, CBinary * pBinary)
{
	strcpy_s(m_szName, pBinary->m_BinaryVector->MeshName.MeshName[pBinary->MeshNameIndex++]);
	m_iMaterialIndex = pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex[pBinary->MaterialIndex++];

#pragma region VERTEXBUFFER

	HRESULT		hr = 0;

	hr = Ready_AnimVertices(pModel, pBinary);

	if (FAILED(hr))
		return E_FAIL;

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


	FACEINDICES32*		pIndices = new FACEINDICES32[m_iNumPrimitives];
	ZeroMemory(pIndices, sizeof(FACEINDICES32) * m_iNumPrimitives);

	for (_uint i = 0; i < m_iNumPrimitives; ++i)
	{
		pIndices[i]= pBinary->m_BinaryVector->pIndices.pIndices[pBinary->pIndicesIndex];		
		pBinary->pIndicesIndex++;
	}


	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pIndices;

	if (FAILED(__super::Create_IndexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

	return S_OK;
}

HRESULT CAnimMeshContainer::Initialize(void * pArg)
{
	return S_OK;
}

/* 메시의 정점을 그리기위해 셰이더에 넘기기위한 뼈행렬의 배열을 구성한다. */
void CAnimMeshContainer::SetUp_BoneMatrices(_float4x4 * pBoneMatrices, _fmatrix PivotMatrix)
{
	if (0 == m_iNumBones)
	{
		XMStoreFloat4x4(&pBoneMatrices[0], XMMatrixIdentity());
		return;
	}

	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		XMStoreFloat4x4(&pBoneMatrices[i], XMMatrixTranspose(m_Bones[i]->Get_OffSetMatrix() * m_Bones[i]->Get_CombinedTransformation() * PivotMatrix));
	}

}

HRESULT CAnimMeshContainer::SetUp_HierarchyNodes(CAnimModel * pModel, CBinary* pBinary)
{

	/* 현재 메시에 영향ㅇ르 ㅈ2ㅜ는 뼈들을 순회한다ㅏ. */
	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		CHierarchyNode*		pHierarchyNode = pModel->Get_HierarchyNode(pBinary->m_BinaryVector->MeshBoneName.MeshBoneName[pBinary->MeshBoneNameIndex++]);

		_float4x4			OffsetMatrix;

		OffsetMatrix = pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix[pBinary->OffSetMatrixIndex++];

		pHierarchyNode->Set_OffsetMatrix(XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));

		m_Bones.push_back(pHierarchyNode);

		Safe_AddRef(pHierarchyNode);
	}
	pBinary->OffSetMatrixIndex = 0;
	pBinary->MeshBoneNameIndex = 0;

	if (0 == m_iNumBones)
	{
		CHierarchyNode*		pNode = pModel->Get_HierarchyNode(m_szName);

		if (nullptr == pNode)
			return S_OK;

		m_iNumBones = 1;

		m_Bones.push_back(pNode);
	}

	return S_OK;
}



HRESULT CAnimMeshContainer::Ready_AnimVertices(const aiMesh* pAIMesh, CAnimModel* pModel, CBinary* pBinary)
{
	m_iNumVertexBuffers = 1;
	m_iNumVertices = pAIMesh->mNumVertices;
	pBinary->m_BinaryVector->NumVertices.NumVertices.push_back(m_iNumVertices);
	m_iStride = sizeof(VTXANIMMODEL);

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumVertices * m_iStride;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXANIMMODEL*		pVertices = new VTXANIMMODEL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMODEL) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		memcpy(&pVertices[i].vTexture, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
		pBinary->m_BinaryVector->VerticesInfo.VerticesInfo.push_back(pVertices[i]);
	}

	m_iNumBones = pAIMesh->mNumBones;
	pBinary->m_BinaryVector->NumBones.iNumBones.push_back(m_iNumBones);
	/* 현재 메시에 영향ㅇ르 ㅈ2ㅜ는 뼈들을 순회한다ㅏ. */
	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		aiBone*		pAIBone = pAIMesh->mBones[i];
		pBinary->m_BinaryVector->NumWeights.iNumWeights.push_back(pAIBone->mNumWeights);
		/* i번째 뼈가 어떤 정점들에게 영향ㅇ르 주는지 순회한다. */
		for (_uint j = 0; j < pAIBone->mNumWeights; ++j)
		{
			_uint		iVertexIndex = pAIBone->mWeights[j].mVertexId;
			pBinary->m_BinaryVector->VertexIndex.VertexIndex.push_back(pAIBone->mWeights[j].mVertexId);
			if (0.0f == pVertices[iVertexIndex].vBlendWeight.x)
			{
				pVertices[iVertexIndex].vBlendIndex.x = i;
				pVertices[iVertexIndex].vBlendWeight.x = pAIBone->mWeights[j].mWeight;	
				pBinary->m_BinaryVector->BlendWeights.BlendWeights.push_back(pAIBone->mWeights[j].mWeight);
			}

			else if (0.0f == pVertices[iVertexIndex].vBlendWeight.y)
			{
				pVertices[iVertexIndex].vBlendIndex.y = i;
				pVertices[iVertexIndex].vBlendWeight.y = pAIBone->mWeights[j].mWeight;		
				pBinary->m_BinaryVector->BlendWeights.BlendWeights.push_back(pAIBone->mWeights[j].mWeight);
			}

			else if (0.0f == pVertices[iVertexIndex].vBlendWeight.z)
			{
				pVertices[iVertexIndex].vBlendIndex.z = i;
				pVertices[iVertexIndex].vBlendWeight.z = pAIBone->mWeights[j].mWeight;	
				pBinary->m_BinaryVector->BlendWeights.BlendWeights.push_back(pAIBone->mWeights[j].mWeight);
			}

			else if (0.0f == pVertices[iVertexIndex].vBlendWeight.w)
			{
				pVertices[iVertexIndex].vBlendIndex.w = i;
				pVertices[iVertexIndex].vBlendWeight.w = pAIBone->mWeights[j].mWeight;	
				pBinary->m_BinaryVector->BlendWeights.BlendWeights.push_back(pAIBone->mWeights[j].mWeight);
			}
		
		}
	}



	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CAnimMeshContainer::Ready_AnimVertices(CAnimModel * pModel, CBinary * pBinary)
{
	m_iNumVertexBuffers = 1;
	m_iNumVertices = pBinary->m_BinaryVector->NumVertices.NumVertices[pBinary->NumVerticesIndex++];
	m_iStride = sizeof(VTXANIMMODEL);

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumVertices * m_iStride;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXANIMMODEL*		pVertices = new VTXANIMMODEL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMODEL) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		pVertices[i] = pBinary->m_BinaryVector->VerticesInfo.VerticesInfo[pBinary->VerticesInfoIndex++];
	}

	m_iNumBones = pBinary->m_BinaryVector->NumBones.iNumBones[pBinary->NumBonesIndex++];

	/* 현재 메시에 영향ㅇ르 ㅈ2ㅜ는 뼈들을 순회한다ㅏ. */
	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		/* i번째 뼈가 어떤 정점들에게 영향ㅇ르 주는지 순회한다. */
		for (_uint j = 0; j < pBinary->m_BinaryVector->NumWeights.iNumWeights[pBinary->NumWeightsIndex]; ++j)
		{
			_uint		iVertexIndex = pBinary->m_BinaryVector->VertexIndex.VertexIndex[pBinary->VertexIndex++];

			if (0.0f == pVertices[iVertexIndex].vBlendWeight.x)
			{
				pVertices[iVertexIndex].vBlendIndex.x = i;
				pVertices[iVertexIndex].vBlendWeight.x = pBinary->m_BinaryVector->BlendWeights.BlendWeights[pBinary->BlendWeightsIndex++];
			}

			else if (0.0f == pVertices[iVertexIndex].vBlendWeight.y)
			{
				pVertices[iVertexIndex].vBlendIndex.y = i;
				pVertices[iVertexIndex].vBlendWeight.y = pBinary->m_BinaryVector->BlendWeights.BlendWeights[pBinary->BlendWeightsIndex++];
			}

			else if (0.0f == pVertices[iVertexIndex].vBlendWeight.z)
			{
				pVertices[iVertexIndex].vBlendIndex.z = i;
				pVertices[iVertexIndex].vBlendWeight.z = pBinary->m_BinaryVector->BlendWeights.BlendWeights[pBinary->BlendWeightsIndex++];
			}

			else if (0.0f == pVertices[iVertexIndex].vBlendWeight.w)
			{
				pVertices[iVertexIndex].vBlendIndex.w = i;
				pVertices[iVertexIndex].vBlendWeight.w = pBinary->m_BinaryVector->BlendWeights.BlendWeights[pBinary->BlendWeightsIndex++];
			}
		}
		pBinary->NumWeightsIndex++;
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

CAnimMeshContainer * CAnimMeshContainer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const aiMesh * pAIMesh, CAnimModel* pModel, _fmatrix PivotMatrix, CBinary* pBinary)
{
	CAnimMeshContainer*			pInstance = new CAnimMeshContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pAIMesh, pModel, PivotMatrix, pBinary)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimMeshContainer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimMeshContainer * CAnimMeshContainer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, CAnimModel * pModel, _fmatrix PivotMatrix, CBinary * pBinary)
{
	CAnimMeshContainer*			pInstance = new CAnimMeshContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pModel, PivotMatrix, pBinary)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimMeshContainer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CAnimMeshContainer::Clone(void * pArg)
{
	CAnimMeshContainer*			pInstance = new CAnimMeshContainer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CAnimMeshContainer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimMeshContainer::Free()
{
	__super::Free();

	for (auto& pHierarchyNode : m_Bones)
		Safe_Release(pHierarchyNode);

	m_Bones.clear();

}
