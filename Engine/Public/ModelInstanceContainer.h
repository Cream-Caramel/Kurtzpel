#pragma once


#include "VIBuffer.h"
#include "Binary.h"


BEGIN(Engine)

class CModelInstanceContainer final : public CVIBuffer
{
private:
	CModelInstanceContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModelInstanceContainer(const CModelInstanceContainer& rhs);
	virtual ~CModelInstanceContainer() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(_fmatrix PivotMatrix, CBinary* pBinary, vector<_float3*> MatrixInfo);
	virtual HRESULT Initialize(void* pArg);

public:
	virtual HRESULT Render();

private:
	_uint				m_iMaterialIndex = 0;
	_uint				m_iInstanceStride = 0;
	_uint				m_iNumInstance = 0;
	ID3D11Buffer*			m_pVBInstance = nullptr;

public:
	static CModelInstanceContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PivotMatrix, CBinary* pBinary, vector<_float3*> MatrixInfo);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
