#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CPlayerEx final : public CMesh
{

private:
	CPlayerEx(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerEx(const CPlayerEx& rhs);
	virtual ~CPlayerEx() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	void StartDissolve();
	void EndDissolve();


private:
	CModel* m_pModel;
	CTexture* m_pDissolveTexture = nullptr;
	_bool m_bStartDissolve = false;
	_bool m_bEndDissolve = false;
	_float m_fDissolveAcc = 0.f;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END