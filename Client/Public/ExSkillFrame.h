#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CExSkillFrame final : public CUI
{
private:
	CExSkillFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CExSkillFrame(const CExSkillFrame& rhs);
	virtual ~CExSkillFrame() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CExSkillFrame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

