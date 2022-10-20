#pragma once

#include "Component.h"
#include "Cell.h"
BEGIN(Engine)
class CSphere;
class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		_int			iCurrentIndex = -1;
	}NAVIGATIONDESC;


private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& rhs);
	virtual ~CNavigation() = default;

public:
	virtual HRESULT Initialize_Prototype(const char* pFilePath);
	virtual HRESULT Initialize(void* pArg);

public:
	_bool isMove(_fvector vPosition);
	void Load_Cell(const char* pFilePath);
	_float Set_PosY(_fvector vPos);


#ifdef _DEBUG
public:
	HRESULT Render();
#endif // _DEBUG

private:
	vector<class CCell*>			m_Cells;
	typedef vector<class CCell*>	CELLS;
	NAVIGATIONDESC					m_NavigationDesc;

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const char* pFilePath);
	CComponent* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

END 