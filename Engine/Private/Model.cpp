#include "..\Public\Model.h"
#include "MeshContainer.h"
#include "Texture.h"

CModel::CModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{
}

CModel::CModel(const CModel & rhs)
	: CComponent(rhs)
	, m_iNumMeshes(rhs.m_iNumMeshes)
	, m_iNumMaterials(rhs.m_iNumMaterials)
	, m_Meshes(rhs.m_Meshes)
	, m_Materials(rhs.m_Materials)
	, m_pBinary(rhs.m_pBinary)
{
	for (auto& pMeshContainer : m_Meshes)
		Safe_AddRef(pMeshContainer);


	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
			Safe_AddRef(Material.pTexture[i]);
	}

	Safe_AddRef(m_pBinary);
}

HRESULT CModel::Initialize_Prototype(const char * pModelFilePath, const char * pModelFileName, const char* pSaveName, _fmatrix PivotMatrix)
{
	m_pBinary = new CBinary;
	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	char		szFullPath[256] = "";

	strcpy_s(szFullPath, pModelFilePath);
	strcat_s(szFullPath, pModelFileName);

	_uint		iFlag = 0;

	iFlag |= aiProcess_GlobalScale | aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace;

	m_pAIScene = m_Importer.ReadFile(szFullPath, iFlag);

	if (nullptr == m_pAIScene)
		return E_FAIL;

	/* 모델을 구성하는 메시들을 만든다. */
	if (FAILED(Ready_MeshContainers(PivotMatrix, m_pBinary)))
		return E_FAIL;

	if (FAILED(Ready_Materials(pModelFilePath, m_pBinary)))
		return E_FAIL;

	SaveBinary(pSaveName);
	return S_OK;
}

HRESULT CModel::Initialize_Prototype(const char * pLoadName, _fmatrix PivotMatrix)
{
	m_pBinary = new CBinary;
	if (FAILED(LoadBinary(pLoadName)))
	{
		MSG_BOX(TEXT("Fail To Load"));
		return E_FAIL;
	}

	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	_uint		iFlag = 0;

	iFlag |= aiProcess_GlobalScale | aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace;

	/* 모델을 구성하는 메시들을 만든다. */
	if (FAILED(Ready_MeshContainers(PivotMatrix)))
		return E_FAIL;

	if (FAILED(Ready_Materials()))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CModel::SetUp_OnShader(CShader * pShader, _uint iMaterialIndex, aiTextureType eTextureType, const char * pConstantName)
{
	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;
	if (m_Materials[iMaterialIndex].pTexture[eTextureType] == nullptr)
		return E_FAIL;

	return m_Materials[iMaterialIndex].pTexture[eTextureType]->Set_SRV(pShader, pConstantName);
}

HRESULT CModel::Render(_uint iMeshIndex)
{

	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

void CModel::SaveBinary(const char * FileName)
{
	string FileSave = FileName;

	string temp = "../Data/ModelBinary/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hReadFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE != hReadFile)
	{
		return;
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_WRITE,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		CREATE_ALWAYS,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MSG_BOX(TEXT("Fail To Binary"));
		return;
	}

	// 2. 파일 쓰기

	DWORD		dwByte = 0;

	WriteFile(hFile, m_pBinary->m_BinaryData, sizeof(BINARYDATA), &dwByte, nullptr);

	int TexPathSize = m_pBinary->m_BinaryVector->TexturePath.TexturePath.size();
	WriteFile(hFile, &TexPathSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < TexPathSize; ++i)
	{
		WriteFile(hFile, m_pBinary->m_BinaryVector->TexturePath.TexturePath[i], sizeof(_tchar) * 256, &dwByte, nullptr);
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

	int VerticesSize = m_pBinary->m_BinaryVector->NumVertices.NumVertices.size();
	WriteFile(hFile, &VerticesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NumVertices.NumVertices[i], sizeof(int), &dwByte, nullptr);
	}

	int NonAniVerticesInfoSize = m_pBinary->m_BinaryVector->NonAniVertexInfo.NonAniVerticesInfo.size();
	WriteFile(hFile, &NonAniVerticesInfoSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NonAniVerticesInfoSize; ++i)
	{
		WriteFile(hFile, &m_pBinary->m_BinaryVector->NonAniVertexInfo.NonAniVerticesInfo[i], sizeof(VTXMODEL), &dwByte, nullptr);
	}

	// 3. 파일 소멸
	CloseHandle(hFile);
}

HRESULT CModel::LoadBinary(const char * FileName)
{
	string FileSave = FileName;

	string temp = "../Data/ModelBinary/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MSG_BOX(TEXT("Don't Have Dat"));
		return E_FAIL;
	}

	DWORD		dwByte = 0;

	ReadFile(hFile, m_pBinary->m_BinaryData, sizeof(BINARYDATA), &dwByte, nullptr);

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

	int VerticesSize;
	ReadFile(hFile, &VerticesSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < VerticesSize; ++i)
	{
		int Vertices;
		ReadFile(hFile, &Vertices, sizeof(int), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NumVertices.NumVertices.push_back(Vertices);
	}

	int NonVertexInfoSize;
	ReadFile(hFile, &NonVertexInfoSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NonVertexInfoSize; ++i)
	{
		VTXMODEL VertexInfo;
		ReadFile(hFile, &VertexInfo, sizeof(VTXMODEL), &dwByte, nullptr);
		m_pBinary->m_BinaryVector->NonAniVertexInfo.NonAniVerticesInfo.push_back(VertexInfo);
	}

	CloseHandle(hFile);	
	return S_OK;
	
}

HRESULT CModel::Ready_MeshContainers(_fmatrix PivotMatrix, CBinary* pBinary)
{
	m_iNumMeshes = m_pAIScene->mNumMeshes;
	pBinary->m_BinaryData->NumMeshes.iNumMeshes = m_iNumMeshes;

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		CMeshContainer*		pMeshContainer = CMeshContainer::Create(m_pDevice, m_pContext, m_pAIScene->mMeshes[i], PivotMatrix, pBinary);
		if (nullptr == pMeshContainer)
			return E_FAIL;

		m_Meshes.push_back(pMeshContainer);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const char* pModelFilePath, CBinary* pBinary)
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

HRESULT CModel::Ready_MeshContainers(_fmatrix PivotMatrix)
{
	m_iNumMeshes = m_pBinary->m_BinaryData->NumMeshes.iNumMeshes;

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		CMeshContainer*		pMeshContainer = CMeshContainer::Create(m_pDevice, m_pContext, PivotMatrix, m_pBinary);
		if (nullptr == pMeshContainer)
			return E_FAIL;

		m_Meshes.push_back(pMeshContainer);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials()
{
	m_iNumMaterials = m_pBinary->m_BinaryData->NumMaterials.iNumMaterials;

	for (_uint i = 0; i < m_iNumMaterials; ++i)
	{
		MATERIALDESC		MaterialDesc;
		ZeroMemory(&MaterialDesc, sizeof(MATERIALDESC));


		for (_uint j = 0; j < 18; ++j)
		{
			if (!wcscmp(m_pBinary->m_BinaryVector->TexturePath.TexturePath[m_pBinary->TexturePathIndex], L""))
			{
				m_pBinary->TexturePathIndex++;
				continue;
			}

			MaterialDesc.pTexture[j] = CTexture::Create(m_pDevice, m_pContext, m_pBinary->m_BinaryVector->TexturePath.TexturePath[m_pBinary->TexturePathIndex++]);
			if (nullptr == MaterialDesc.pTexture[j])
				return E_FAIL;
		}

		m_Materials.push_back(MaterialDesc);
	}

	return S_OK;
}

CModel * CModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pModelFilePath, const char * pModelFileName, const char* pSaveName, _fmatrix PivotMatrix)
{
	CModel*			pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pModelFilePath, pModelFileName, pSaveName, PivotMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CModel * CModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pLoadName, _fmatrix PivotMatrix)
{
	CModel*			pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pLoadName, PivotMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CModel::Clone(void * pArg)
{
	CModel*			pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CModel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
			Safe_Release(Material.pTexture[i]);
	}
	m_Materials.clear();

	for (auto& pMeshContainer : m_Meshes)
		Safe_Release(pMeshContainer);

	m_Meshes.clear();

	Safe_Release(m_pBinary);

	m_Importer.FreeScene();
}
