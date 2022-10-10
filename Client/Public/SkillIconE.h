#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CSkillIconE final : public CUI
{
private:
	CSkillIconE(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CSkillIconE(const CSkillIconE& rhs);
	virtual ~CSkillIconE() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CSkillIconE* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

