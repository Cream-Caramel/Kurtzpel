#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CPressKey final : public CUI
{
private:
	CPressKey(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPressKey(const CPressKey& rhs);
	virtual ~CPressKey() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fPressAcc = 0.f;
	_bool m_bUp = true;
	HRESULT Ready_Components();
	_matrix Identity;
	_matrix Proj;


public:
	static CPressKey* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

