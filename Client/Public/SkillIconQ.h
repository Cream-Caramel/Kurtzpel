#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CSkillIconQ final : public CUI
{
private:
	CSkillIconQ(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CSkillIconQ(const CSkillIconQ& rhs);
	virtual ~CSkillIconQ() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CSkillIconQ* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

