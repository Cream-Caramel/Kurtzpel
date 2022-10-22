#pragma once
#include "Client_Defines.h"
#include "MeshInstance.h"
BEGIN(Client)

class CTRSky final : public CMeshInstance
{

private:
	CTRSky(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CTRSky(const CTRSky& rhs);
	virtual ~CTRSky() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;



private:
	CModelsInstance* m_pModel = nullptr;


public:
	static CMeshInstance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END