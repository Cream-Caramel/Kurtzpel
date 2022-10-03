#include "..\Public\AnimModel.h"
#include "AnimMeshContainer.h"
#include "Texture.h"
#include "HierarchyNode.h"
#include "Animation.h"
#include "Shader.h"


CAnimModel::CAnimModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{
}

CAnimModel::CAnimModel(const CAnimModel & rhs)
	: CComponent(rhs)
	, m_pAIScene(rhs.m_pAIScene)
	, m_iNumMeshes(rhs.m_iNumMeshes)
	, m_iNumMaterials(rhs.m_iNumMaterials)
	, m_Meshes(rhs.m_Meshes)
	, m_Materials(rhs.m_Materials)
	, m_Animations(rhs.m_Animations)
	, m_iCurrentAnimIndex(rhs.m_iCurrentAnimIndex)
	, m_PivotMatrix(rhs.m_PivotMatrix)
	, m_iNumAnimations(rhs.m_iNumAnimations)
	, m_pBinary(rhs.m_pBinary)

{
	for (auto& pMeshContainer : m_Meshes)
		Safe_AddRef(pMeshContainer);


	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
			Safe_AddRef(Material.pTexture[i]);
	}

	for (auto& pAnimation : m_Animations)
		Safe_AddRef(pAnimation);

	Safe_AddRef(m_pBinary);
}

CHierarchyNode * CAnimModel::Get_HierarchyNode(const char * pNodeName)
{
	auto	iter = find_if(m_HierarchyNodes.begin(), m_HierarchyNodes.end(), [&](CHierarchyNode* pNode)
	{
		return !strcmp(pNodeName, pNode->Get_Name());
	});

	if (iter == m_HierarchyNodes.end())
		return nullptr;

	return *iter;
}

_uint CAnimModel::Get_MaterialIndex(_uint iMeshIndex)
{
	return m_Meshes[iMeshIndex]->Get_MaterialIndex();
}

void CAnimModel::Set_AnimIndex(_uint iAnimIndex)
{
	m_iCurrentAnimIndex = iAnimIndex;
	m_Animations[m_iCurrentAnimIndex]->ResetChennel();
	m_Animations[m_iCurrentAnimIndex]->SetPlayTime(0.f);	
}

HRESULT CAnimModel::Initialize_Prototype(const char * pModelFilePath, const char * pModelFileName, const char* pSaveName, _fmatrix PivotMatrix)
{
	m_pBinary = new CBinary;

	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	char		szFullPath[256] = "";

	strcpy_s(szFullPath, pModelFilePath);
	strcat_s(szFullPath, pModelFileName);

	_uint		iFlag = 0;

	iFlag |= aiProcess_ConvertToLeftHanded | aiProcess_GlobalScale | aiProcess_CalcTangentSpace;

	m_pAIScene = m_Importer.ReadFile(szFullPath, iFlag);

	if (nullptr == m_pAIScene)
		return E_FAIL;

	Ready_HierarchyNodes(m_pAIScene->mRootNode, nullptr, 0, m_pBinary);

	sort(m_HierarchyNodes.begin(), m_HierarchyNodes.end(), [](CHierarchyNode* pSour, CHierarchyNode* pDest)
	{
		return pSour->Get_Depth() < pDest->Get_Depth();
	});

	/* ���� �����ϴ� �޽õ��� �����. */
	if (FAILED(Ready_MeshContainers(PivotMatrix, m_pBinary)))
		return E_FAIL;

	if (FAILED(Ready_Materials(pModelFilePath, m_pBinary)))
		return E_FAIL;

	if (FAILED(Ready_Animations(m_pBinary)))
		return E_FAIL;

	_uint iNumMeshes = 0;

	for (auto& pMeshContainer : m_Meshes)
	{
		for (_uint i = 0; i < m_pBinary->m_BinaryVector->NumBones.iNumBones[iNumMeshes]; ++i)
		{
			aiBone*		pAIBone = m_pAIScene->mMeshes[iNumMeshes]->mBones[i];
			char BoneName[256] = "";
			strcpy_s(BoneName, pAIBone->mName.data);

			char* temp = new char[256];

			for (int i = 0; i < 256; ++i)
			{
				temp[i] = BoneName[i];
			}
			m_pBinary->m_BinaryVector->MeshBoneName.MeshBoneName.push_back(temp);

			CHierarchyNode*		pHierarchyNode = Get_HierarchyNode(pAIBone->mName.data);

			_float4x4			OffsetMatrix;

			memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));

			m_pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix.push_back(OffsetMatrix);
		}
	}

	SaveBinary(pSaveName);
	return S_OK;
}

HRESULT CAnimModel::Initialize_Prototype(const char * pLoadName, _fmatrix PivotMatrix)
{
	m_pBinary = new CBinary;
	LoadBinary(pLoadName);
	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	Ready_LHierarchyNodes(nullptr, 0, m_pBinary);

	sort(m_HierarchyNodes.begin(), m_HierarchyNodes.end(), [](CHierarchyNode* pSour, CHierarchyNode* pDest)
	{
		return pSour->Get_Depth() < pDest->Get_Depth();
	});

	/* ���� �����ϴ� �޽õ��� �����. */
	if (FAILED(Ready_LMeshContainers(PivotMatrix, m_pBinary)))
		return E_FAIL;

	if (FAILED(Ready_LMaterials(m_pBinary)))
		return E_FAIL;


	if (FAILED(Ready_LAnimations(m_pBinary)))
		return E_FAIL;

	m_pBinary->ResetIndex();

	return S_OK;
}

HRESULT CAnimModel::Initialize(void * pArg)
{
	Ready_LHierarchyNodes(nullptr, 0, m_pBinary);

	sort(m_HierarchyNodes.begin(), m_HierarchyNodes.end(), [](CHierarchyNode* pSour, CHierarchyNode* pDest)
	{
		return pSour->Get_Depth() < pDest->Get_Depth();
	});

	m_pBinary->HierarchyNameIndex = 0;
	m_pBinary->NumChildRenIndex = 0;
	m_pBinary->HierarchyTransformIndex = 0;

	vector<CAnimMeshContainer*>		MeshContainers;

	for (auto& pPrototype : m_Meshes)
	{
		CAnimMeshContainer*		pMeshContainer = (CAnimMeshContainer*)pPrototype->Clone();
		if (nullptr == pMeshContainer)
			return E_FAIL;

		MeshContainers.push_back(pMeshContainer);

		Safe_Release(pPrototype);
	}

	m_Meshes.clear();

	m_Meshes = MeshContainers;

	for (auto& pMeshContainer : m_Meshes)
	{
		if (nullptr != pMeshContainer)
			pMeshContainer->SetUp_HierarchyNodes(this, m_pBinary);
	}

	vector<CAnimation*>		Animations;

	for (auto& pPrototype : m_Animations)
	{
		CAnimation*		pAnimation = pPrototype->Clone(this);
		if (nullptr == pAnimation)
			return E_FAIL;

		Animations.push_back(pAnimation);

		Safe_Release(pPrototype);
	}

	m_Animations.clear();

	m_Animations = Animations;
	m_pBinary->ResetIndex();
	return S_OK;
}

HRESULT CAnimModel::SetUp_OnShader(CShader * pShader, _uint iMaterialIndex, aiTextureType eTextureType, const char * pConstantName)
{
	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	return m_Materials[iMaterialIndex].pTexture[eTextureType]->Set_SRV(pShader, pConstantName);
}

HRESULT CAnimModel::Play_Animation(_float fTimeDelta, CAnimModel* pAnimModel)
{
	if (m_iCurrentAnimIndex >= m_iNumAnimations)
		return E_FAIL;

	/* ���� ����ϰ����ϴ� �ִϸ��̼��� �����ؾ��� ������ ��������� �����س���. */
	m_bAniEnd = m_Animations[m_iCurrentAnimIndex]->Play_Animation(fTimeDelta, pAnimModel);

	/* ��������� ����������(�θ𿡼� �ڽ�����) �����Ͽ� m_CombinedTransformation�� �����.  */
	for (auto& pHierarchyNode : m_HierarchyNodes)
	{
		pHierarchyNode->Set_CombinedTransformation();
	}

	return S_OK;
}

HRESULT CAnimModel::Render(CShader* pShader, _uint iMeshIndex)
{
	_float4x4		BoneMatrices[256];

	m_Meshes[iMeshIndex]->SetUp_BoneMatrices(BoneMatrices, XMLoadFloat4x4(&m_PivotMatrix));

	if (FAILED(pShader->Set_RawValue("g_BoneMatrices", BoneMatrices, sizeof(_float4x4) * 256)))
		return E_FAIL;	

	pShader->Begin(0);

	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

const char * CAnimModel::Get_Name(int AnimIndex)
{
	return m_Animations[AnimIndex]->Get_Name();
}

void CAnimModel::Set_Name(const char * AnimName, int AnimIndex)
{
	m_Animations[AnimIndex]->Set_Name(AnimName);
}

void CAnimModel::ChangeAnimIndex(int Index1, int Index2)
{
	CAnimation* temp = m_Animations[Index1];
	m_Animations[Index1] = m_Animations[Index2];
	m_Animations[Index2] = temp;	
}

HRESULT CAnimModel::Ready_MeshContainers(_fmatrix PivotMatrix, CBinary* pBinary)
{
	m_iNumMeshes = m_pAIScene->mNumMeshes;
	pBinary->m_BinaryData->NumMeshes.iNumMeshes = m_iNumMeshes;
	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		CAnimMeshContainer*		pMeshContainer = CAnimMeshContainer::Create(m_pDevice, m_pContext, m_pAIScene->mMeshes[i], this, PivotMatrix, pBinary);
		if (nullptr == pMeshContainer)
			return E_FAIL;

		m_Meshes.push_back(pMeshContainer);
	}

	return S_OK;
}

HRESULT CAnimModel::Ready_Materials(const char* pModelFilePath, CBinary* pBinary)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	m_iNumMaterials = m_pAIScene->mNumMaterials;
	pBinary->m_BinaryData->NumMaterials.iNumMaterials = m_iNumMaterials;

	for (_uint i = 0; i < m_iNumMaterials; ++i)
	{
		MATERIALDESC		MaterialDesc;
		ZeroMemory(&MaterialDesc, sizeof(MATERIALDESC));

		aiMaterial*			pAIMaterial = m_pAIScene->mMaterials[i];

		for (_uint j = 0; j < AI_TEXTURE_TYPE_MAX; ++j)
		{
			aiString		strPath;

			if (FAILED(pAIMaterial->GetTexture(aiTextureType(j), 0, &strPath)))
			{

				_tchar Null[256] = { 0 };
				_tchar* temp = new _tchar[256];
				for (int i = 0; i < 256; ++i)
					temp[i] = Null[i];
				pBinary->m_BinaryVector->TexturePath.TexturePath.push_back(temp);
				continue;
			}
				

			char			szFullPath[256] = "";
			char			szFileName[256] = "";
			char			szExt[256] = "";

			_splitpath_s(strPath.data, nullptr, 0, nullptr, 0, szFileName, 256, szExt, 256);

			strcpy_s(szFullPath, pModelFilePath);
			strcat_s(szFullPath, szFileName);
			strcat_s(szFullPath, szExt);

			_tchar			szWideFullPath[256] = TEXT("");

			MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szWideFullPath, 256);


			MaterialDesc.pTexture[j] = CTexture::Create(m_pDevice, m_pContext, szWideFullPath);
			if (nullptr == MaterialDesc.pTexture[j])
				return E_FAIL;

			_tchar* temp = new _tchar[256];
			for (int i = 0; i < 256; ++i)
			{
				temp[i] = szWideFullPath[i];
			}
			pBinary->m_BinaryVector->TexturePath.TexturePath.push_back(temp);
		}

		m_Materials.push_back(MaterialDesc);
	}

	return S_OK;
}

HRESULT CAnimModel::Ready_HierarchyNodes(aiNode* pNode, CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary)
{
	CHierarchyNode*		pHierarchyNode = CHierarchyNode::Create(pNode, pParent, iDepth++, pBinary);

	if (nullptr == pHierarchyNode)
		return E_FAIL;
	
	m_HierarchyNodes.push_back(pHierarchyNode);

	for (_uint i = 0; i < pHierarchyNode->Get_ChildRen(); ++i)
	{
		Ready_HierarchyNodes(pNode->mChildren[i], pHierarchyNode, iDepth, pBinary);
	}

	return S_OK;
}

HRESULT CAnimModel::Ready_Animations(CBinary* pBinary)
{
	m_iNumAnimations = m_pAIScene->mNumAnimations;
	pBinary->m_BinaryData->NumAnimations.iNumAnimations = m_iNumAnimations;

	for (_uint i = 0; i < m_pAIScene->mNumAnimations; ++i)
	{
		aiAnimation*		pAIAnimation = m_pAIScene->mAnimations[i];

		CAnimation*			pAnimation = CAnimation::Create(pAIAnimation, pBinary);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}
	return S_OK;
}

HRESULT CAnimModel::Ready_LMeshContainers(_fmatrix PivotMatrix, CBinary * pBinary)
{
	m_iNumMeshes = pBinary->m_BinaryData->NumMeshes.iNumMeshes;

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		CAnimMeshContainer*		pMeshContainer = CAnimMeshContainer::Create(m_pDevice, m_pContext, this, PivotMatrix, pBinary);
		if (nullptr == pMeshContainer)
			return E_FAIL;

		m_Meshes.push_back(pMeshContainer);
	}

	return S_OK;
}

HRESULT CAnimModel::Ready_LMaterials(CBinary * pBinary)
{
	m_iNumMaterials = pBinary->m_BinaryData->NumMaterials.iNumMaterials;

	for (_uint i = 0; i < m_iNumMaterials; ++i)
	{
		MATERIALDESC		MaterialDesc;
		ZeroMemory(&MaterialDesc, sizeof(MATERIALDESC));


		for (_uint j = 0; j < 18; ++j)
		{
			if (!wcscmp(pBinary->m_BinaryVector->TexturePath.TexturePath[pBinary->TexturePathIndex], L""))
			{
				pBinary->TexturePathIndex++;
				continue;
			}

			MaterialDesc.pTexture[j] = CTexture::Create(m_pDevice, m_pContext, pBinary->m_BinaryVector->TexturePath.TexturePath[pBinary->TexturePathIndex++]);
			if (nullptr == MaterialDesc.pTexture[j])
				return E_FAIL;
		}

		m_Materials.push_back(MaterialDesc);
	}

	return S_OK;
}

HRESULT CAnimModel::Ready_LHierarchyNodes(CHierarchyNode * pParent, _uint iDepth, CBinary * pBinary)
{
	CHierarchyNode*		pHierarchyNode = CHierarchyNode::Create(pParent, iDepth++, pBinary);

	if (nullptr == pHierarchyNode)
		return E_FAIL;

	m_HierarchyNodes.push_back(pHierarchyNode);

	for (_uint i = 0; i < pHierarchyNode->Get_ChildRen(); ++i)
	{
		Ready_LHierarchyNodes(pHierarchyNode, iDepth, pBinary);
	}

	return S_OK;
}

HRESULT CAnimModel::Ready_LAnimations(CBinary * pBinary)
{
	m_iNumAnimations = pBinary->m_BinaryData->NumAnimations.iNumAnimations;

	for (_uint i = 0; i < m_iNumAnimations; ++i)
	{
		CAnimation*			pAnimation = CAnimation::Create(pBinary);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}
	return S_OK;
}

_float CAnimModel::GetTickPerSecond()
{
	return m_Animations[m_iCurrentAnimIndex]->GetTickPerSecond();
}

void CAnimModel::SetTickPerSecond(_float fSecond)
{
	m_Animations[m_iCurrentAnimIndex]->SetTickPerSecond(fSecond);
}

_float CAnimModel::GetDuration()
{
	return m_Animations[m_iCurrentAnimIndex]->GetDuration();
}

void CAnimModel::SetDuration(_float fDuration)
{
	m_Animations[m_iCurrentAnimIndex]->SetDuration(fDuration);
}

_float CAnimModel::GetPlayTime()
{
	return m_Animations[m_iCurrentAnimIndex]->GetPlayTime();
}

void CAnimModel::SetPlayTime(_float fPlayTime)
{
	m_Animations[m_iCurrentAnimIndex]->SetPlayTime(fPlayTime);
}

_float CAnimModel::GetTimeLimit()
{
	return m_Animations[m_iCurrentAnimIndex]->GetTimeLimit(); 
}

void CAnimModel::SetTimeLimit(_float TimeLimit)
{
	m_Animations[m_iCurrentAnimIndex]->SetTimeLimit(TimeLimit); 
}

void CAnimModel::ResetKeyFrame()
{
	m_Animations[m_iCurrentAnimIndex]->ResetChennel();
}

void CAnimModel::SetNextIndex(_uint iNextIndex)
{
	m_iNextAnimIndex = iNextIndex;
}

void CAnimModel::DeleteAnimation(int Index)
{
	Safe_Release(m_Animations[Index]);
	m_Animations.erase(m_Animations.begin() + Index);
	m_iNumAnimations = m_Animations.size();
}

void CAnimModel::SaveBinary(const char* FileName)
{
	string FileSave = FileName;

	string temp = "../Data/AnimModelBinary/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hReadFile = CreateFile(FilePath,			// ���� ��ο� �̸� ���
		GENERIC_READ,				// ���� ���� ��� (GENERIC_WRITE ���� ����, GENERIC_READ �б� ����)
		NULL,						// �������, ������ �����ִ� ���¿��� �ٸ� ���μ����� ������ �� ����� ���ΰ�, NULL�� ��� �������� �ʴ´�
		NULL,						// ���� �Ӽ�, �⺻��	
		OPEN_EXISTING,				// ���� ���, CREATE_ALWAYS�� ������ ���ٸ� ����, �ִٸ� ���� ����, OPEN_EXISTING ������ ���� ��쿡�� ����
		FILE_ATTRIBUTE_NORMAL,		// ���� �Ӽ�(�б� ����, ���� ��), FILE_ATTRIBUTE_NORMAL �ƹ��� �Ӽ��� ���� �Ϲ� ���� ����
		NULL);						// �������� ������ �Ӽ��� ������ ���ø� ����, �츮�� ������� �ʾƼ� NULL

	if (INVALID_HANDLE_VALUE != hReadFile)
	{
		return;
	}


	HANDLE		hFile = CreateFile(FilePath,			// ���� ��ο� �̸� ���
		GENERIC_WRITE,				// ���� ���� ��� (GENERIC_WRITE ���� ����, GENERIC_READ �б� ����)
		NULL,						// �������, ������ �����ִ� ���¿��� �ٸ� ���μ����� ������ �� ����� ���ΰ�, NULL�� ��� �������� �ʴ´�
		NULL,						// ���� �Ӽ�, �⺻��	
		CREATE_ALWAYS,				// ���� ���, CREATE_ALWAYS�� ������ ���ٸ� ����, �ִٸ� ���� ����, OPEN_EXISTING ������ ���� ��쿡�� ����
		FILE_ATTRIBUTE_NORMAL,		// ���� �Ӽ�(�б� ����, ���� ��), FILE_ATTRIBUTE_NORMAL �ƹ��� �Ӽ��� ���� �Ϲ� ���� ����
		NULL);						// �������� ������ �Ӽ��� ������ ���ø� ����, �츮�� ������� �ʾƼ� NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// �˾� â�� ������ִ� ����� �Լ�
		// 1. �ڵ� 2. �˾� â�� �������ϴ� �޽��� 3. �˾� â �̸� 4. ��ư �Ӽ�
		MSG_BOX(TEXT("Fail To Binary"));
		return;
	}

	// 2. ���� ����


	DWORD		dwByte = 0;
//	HierarchyTransform
	
	WriteFile(hFile, m_pBinary->m_BinaryData, sizeof(BINARYDATA), &dwByte, nullptr);

	int HierarchyTransformSize = m_pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform.size();
	WriteFile(hFile, &HierarchyTransformSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < HierarchyTransformSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform[i], sizeof(_float4x4), &dwByte, nullptr);
	}

	int TexPathSize = m_pBinary->m_BinaryVector->TexturePath.TexturePath.size();
	WriteFile(hFile, &TexPathSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TexPathSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->TexturePath.TexturePath[i], sizeof(_tchar) * 256, &dwByte, nullptr);
	}

	int NumChildSize = m_pBinary->m_BinaryVector->NumChildRen.iNumChildren.size();
	WriteFile(hFile, &NumChildSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NumChildSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumChildRen.iNumChildren[i], sizeof(int), &dwByte, nullptr);
	}

	int HierarchyNameSize = m_pBinary->m_BinaryVector->HierarchyName.HierarchyName.size();
	WriteFile(hFile, &HierarchyNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < HierarchyNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->HierarchyName.HierarchyName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int MeshNameSize = m_pBinary->m_BinaryVector->MeshName.MeshName.size();
	WriteFile(hFile, &MeshNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MeshNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->MeshName.MeshName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int MaterialIndexSize = m_pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex.size();
	WriteFile(hFile, &MaterialIndexSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MaterialIndexSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex[i], sizeof(int), &dwByte, nullptr);
	}

	int PrimitiveSize = m_pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives.size();
	WriteFile(hFile, &PrimitiveSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < PrimitiveSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives[i], sizeof(int), &dwByte, nullptr);
	}

	int IndicesSize = m_pBinary->m_BinaryVector->pIndices.pIndices.size();
	WriteFile(hFile, &IndicesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < IndicesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->pIndices.pIndices[i], sizeof(FACEINDICES32), &dwByte, nullptr);
	}

	int OffSetMaxtrixSize = m_pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix.size();
	WriteFile(hFile, &OffSetMaxtrixSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < OffSetMaxtrixSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix[i], sizeof(_float4x4), &dwByte, nullptr);
	}

	int VerticesSize = m_pBinary->m_BinaryVector->NumVertices.NumVertices.size();
	WriteFile(hFile, &VerticesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumVertices.NumVertices[i], sizeof(int), &dwByte, nullptr);
	}

	int VerticesInfoSize = m_pBinary->m_BinaryVector->VerticesInfo.VerticesInfo.size();
	WriteFile(hFile, &VerticesInfoSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesInfoSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->VerticesInfo.VerticesInfo[i], sizeof(VTXANIMMODEL), &dwByte, nullptr);
	}

	int VerticesIndexSize = m_pBinary->m_BinaryVector->VertexIndex.VertexIndex.size();
	WriteFile(hFile, &VerticesIndexSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesIndexSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->VertexIndex.VertexIndex[i], sizeof(int), &dwByte, nullptr);
	}

	int BlendWeightSize = m_pBinary->m_BinaryVector->BlendWeights.BlendWeights.size();
	WriteFile(hFile, &BlendWeightSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < BlendWeightSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->BlendWeights.BlendWeights[i], sizeof(float), &dwByte, nullptr);
	}

	int DurationSize = m_pBinary->m_BinaryVector->Durations.Durations.size();
	WriteFile(hFile, &DurationSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < DurationSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->Durations.Durations[i], sizeof(_float), &dwByte, nullptr);
	}

	int TickPerSecondSize = m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds.size();
	WriteFile(hFile, &TickPerSecondSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TickPerSecondSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds[i], sizeof(_float), &dwByte, nullptr);
	}

	int ChannelSize = m_pBinary->m_BinaryVector->NumChannels.NumChannels.size();
	WriteFile(hFile, &ChannelSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ChannelSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumChannels.NumChannels[i], sizeof(int), &dwByte, nullptr);
	}

	int ChannelNameSize = m_pBinary->m_BinaryVector->ChannelName.ChannelName.size();
	WriteFile(hFile, &ChannelNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ChannelNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->ChannelName.ChannelName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int NumKeyFramesSize = m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames.size();
	WriteFile(hFile, &NumKeyFramesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NumKeyFramesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames[i], sizeof(int), &dwByte, nullptr);
	}

	int KeyFrameSize = m_pBinary->m_BinaryVector->KeyFrames.KeyFrames.size();
	WriteFile(hFile, &KeyFrameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < KeyFrameSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->KeyFrames.KeyFrames[i], sizeof(KEYFRAME), &dwByte, nullptr);
	}

	int BonesSize = m_pBinary->m_BinaryVector->NumBones.iNumBones.size();
	WriteFile(hFile, &BonesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < BonesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumBones.iNumBones[i], sizeof(int), &dwByte, nullptr);
	}

	int WeightSize = m_pBinary->m_BinaryVector->NumWeights.iNumWeights.size();
	WriteFile(hFile, &WeightSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < WeightSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumWeights.iNumWeights[i], sizeof(int), &dwByte, nullptr);
	}

	int MeshBoneNameSize = m_pBinary->m_BinaryVector->MeshBoneName.MeshBoneName.size();
	WriteFile(hFile, &MeshBoneNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MeshBoneNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->MeshBoneName.MeshBoneName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int AnimNameSize = m_pBinary->m_BinaryVector->AnimName.AnimName.size();
	WriteFile(hFile, &AnimNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < AnimNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->AnimName.AnimName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int TimeLimitSize = m_pBinary->m_BinaryVector->TimeLimits.TimeLimits.size();
	WriteFile(hFile, &TimeLimitSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TimeLimitSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->TimeLimits.TimeLimits[i], sizeof(_float), &dwByte, nullptr);
	}

	CloseHandle(hFile);
}

void CAnimModel::SaveEditBinary(const char * FileName)
{
	string FileSave = FileName;

	string temp = "../Data/AnimModelBinary/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// ���� ��ο� �̸� ���
		GENERIC_WRITE,				// ���� ���� ��� (GENERIC_WRITE ���� ����, GENERIC_READ �б� ����)
		NULL,						// �������, ������ �����ִ� ���¿��� �ٸ� ���μ����� ������ �� ����� ���ΰ�, NULL�� ��� �������� �ʴ´�
		NULL,						// ���� �Ӽ�, �⺻��	
		CREATE_ALWAYS,				// ���� ���, CREATE_ALWAYS�� ������ ���ٸ� ����, �ִٸ� ���� ����, OPEN_EXISTING ������ ���� ��쿡�� ����
		FILE_ATTRIBUTE_NORMAL,		// ���� �Ӽ�(�б� ����, ���� ��), FILE_ATTRIBUTE_NORMAL �ƹ��� �Ӽ��� ���� �Ϲ� ���� ����
		NULL);						// �������� ������ �Ӽ��� ������ ���ø� ����, �츮�� ������� �ʾƼ� NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// �˾� â�� ������ִ� ����� �Լ�
		// 1. �ڵ� 2. �˾� â�� �������ϴ� �޽��� 3. �˾� â �̸� 4. ��ư �Ӽ�
		MSG_BOX(TEXT("Fail To Binary"));
		return;
	}

	// 2. ���� ����

	for (auto& iter : m_pBinary->m_BinaryVector->AnimName.AnimName)
	{
		Safe_Delete(iter);
	}

	for (auto& iter : m_pBinary->m_BinaryVector->ChannelName.ChannelName)
	{
		Safe_Delete(iter);
	}

	m_pBinary->m_BinaryVector->AnimName.AnimName.clear();
	m_pBinary->m_BinaryVector->Durations.Durations.clear();
	m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds.clear();
	m_pBinary->m_BinaryVector->TimeLimits.TimeLimits.clear();
	m_pBinary->m_BinaryVector->ChannelName.ChannelName.clear();
	m_pBinary->m_BinaryVector->NumChannels.NumChannels.clear();
	m_pBinary->m_BinaryVector->KeyFrames.KeyFrames.clear();
	m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames.clear();
	m_pBinary->m_BinaryData->NumAnimations.iNumAnimations = m_iNumAnimations;

	for (int i = 0; i < m_Animations.size(); ++i)
	{
		char* temp = new char[256];
		for (int j = 0; j < 256; ++j)
			temp[j] = m_Animations[i]->Get_Name()[j];
		m_pBinary->m_BinaryVector->AnimName.AnimName.push_back(temp);
		m_pBinary->m_BinaryVector->Durations.Durations.push_back(m_Animations[i]->GetDuration());
		m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds.push_back(m_Animations[i]->GetTickPerSecond());
		m_pBinary->m_BinaryVector->TimeLimits.TimeLimits.push_back(m_Animations[i]->GetTimeLimit());
		m_pBinary->m_BinaryVector->NumChannels.NumChannels.push_back(m_Animations[i]->Get_ChannelSize());
		for (int k = 0; k < m_Animations[i]->Get_ChannelSize(); ++k)
		{
			char* temp2 = new char[256];
			for (int h = 0; h < 256; ++h)
				temp2[h] = m_Animations[i]->GetChannel(k)->Get_Name()[h];
			m_pBinary->m_BinaryVector->ChannelName.ChannelName.push_back(temp2);
			m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames.push_back(m_Animations[i]->GetChannel(k)->Get_KeyFrameSize());
			for (int m = 0; m < m_Animations[i]->GetChannel(k)->Get_KeyFrameSize(); ++m)
			{
				m_pBinary->m_BinaryVector->KeyFrames.KeyFrames.push_back(m_Animations[i]->GetChannel(k)->GetKeyFrame(m));
			}
		}

	}

	DWORD		dwByte = 0;

	WriteFile(hFile, m_pBinary->m_BinaryData, sizeof(BINARYDATA), &dwByte, nullptr);

	int HierarchyTransformSize = m_pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform.size();
	WriteFile(hFile, &HierarchyTransformSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < HierarchyTransformSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform[i], sizeof(_float4x4), &dwByte, nullptr);
	}

	int TexPathSize = m_pBinary->m_BinaryVector->TexturePath.TexturePath.size();
	WriteFile(hFile, &TexPathSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TexPathSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->TexturePath.TexturePath[i], sizeof(_tchar) * 256, &dwByte, nullptr);
	}

	int NumChildSize = m_pBinary->m_BinaryVector->NumChildRen.iNumChildren.size();
	WriteFile(hFile, &NumChildSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NumChildSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumChildRen.iNumChildren[i], sizeof(int), &dwByte, nullptr);
	}

	int HierarchyNameSize = m_pBinary->m_BinaryVector->HierarchyName.HierarchyName.size();
	WriteFile(hFile, &HierarchyNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < HierarchyNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->HierarchyName.HierarchyName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int MeshNameSize = m_pBinary->m_BinaryVector->MeshName.MeshName.size();
	WriteFile(hFile, &MeshNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MeshNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->MeshName.MeshName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int MaterialIndexSize = m_pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex.size();
	WriteFile(hFile, &MaterialIndexSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MaterialIndexSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex[i], sizeof(int), &dwByte, nullptr);
	}

	int PrimitiveSize = m_pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives.size();
	WriteFile(hFile, &PrimitiveSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < PrimitiveSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives[i], sizeof(int), &dwByte, nullptr);
	}

	int IndicesSize = m_pBinary->m_BinaryVector->pIndices.pIndices.size();
	WriteFile(hFile, &IndicesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < IndicesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->pIndices.pIndices[i], sizeof(FACEINDICES32), &dwByte, nullptr);
	}

	int OffSetMaxtrixSize = m_pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix.size();
	WriteFile(hFile, &OffSetMaxtrixSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < OffSetMaxtrixSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix[i], sizeof(_float4x4), &dwByte, nullptr);
	}

	int VerticesSize = m_pBinary->m_BinaryVector->NumVertices.NumVertices.size();
	WriteFile(hFile, &VerticesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumVertices.NumVertices[i], sizeof(int), &dwByte, nullptr);
	}

	int VerticesInfoSize = m_pBinary->m_BinaryVector->VerticesInfo.VerticesInfo.size();
	WriteFile(hFile, &VerticesInfoSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesInfoSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->VerticesInfo.VerticesInfo[i], sizeof(VTXANIMMODEL), &dwByte, nullptr);
	}

	int VerticesIndexSize = m_pBinary->m_BinaryVector->VertexIndex.VertexIndex.size();
	WriteFile(hFile, &VerticesIndexSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesIndexSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->VertexIndex.VertexIndex[i], sizeof(int), &dwByte, nullptr);
	}

	int BlendWeightSize = m_pBinary->m_BinaryVector->BlendWeights.BlendWeights.size();
	WriteFile(hFile, &BlendWeightSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < BlendWeightSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->BlendWeights.BlendWeights[i], sizeof(float), &dwByte, nullptr);
	}

	int DurationSize = m_pBinary->m_BinaryVector->Durations.Durations.size();
	WriteFile(hFile, &DurationSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < DurationSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->Durations.Durations[i], sizeof(_float), &dwByte, nullptr);
	}

	int TickPerSecondSize = m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds.size();
	WriteFile(hFile, &TickPerSecondSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TickPerSecondSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds[i], sizeof(_float), &dwByte, nullptr);
	}

	int ChannelSize = m_pBinary->m_BinaryVector->NumChannels.NumChannels.size();
	WriteFile(hFile, &ChannelSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ChannelSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumChannels.NumChannels[i], sizeof(int), &dwByte, nullptr);
	}

	int ChannelNameSize = m_pBinary->m_BinaryVector->ChannelName.ChannelName.size();
	WriteFile(hFile, &ChannelNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ChannelNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->ChannelName.ChannelName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int NumKeyFramesSize = m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames.size();
	WriteFile(hFile, &NumKeyFramesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NumKeyFramesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames[i], sizeof(int), &dwByte, nullptr);
	}

	int KeyFrameSize = m_pBinary->m_BinaryVector->KeyFrames.KeyFrames.size();
	WriteFile(hFile, &KeyFrameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < KeyFrameSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->KeyFrames.KeyFrames[i], sizeof(KEYFRAME), &dwByte, nullptr);
	}

	int BonesSize = m_pBinary->m_BinaryVector->NumBones.iNumBones.size();
	WriteFile(hFile, &BonesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < BonesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumBones.iNumBones[i], sizeof(int), &dwByte, nullptr);
	}

	int WeightSize = m_pBinary->m_BinaryVector->NumWeights.iNumWeights.size();
	WriteFile(hFile, &WeightSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < WeightSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumWeights.iNumWeights[i], sizeof(int), &dwByte, nullptr);
	}

	int MeshBoneNameSize = m_pBinary->m_BinaryVector->MeshBoneName.MeshBoneName.size();
	WriteFile(hFile, &MeshBoneNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MeshBoneNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->MeshBoneName.MeshBoneName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int AnimNameSize = m_pBinary->m_BinaryVector->AnimName.AnimName.size();
	WriteFile(hFile, &AnimNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < AnimNameSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->AnimName.AnimName[i], sizeof(char) * 256, &dwByte, nullptr);
	}

	int TimeLimitSize = m_pBinary->m_BinaryVector->TimeLimits.TimeLimits.size();
	WriteFile(hFile, &TimeLimitSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TimeLimitSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->TimeLimits.TimeLimits[i], sizeof(_float), &dwByte, nullptr);
	}

	CloseHandle(hFile);

	MSG_BOX(TEXT("SuccesSave"));
}

HRESULT CAnimModel::LoadBinary(const char * pLoadName)
{
	string FileSave = pLoadName;

	string temp = "../Data/AnimModelBinary/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// ���� ��ο� �̸� ���
		GENERIC_READ,				// ���� ���� ��� (GENERIC_WRITE ���� ����, GENERIC_READ �б� ����)
		NULL,						// �������, ������ �����ִ� ���¿��� �ٸ� ���μ����� ������ �� ����� ���ΰ�, NULL�� ��� �������� �ʴ´�
		NULL,						// ���� �Ӽ�, �⺻��	
		OPEN_EXISTING,				// ���� ���, CREATE_ALWAYS�� ������ ���ٸ� ����, �ִٸ� ���� ����, OPEN_EXISTING ������ ���� ��쿡�� ����
		FILE_ATTRIBUTE_NORMAL,		// ���� �Ӽ�(�б� ����, ���� ��), FILE_ATTRIBUTE_NORMAL �ƹ��� �Ӽ��� ���� �Ϲ� ���� ����
		NULL);						// �������� ������ �Ӽ��� ������ ���ø� ����, �츮�� ������� �ʾƼ� NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// �˾� â�� ������ִ� ����� �Լ�
		// 1. �ڵ� 2. �˾� â�� �������ϴ� �޽��� 3. �˾� â �̸� 4. ��ư �Ӽ�
		MSG_BOX(TEXT("Don't Have Dat"));
		return E_FAIL;
	}

	DWORD		dwByte = 0;

	ReadFile(hFile, m_pBinary->m_BinaryData, sizeof(BINARYDATA), &dwByte, nullptr);

	int HierarchyTransformSize;
	ReadFile(hFile, &HierarchyTransformSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < HierarchyTransformSize; ++i)
	{
		_float4x4 HierarchyTransform;
		ReadFile(hFile, &HierarchyTransform, sizeof(_float4x4), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->HierarchyTransform.HierarchyTransform.push_back(HierarchyTransform);
	}

	int TexPathSize;
	ReadFile(hFile, &TexPathSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TexPathSize; ++i)
	{
		_tchar TexPath[256];
		ReadFile(hFile, &TexPath, sizeof(_tchar) * 256, &dwByte, nullptr);

		_tchar* temp = new _tchar[256];
		for (int i = 0; i < 256; ++i)
			temp[i] = TexPath[i];

		m_pBinary->m_BinaryVector->TexturePath.TexturePath.push_back(temp);
	}

	int NumChildSize;
	ReadFile(hFile, &NumChildSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NumChildSize; ++i)
	{
		int NumChildren;
		ReadFile(hFile, &NumChildren, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumChildRen.iNumChildren.push_back(NumChildren);
	}

	int HierarchyNameSize;
	ReadFile(hFile, &HierarchyNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < HierarchyNameSize; ++i)
	{
		char HierarchyName[256];
		ReadFile(hFile, &HierarchyName, sizeof(char) * 256, &dwByte, nullptr);

		char* temp = new char[256];
		for (int i = 0; i < 256; ++i)
			temp[i] = HierarchyName[i];

		m_pBinary->m_BinaryVector->HierarchyName.HierarchyName.push_back(temp);
	}

	int MeshNameSize;
	ReadFile(hFile, &MeshNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MeshNameSize; ++i)
	{
		char MeshName[256];
		ReadFile(hFile, &MeshName, sizeof(char) * 256, &dwByte, nullptr);

		char* temp = new char[256];
		for (int i = 0; i < 256; ++i)
			temp[i] = MeshName[i];

		m_pBinary->m_BinaryVector->MeshName.MeshName.push_back(temp);
	}

	int MaterialIndexSize;
	ReadFile(hFile, &MaterialIndexSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MaterialIndexSize; ++i)
	{
		int MaterialIndex;
		ReadFile(hFile, &MaterialIndex, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->MaterialIndex.iMaterialIndex.push_back(MaterialIndex);
	}

	int PrimitiveSize;
	ReadFile(hFile, &PrimitiveSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < PrimitiveSize; ++i)
	{
		int Primitive;
		ReadFile(hFile, &Primitive, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumPrimitives.iNumPrimitives.push_back(Primitive);
	}

	int IndicesSize;
	ReadFile(hFile, &IndicesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < IndicesSize; ++i)
	{
		FACEINDICES32 Indices;
		ReadFile(hFile, &Indices, sizeof(FACEINDICES32), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->pIndices.pIndices.push_back(Indices);
	}

	int OffSetMatrixSize;
	ReadFile(hFile, &OffSetMatrixSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < OffSetMatrixSize; ++i)
	{
		_float4x4 OffSetMatrix;
		ReadFile(hFile, &OffSetMatrix, sizeof(_float4x4), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->OffSetMatrix.OffSetMatrix.push_back(OffSetMatrix);
	}

	int VerticesSize;
	ReadFile(hFile, &VerticesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesSize; ++i)
	{
		int Vertices;
		ReadFile(hFile, &Vertices, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumVertices.NumVertices.push_back(Vertices);
	}

	int VertexInfoSize;
	ReadFile(hFile, &VertexInfoSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VertexInfoSize; ++i)
	{
		VTXANIMMODEL VertexInfo;
		ReadFile(hFile, &VertexInfo, sizeof(VTXANIMMODEL), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->VerticesInfo.VerticesInfo.push_back(VertexInfo);
	}

	int VertexIndexSize;
	ReadFile(hFile, &VertexIndexSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VertexIndexSize; ++i)
	{
		int VertexIndex;
		ReadFile(hFile, &VertexIndex, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->VertexIndex.VertexIndex.push_back(VertexIndex);
	}

	int BlendWeightSize;
	ReadFile(hFile, &BlendWeightSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < BlendWeightSize; ++i)
	{
		float BlendWeight;
		ReadFile(hFile, &BlendWeight, sizeof(float), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->BlendWeights.BlendWeights.push_back(BlendWeight);
	}

	int DurationSize;
	ReadFile(hFile, &DurationSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < DurationSize; ++i)
	{
		_float Duration;
		ReadFile(hFile, &Duration, sizeof(_float), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->Durations.Durations.push_back(Duration);
	}

	int TickPerSecondSize;
	ReadFile(hFile, &TickPerSecondSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TickPerSecondSize; ++i)
	{
		_float TickPerSecond;
		ReadFile(hFile, &TickPerSecond, sizeof(_float), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds.push_back(TickPerSecond);
	}

	int ChannelSize;
	ReadFile(hFile, &ChannelSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ChannelSize; ++i)
	{
		int NumChannel;
		ReadFile(hFile, &NumChannel, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumChannels.NumChannels.push_back(NumChannel);
	}

	int ChannelNameSize;
	ReadFile(hFile, &ChannelNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < ChannelNameSize; ++i)
	{
		char ChannelName[256];
		ReadFile(hFile, &ChannelName, sizeof(char) * 256, &dwByte, nullptr);

		char* temp = new char[256];
		for (int i = 0; i < 256; ++i)
			temp[i] = ChannelName[i];

		m_pBinary->m_BinaryVector->ChannelName.ChannelName.push_back(temp);
	}

	int KeyFramesSize;
	ReadFile(hFile, &KeyFramesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < KeyFramesSize; ++i)
	{
		int NumKeyFrame;
		ReadFile(hFile, &NumKeyFrame, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumKeyFrames.NumKeyFrames.push_back(NumKeyFrame);
	}

	int KeyFrameSize;
	ReadFile(hFile, &KeyFrameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < KeyFrameSize; ++i)
	{
		KEYFRAME KeyFrame;
		ReadFile(hFile, &KeyFrame, sizeof(KEYFRAME), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->KeyFrames.KeyFrames.push_back(KeyFrame);
	}

	int BoneSize;
	ReadFile(hFile, &BoneSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < BoneSize; ++i)
	{
		int Bones;
		ReadFile(hFile, &Bones, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumBones.iNumBones.push_back(Bones);
	}

	int WeightSize;
	ReadFile(hFile, &WeightSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < WeightSize; ++i)
	{
		int Weights;
		ReadFile(hFile, &Weights, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumWeights.iNumWeights.push_back(Weights);
	}

	int MeshBoneNameSize;
	ReadFile(hFile, &MeshBoneNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < MeshBoneNameSize; ++i)
	{
		char MeshBoneName[256];
		ReadFile(hFile, &MeshBoneName, sizeof(char) * 256, &dwByte, nullptr);

		char* temp = new char[256];
		for (int i = 0; i < 256; ++i)
			temp[i] = MeshBoneName[i];

		m_pBinary->m_BinaryVector->MeshBoneName.MeshBoneName.push_back(temp);
	}

	int AnimNameSize;
	ReadFile(hFile, &AnimNameSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < AnimNameSize; ++i)
	{
		char AnimName[256];
		ReadFile(hFile, &AnimName, sizeof(char) * 256, &dwByte, nullptr);

		char* temp = new char[256];
		for (int i = 0; i < 256; ++i)
			temp[i] = AnimName[i];

		m_pBinary->m_BinaryVector->AnimName.AnimName.push_back(temp);
	}

	int TimeLimitSize;
	ReadFile(hFile, &TimeLimitSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TimeLimitSize; ++i)
	{
		_float TimeLimit;
		ReadFile(hFile, &TimeLimit, sizeof(_float), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->TimeLimits.TimeLimits.push_back(TimeLimit);
	}

	CloseHandle(hFile);
	return S_OK;
}


CAnimModel * CAnimModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pModelFilePath, const char * pModelFileName, const char* pSaveName, _fmatrix PivotMatrix)
{
	CAnimModel*			pInstance = new CAnimModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pModelFilePath, pModelFileName,pSaveName, PivotMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimModel * CAnimModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pLoadName, _fmatrix PivotMatrix)
{
	CAnimModel*			pInstance = new CAnimModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pLoadName, PivotMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CAnimModel::Clone(void * pArg)
{
	CAnimModel*			pInstance = new CAnimModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CAnimModel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimModel::Free()
{
	__super::Free();

	for (auto& pHierarchyNode : m_HierarchyNodes)
		Safe_Release(pHierarchyNode);

	m_HierarchyNodes.clear();

	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
			Safe_Release(Material.pTexture[i]);
	}
	m_Materials.clear();

	for (auto& pMeshContainer : m_Meshes)
		Safe_Release(pMeshContainer);

	m_Meshes.clear();

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);

	m_Animations.clear();

	Safe_Release(m_pBinary);

	m_Importer.FreeScene();
}

