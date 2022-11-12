#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CPlayerLight final : public CMesh
{
public:
	typedef struct tagPlayerLightInfo
	{
		_float3 vScale{ 1.f,1.f, 1.f };
		_float4 vPos;
		_float3 vAngle{ 0.f,0.f,0.f };
		_float fCloseSpeed;

	}PLAYERLIGHT;
private:
	CPlayerLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerLight(const CPlayerLight& rhs);
	virtual ~CPlayerLight() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModel;
	_float m_fCloseSpeed;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END