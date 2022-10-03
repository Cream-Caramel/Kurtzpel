#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CRenderer;
class CTransform;
class CVIBuffer_Tile;
END

BEGIN(Client)

class CBaseTile final : public CGameObject
{

private:
	CBaseTile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBaseTile(const CBaseTile& rhs);
	virtual ~CBaseTile() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();
	void Create_Tile(int VtxX, int VtxZ);
	void SetTileColor(_float4 TileColor);

private:
	//ID3D11RasterizerState* m_WireFrame;

private:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	CVIBuffer_Tile*		m_pVIBufferCom = nullptr;

private:
	HRESULT Ready_Components();
	HRESULT SetUp_ShaderResources(); /* 셰이더 전역변수에 값 던진다. */

public:
	static CBaseTile* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END