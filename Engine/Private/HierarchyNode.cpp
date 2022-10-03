#include "..\Public\HierarchyNode.h"

CHierarchyNode::CHierarchyNode()
{

}

HRESULT CHierarchyNode::Initialize(aiNode * pAINode, CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary)
{
	m_iChildRen = pAINode->mNumChildren;

	strcpy_s(m_szName, pAINode->mName.data);

	char* temp = new char[256];

	for (int i = 0; i < 256; ++i)
	{
		temp[i] = m_szName[i];
	}

	pBinary->m_BinaryVector->NumChildRen.iNumChildren.push_back(m_iChildRen);
	pBinary->m_BinaryVector->HierarchyName.HierarchyName.push_back(temp);

	XMStoreFloat4x4(&m_OffsetMatrix, XMMatrixIdentity());

	memcpy(&m_Transformation, &pAINode->mTransformation, sizeof(_float4x4));
	XMStoreFloat4x4(&m_Transformation, XMMatrixTranspose(XMLoadFloat4x4(&m_Transformation)));

	pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform.push_back(m_Transformation);

	m_iDepth = iDepth;
	m_pParent = pParent;

	XMStoreFloat4x4(&m_CombinedTransformation, XMMatrixIdentity());

	Safe_AddRef(m_pParent);

	return S_OK;
}

HRESULT CHierarchyNode::Initialize(CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary)
{
	m_iChildRen = pBinary->m_BinaryVector->NumChildRen.iNumChildren[pBinary->NumChildRenIndex++];

	strcpy_s(m_szName, pBinary->m_BinaryVector->HierarchyName.HierarchyName[pBinary->HierarchyNameIndex++]);

	XMStoreFloat4x4(&m_OffsetMatrix, XMMatrixIdentity());

	m_Transformation = pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform[pBinary->HierarchyTransformIndex++];

	m_iDepth = iDepth;
	m_pParent = pParent;

	XMStoreFloat4x4(&m_CombinedTransformation, XMMatrixIdentity());

	Safe_AddRef(m_pParent);

	return S_OK;
}

void CHierarchyNode::Set_CombinedTransformation()
{
	if (nullptr != m_pParent)
		XMStoreFloat4x4(&m_CombinedTransformation, XMLoadFloat4x4(&m_Transformation) * XMLoadFloat4x4(&m_pParent->m_CombinedTransformation));
	else
		m_CombinedTransformation = m_Transformation;
}

void CHierarchyNode::Set_OffsetMatrix(_fmatrix OffsetMatrix)
{
	XMStoreFloat4x4(&m_OffsetMatrix, OffsetMatrix);
}

CHierarchyNode * CHierarchyNode::Create(aiNode * pAINode, CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary)
{
	CHierarchyNode*			pInstance = new CHierarchyNode();

	if (FAILED(pInstance->Initialize(pAINode, pParent, iDepth, pBinary)))
	{
		MSG_BOX(TEXT("Failed To Created : CHierarchyNode"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CHierarchyNode * CHierarchyNode::Create(CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary)
{
	CHierarchyNode*			pInstance = new CHierarchyNode();

	if (FAILED(pInstance->Initialize(pParent, iDepth, pBinary)))
	{
		MSG_BOX(TEXT("Failed To Created : CHierarchyNode"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHierarchyNode::Free()
{
	Safe_Release(m_pParent);
}
