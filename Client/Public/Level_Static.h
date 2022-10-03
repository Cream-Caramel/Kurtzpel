#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Static final : public CLevel
{
private:
	CLevel_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Static() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CLevel_Static* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END