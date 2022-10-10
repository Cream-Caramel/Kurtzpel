#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CMPBarFrame final : public CUI
{
private:
	CMPBarFrame(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CMPBarFrame(const CMPBarFrame& rhs);
	virtual ~CMPBarFrame() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CMPBarFrame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

