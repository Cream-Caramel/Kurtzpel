#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CExGauge final : public CUI
{
private:
	CExGauge(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CExGauge(const CExGauge& rhs);
	virtual ~CExGauge() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_TextureNum(_uint iTextureNum);
	void Reset_TextureNum() { m_iTextureNum = 0; }
	_uint Get_TextureNum() { return m_iTextureNum; }
private:
	_uint m_iTextureNum;
	_float m_fExGaugeAcc;
	_float m_ShaderGauge;
	_bool m_bMaxTexture;
	_bool m_bMaxAcc;

public:
	static CExGauge* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

