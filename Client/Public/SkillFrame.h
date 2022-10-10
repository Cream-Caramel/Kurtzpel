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

public:
	_bool Get_CoolTime() { return m_bCoolTime; }
	void Set_CoolTime() { m_bCoolTime = true; }
private:
	void Setting();
private:
	int m_iIndex;
	_float m_fCoolTime;
	_float m_fMaxCoolTime;
	_float ShaderCoolTime;
	_float m_fCoolTimeOn;
	_float m_fMaxCoolTimeOn;
	_float ShaderCoolTimeOn;
	bool m_bCoolTime;
	bool m_bCoolTimeOn;
	bool m_bCoolTimeOn2;

public:
	static CSkillFrame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END

