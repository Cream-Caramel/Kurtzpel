#pragma once

#include "Component.h"
#include "MeshContainer.h"
#include "Binary.h"

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	_uint Get_MaterialIndex(_uint iMeshIndex) {
		return m_Meshes[iMeshIndex]->Get_MaterialIndex();
	}
public:
	virtual HRESULT Initialize_Prototype(const char* pModelFilePath, const char* pModelFileName, const char* pSaveName, _fmatrix PivotMatrix);
	virtual HRESULT Initialize_Prototype(const char* pLoadName, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT SetUp_OnShader(class CShader* pShader, _uint iMaterialIndex, aiTextureType eTextureType, const char* pConstantName);
	HRESULT Render(_uint iMeshIndex);
	void SaveBinary(const char* FileName);
	HRESULT LoadBinary(const char* FileName);
private:
	const aiScene*				m_pAIScene = nullptr;
	Assimp::Importer			m_Importer;
	_float4x4					m_PivotMatrix;

private:
	_uint									m_iNumMeshes = 0;
	vector<class CMeshContainer*>			m_Meshes;
	typedef vector<class CMeshContainer*>	MESHES;

private:
	_uint									m_iNumMaterials = 0;
	vector<MATERIALDESC>					m_Materials;
	CBinary* m_pBinary;

private:
	HRESULT Ready_MeshContainers(_fmatrix PivotMatrix, CBinary* pBinary);
	HRESULT Ready_Materials(const char* pModelFilePath, CBinary* pBinary);
	HRESULT Ready_MeshContainers(_fmatrix PivotMatrix);
	HRESULT Ready_Materials();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,  const char* pModelFilePath, const char* pModelFileName, const char* pSaveName, _fmatrix PivotMatrix = XMMatrixIdentity());
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,  const char* pLoadName, _fmatrix PivotMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg = nullptr);
	virtual void Free() override;
};

END