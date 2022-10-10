#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CKeyShift final : public CUI
{
private:
	CKeyShift(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CKeyShift(const CKeyShift& rhs);
	virtual ~CKeyShift() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CKeyShift* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

