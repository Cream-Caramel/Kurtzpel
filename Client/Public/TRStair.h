#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CTRStair final : public CMesh
{

private:
	CTRStair(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CTRStair(const CTRStair& rhs);
	virtual ~CTRStair() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;



private:
	CModel* m_pModel;


public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END