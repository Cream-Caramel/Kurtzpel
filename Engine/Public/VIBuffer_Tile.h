#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Tile final : public CVIBuffer
{
private:
	CVIBuffer_Tile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Tile(const CVIBuffer_Tile& rhs);
	virtual ~CVIBuffer_Tile() = default;

public:
	typedef struct tagVtxIndex
	{
		int Index[3]{ 0 };
	}VTXINDEX;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	HRESULT Render();

	HRESULT CreateTile();
	HRESULT Map(D3D11_MAPPED_SUBRESOURCE* pSubResource);
	HRESULT UnMap();
	_float3* GetVtxPos() { return m_pVerticesPos; }
	int GetVtxSize() {
		int size = m_iNumVerticesX
			* m_iNumVerticesZ
			;
		return size;
	}

	int GetVtxX() { return m_iNumVerticesX; }
	int GetVtxZ() { return m_iNumVerticesZ; }


	virtual _bool Picking(class CTransform* pTransform, _float3* pOut, VTXINDEX& Vtxpos);


	void SetVtxNum(int VtxX, int VtxZ) { m_iNumVerticesX = VtxX; m_iNumVerticesZ = VtxZ; }

	_uint m_iNumVerticesX = 2;
	_uint m_iNumVerticesZ = 2;





public:
	static CVIBuffer_Tile* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg);
	virtual void Free();
};

END