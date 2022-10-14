#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CBossMPBar final : public CUI
{
private:
	CBossMPBar(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CBossMPBar(const CBossMPBar& rhs);
	virtual ~CBossMPBar() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fBossMp;
	_float m_fShaderMp;
	_bool m_bDie = false;
	_float m_fDieAcc = 0.f;
public:
	static CBossMPBar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

