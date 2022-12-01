#pragma once
#include "Client_Defines.h"
#include "UI.h"

BEGIN(Client)

class CFadeInOut final : public CUI
{
public:
	enum FADE {FADEIN, FADEOUT, FADEEND};
private:
	CFadeInOut(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CFadeInOut(const CFadeInOut& rhs);
	virtual ~CFadeInOut() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void On_Fade() { m_bFade = true; }
	void Set_Fade(FADE eFade) { m_eFadeIn = eFade; }

private:
	_bool m_bFade = false;
	FADE m_eFadeIn;
	_float m_fFadeAcc = 1.f;

private:
	HRESULT Ready_Components();
	_matrix Identity;
	_matrix Proj;
public:
	static CFadeInOut* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

