#pragma once
#include "Client_Defines.h"
#include "MeshInstance.h"
#include "Renderer.h"
BEGIN(Client)

class CInstanceModel final : public CMeshInstance
{
public:
	typedef struct tagInstanceInfo
	{
		_tchar* sTag;
		LEVEL eLevel;
		CRenderer::RENDERGROUP eRenderGroup = CRenderer::RENDER_NONALPHABLEND;
	}INSTANCEINFO;
private:
	CInstanceModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CInstanceModel(const CInstanceModel& rhs);
	virtual ~CInstanceModel() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;



private:
	CModelsInstance* m_pModel = nullptr;
	INSTANCEINFO* m_InstanceInfo;
	CRenderer::RENDERGROUP m_eRenderGroup;
public:
	static CMeshInstance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END