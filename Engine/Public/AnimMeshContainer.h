#pragma once


#include "VIBuffer.h"
#include "Model.h"
#include "Engine_Binary.h"
#include "Binary.h"


/* 모델을 구성하는 하나의 메시. */
/* 이 메시를 구성하는 정점, 인덱스 버퍼를 보관한다. */

BEGIN(Engine)

class CAnimMeshContainer final : public CVIBuffer
{
private:
	CAnimMeshContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimMeshContainer(const CAnimMeshContainer& rhs);
	virtual ~CAnimMeshContainer() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(class CAnimModel* pModel, _fmatrix PivotMatrix, CBinary* pBinary);
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT SetUp_HierarchyNodes(class CAnimModel* pModel, CBinary* pBinary);
	void SetUp_BoneMatrices(_float4x4* pBoneMatrices, _fmatrix PivotMatrix);
	_uint GetBones() { return m_iNumBones; }

private:
	char				m_szName[256] = "";
	_uint				m_iMaterialIndex = 0;
private:
	/* 이 메시에 영향ㅇ르 주는 뼈의 갯수. */
	_uint							m_iNumBones = 0;

	/* 이 메시에 영향을 주는 뼈들을 모아놓느다. */
	/* why ? : 메시 별로 렌더링할 때 이 메시에 영향ㅇ르 주는 뼏르의 행렬을 모아서 셰{ㅇ디러ㅗ 도ㅓㄴ질려고. */
	vector<class CHierarchyNode*>	m_Bones;

private:
	HRESULT Ready_AnimVertices(CAnimModel* pModel, CBinary* pBinary);
public:
	static CAnimMeshContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, class CAnimModel* pModel, _fmatrix PivotMatrix, CBinary* pBinary);
	virtual CComponent* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

END