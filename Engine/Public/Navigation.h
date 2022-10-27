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
	_bool isMove(_fvector vPosition, _vector* vNormal);
	void Load_Cell(const char* pFilePath);
	_float Get_PosY(_fvector vPos);
	void Set_NaviRender() { m_bNaviRender = !m_bNaviRender; }
	void Set_BattleIndex(_uint iBattleIndex) { m_iBattleIndex = iBattleIndex; }
	int Get_CurrentIndex() { return m_NavigationDesc.iCurrentIndex; }

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;
#endif // _DEBUG

private:
	vector<class CCell*>			m_Cells;
	typedef vector<class CCell*>	CELLS;
	NAVIGATIONDESC					m_NavigationDesc;
	int							m_iBattleIndex = -1;
	_bool m_bNaviRender = false;

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const char* pFilePath);
	CComponent* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

END 