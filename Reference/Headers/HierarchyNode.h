#pragma once

#include "Base.h"
#include "Engine_Binary.h"
#include "Binary.h"
/* 계층구조(뼈끼리의 상속관계)를 표현하기위한 aiNode이다. */

BEGIN(Engine)

class CHierarchyNode final : public CBase
{
private:
	CHierarchyNode();
	virtual ~CHierarchyNode() = default;

public:
	const char* Get_Name() const {
		return m_szName;
	}
	_uint Get_Depth() const {
		return m_iDepth;
	}

	_matrix Get_OffSetMatrix() {
		return XMLoadFloat4x4(&m_OffsetMatrix);
	}

	_matrix Get_CombinedTransformation() {
		return XMLoadFloat4x4(&m_CombinedTransformation);
	}


public:
	void Set_Transformation(_fmatrix Transformation) {
		XMStoreFloat4x4(&m_Transformation, Transformation);
	}

	_float4x4 Get_Transformation() {
		return m_Transformation;
	}

public:
	HRESULT Initialize(class CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary);
	void Set_CombinedTransformation();
	void Set_OffsetMatrix(_fmatrix OffsetMatrix);
	_uint Get_ChildRen() { return m_iChildRen; }
private:
	char				m_szName[256] = "";
	_float4x4			m_OffsetMatrix;
	_float4x4			m_Transformation;
	_float4x4			m_CombinedTransformation;
	CHierarchyNode*		m_pParent = nullptr;
	_uint				m_iDepth = 0;
	_uint				m_iChildRen = 0;

public:
	static CHierarchyNode* Create(class CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary);
	virtual void Free();
};

END