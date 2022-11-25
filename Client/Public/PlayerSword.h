#pragma once
#include "Client_Defines.h"
#include "Mesh.h"

BEGIN(Engine)
class COBB;
END

BEGIN(Client)
class CTrail;
class CPlayer;
class CPlayerSword final : public CMesh
{

private:
	CPlayerSword(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerSword(const CPlayerSword& rhs);
	virtual ~CPlayerSword() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_ShadowDepth() override;
	virtual void Collision(CGameObject* pOther, string sTag);
	void Set_OBB(_float3 vSize);
	void Set_RHand();
	void Set_Spine();

private:
	CModel* m_pModel = nullptr;
	COBB* m_pOBB = nullptr;
	CPlayer* m_pPlayer = nullptr;
	CTrail* m_pTrail = nullptr;
	CTexture* m_pTexture = nullptr;
	CShader* m_pTexShader = nullptr;

public:
	static CPlayerSword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END