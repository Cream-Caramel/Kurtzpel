#pragma once

#include "Component.h"
#include "MeshInstanceContainer.h"
#include "Binary.h"

BEGIN(Engine)

class ENGINE_DLL CModelInstance final : public CComponent
{
private:
	CModelInstance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModelInstance(const CModelInstance& rhs);
	virtual ~CModelInstance() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	_uint Get_MaterialIndex(_uint iMeshIndex) {
		return m_Meshes[iMeshIndex]->Get_MaterialIndex();
	}

	_matrix Get_PivotMatrix() {
		return XMLoadFloat4x4(&m_PivotMatrix);
	}

	void Set_PivotMatrix(_matrix PivotMatrix) { XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix); }

public:
	virtual HRESULT Initialize_Prototype(const char* pLoadName, const char* pSavePath, vector<_float3*> Matrix, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT SetUp_OnShader(class CShader* pShader, _uint iMaterialIndex, TEXTURETYPE eTextureType, const char* pConstantName);
	HRESULT Render(_uint iMeshIndex);
	HRESULT LoadBinary(const char* FileName, const char* pSavePath);
	
private:
	_float4x4					m_PivotMatrix;

private:
	_uint									m_iNumMeshes = 0;
	vector<class CMeshInstanceContainer*>			m_Meshes;
	typedef vector<class CMeshInstanceContainer*>	MESHES;

private:
	_uint									m_iNumMaterials = 0;
	vector<MATERIALDESC>					m_Materials;
	CBinary* m_pBinary;

private:
	HRESULT Ready_MeshContainers(vector<_float3*> Matrix, _fmatrix PivotMatrix);
	HRESULT Ready_Materials();
	
public:
	static CModelInstance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,  const char* pLoadName, const char* pSavePath, vector<_float3*> Matrix, _fmatrix PivotMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END