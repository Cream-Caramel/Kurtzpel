#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CBossBarLine final : public CUI
{
private:
	CBossBarLine(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CBossBarLine(const CBossBarLine& rhs);
	virtual ~CBossBarLine() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	_bool m_bDie = false;
	_float m_fDieAcc = 0.f;

public:
	static CBossBarLine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

