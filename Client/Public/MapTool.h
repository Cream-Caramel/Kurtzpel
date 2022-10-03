#pragma once
#include "Client_Defines.h"
#include "Base.h"
BEGIN(Client)
class CMapTool final : public CBase
{
public:
	CMapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMapTool() = default;

private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;


public:
	void ShowMapWindow(_float fTimeDelta);
	void AddBaseTile(CBaseTile* pBaseTile);

private:
	float m_fTileColor[4]{ 1,1,1,1 };
	int m_iVtxNumX = 1;
	int m_iVexNumZ = 1;
	CBaseTile* m_pBaseTile = nullptr;

public:
	virtual void Free() override;
};

END

