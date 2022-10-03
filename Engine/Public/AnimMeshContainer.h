#pragma once


#include "VIBuffer.h"
#include "Model.h"
#include "Engine_Binary.h"
#include "Binary.h"


/* ���� �����ϴ� �ϳ��� �޽�. */
/* �� �޽ø� �����ϴ� ����, �ε��� ���۸� �����Ѵ�. */

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
	/* �� �޽ÿ� ���⤷�� �ִ� ���� ����. */
	_uint							m_iNumBones = 0;

	/* �� �޽ÿ� ������ �ִ� ������ ��Ƴ�����. */
	/* why ? : �޽� ���� �������� �� �� �޽ÿ� ���⤷�� �ִ� ������ ����� ��Ƽ� ��{���𷯤� ���ä�������. */
	vector<class CHierarchyNode*>	m_Bones;

private:
	HRESULT Ready_AnimVertices(CAnimModel* pModel, CBinary* pBinary);
public:
	static CAnimMeshContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, class CAnimModel* pModel, _fmatrix PivotMatrix, CBinary* pBinary);
	virtual CComponent* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

END