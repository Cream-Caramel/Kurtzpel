#pragma once
#include "Client_Defines.h"
#include "Mesh.h"
BEGIN(Client)

class CPlayerRageSword final : public CMesh
{
public:
	typedef struct tagRageSword
	{
		int iSowrdNum;
	}RAGESOWRD;
private:
	CPlayerRageSword(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CPlayerRageSword(const CPlayerRageSword& rhs);
	virtual ~CPlayerRageSword() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	void StartDissolve();
	void EndDissolve();
	void Set_On(_float4 vPos);
	void Set_Off();


private:
	CModel* m_pModel;
	CTexture* m_pDissolveTexture = nullptr;
	_bool m_bStartDissolve = false;
	_bool m_bEndDissolve = false;
	_float m_fDissolveAcc = 0.f;
	RAGESOWRD m_RageSowrdInfo;

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};
END