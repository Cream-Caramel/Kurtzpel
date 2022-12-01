#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CLogoScene final : public CUI
{
private:
	CLogoScene(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CLogoScene(const CLogoScene& rhs);
	virtual ~CLogoScene() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Components();
	_matrix Identity;
	_matrix Proj;
public:
	static CLogoScene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

