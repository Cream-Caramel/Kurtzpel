#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CCount final : public CUI
{
private:
	CCount(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CCount(const CCount& rhs);
	virtual ~CCount() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	

public:
	_uint Get_Count();
	void Set_Count(_uint iCount);

private:
	_uint m_iTextureNum;


public:
	static CCount* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

