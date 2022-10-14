#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CBossHPBarFrame final : public CUI
{
private:
	CBossHPBarFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CBossHPBarFrame(const CBossHPBarFrame& rhs);
	virtual ~CBossHPBarFrame() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool m_bDie = false;
	_float m_fDieAcc = 0.f;
	_float m_fNowBossHp;
	_float m_fPreBossHp;
	_float m_fShaderNowHp;
	_float m_fShaderPreHp;
	

public:
	static CBossHPBarFrame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

