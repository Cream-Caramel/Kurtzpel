#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CBarLine final : public CUI
{
private:
	CBarLine(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CBarLine(const CBarLine& rhs);
	virtual ~CBarLine() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CBarLine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

