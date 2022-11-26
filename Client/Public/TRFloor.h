#pragma once
#include "Client_Defines.h"
#include "MeshInstance.h"
BEGIN(Client)

class CTRFloor final : public CMeshInstance
{

private:
	CTRFloor(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CTRFloor(const CTRFloor& rhs);
	virtual ~CTRFloor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_ShadowDepth() override;


private:
	CModelsInstance* m_pModel = nullptr;

public:
	static CMeshInstance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END