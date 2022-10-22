#include "..\Public\ModelsInstance.h"
#include "ModelInstanceContainer.h"
#include "Texture.h"

CModelsInstance::CModelsInstance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{
}

CModelsInstance::CModelsInstance(const CModelsInstance & rhs)
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
		for (_uint i = 0; i < TEX_END; ++i)
			Safe_AddRef(Material.pTexture[i]);
	}

	Safe_AddRef(m_pBinary);
}


HRESULT CModelsInstance::Initialize_Prototype(const char * pLoadName, const char* pSavePath, vector<_float3*> Matrix, _fmatrix PivotMatrix)
{
	m_pBinary = new CBinary;
	if (FAILED(LoadBinary(pLoadName, pSavePath)))
	{
		MSG_BOX(TEXT("Fail To Load"));
		return E_FAIL;
	}

	XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix);

	/* 모델을 구성하는 메시들을 만든다. */
	if (FAILED(Ready_MeshContainers(Matrix, PivotMatrix)))
		return E_FAIL;

	if (FAILED(Ready_Materials()))
		return E_FAIL;

	return S_OK;
}

HRESULT CModelsInstance::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CModelsInstance::SetUp_OnShader(CShader * pShader, _uint iMaterialIndex, TEXTURETYPE eTextureType, const char * pConstantName)
{
	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;
	if (m_Materials[iMaterialIndex].pTexture[eTextureType] == nullptr)
		return E_FAIL;

	return m_Materials[iMaterialIndex].pTexture[eTextureType]->Set_SRV(pShader, pConstantName);
}

HRESULT CModelsInstance::Render(_uint iMeshIndex)
{

	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}


HRESULT CModelsInstance::LoadBinary(const char * FileName, const char* pSavePath)
{
	string FileSave = FileName;
	string SavePath = pSavePath;
	string temp = "../Data/ModelBinary/";

	FileSave = temp + SavePath + "/" + FileSave + ".dat";

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

HRESULT CModelsInstance::Ready_MeshContainers(vector<_float3*> Matrix, _fmatrix PivotMatrix)
{
	m_iNumMeshes = m_pBinary->m_BinaryData->NumMeshes.iNumMeshes;

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		CModelInstanceContainer*		pModelInstanceContainer = CModelInstanceContainer::Create(m_pDevice, m_pContext, PivotMatrix, m_pBinary, Matrix);
		if (nullptr == pModelInstanceContainer)
			return E_FAIL;

		m_Meshes.push_back(pModelInstanceContainer);
	}

	return S_OK;
}

HRESULT CModelsInstance::Ready_Materials()
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

CModelsInstance * CModelsInstance::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pLoadName, const char* pSavePath, vector<_float3*> Matrix, _fmatrix PivotMatrix)
{
	CModelsInstance*			pInstance = new CModelsInstance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pLoadName, pSavePath, Matrix, PivotMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CModelsInstance::Clone(void * pArg)
{
	CModelsInstance*			pInstance = new CModelsInstance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CModelsInstance"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModelsInstance::Free()
{
	__super::Free();

	for (auto& Material : m_Materials)
	{
		for (_uint i = 0; i < TEX_END; ++i)
			Safe_Release(Material.pTexture[i]);
	}
	m_Materials.clear();

	for (auto& pMeshContainer : m_Meshes)
		Safe_Release(pMeshContainer);

	m_Meshes.clear();

	Safe_Release(m_pBinary);
}
