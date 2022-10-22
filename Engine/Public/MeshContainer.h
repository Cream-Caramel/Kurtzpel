#pragma once


#include "VIBuffer.h"
#include "Binary.h"

/* ���� �����ϴ� �ϳ��� �޽�. */
/* �� �޽ø� �����ϴ� ����, �ε��� ���۸� �����Ѵ�. */

BEGIN(Engine)

class CMeshContainer final : public CVIBuffer
{
private:
	CMeshContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshContainer(const CMeshContainer& rhs);
	virtual ~CMeshContainer() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(_fmatrix PivotMatrix, CBinary* pBinary);
	virtual HRESULT Initialize(void* pArg);

private:
	_uint				m_iMaterialIndex = 0;

public:
	static CMeshContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PivotMatrix, CBinary* pBinary);
	virtual CComponent* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

END