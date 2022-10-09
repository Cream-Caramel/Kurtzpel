#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CSkillFrame final : public CUI
{
private:
	CSkillFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CSkillFrame(const CSkillFrame& rhs);
	virtual ~CSkillFrame() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	_float m_fCoolTime;
	_float m_fMaxCoolTime;
	bool m_bCoolTime;
	bool m_bUse;

public:
	static CSkillFrame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

