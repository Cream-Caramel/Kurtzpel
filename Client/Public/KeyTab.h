#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CKeyTab final : public CUI
{
private:
	CKeyTab(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CKeyTab(const CKeyTab& rhs);
	virtual ~CKeyTab() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Set_Down() { m_bDown = true; }


private:
	_bool m_bDown;
	_float m_fDownAcc;
public:
	static CKeyTab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

