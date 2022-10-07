#pragma once

#include "Component.h"
#include "Engine_Binary.h"
#include "Binary.h"

BEGIN(Engine)
class CAnimation;
class ENGINE_DLL CAnimModel final : public CComponent
{

private:
	CAnimModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimModel(const CAnimModel& rhs);
	virtual ~CAnimModel() = default;

public:
	class CHierarchyNode* Get_HierarchyNode(const char* pNodeName);

	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	_matrix Get_PivotMatrix() {
		return XMLoadFloat4x4(&m_PivotMatrix);
	}

	_uint Get_MaterialIndex(_uint iMeshIndex);

	void Set_AnimIndex(_uint iAnimIndex);

	_uint Get_NumAnimations() { return m_iNumAnimations; }

	void Set_PivotMatrix(_matrix PivotMatrix) { XMStoreFloat4x4(&m_PivotMatrix, PivotMatrix); }

public:
	virtual HRESULT Initialize_Prototype(const char* pLoadName, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg);


public:
	HRESULT SetUp_OnShader(class CShader* pShader, _uint iMaterialIndex, TEXTURETYPE eTextureType, const char* pConstantName);
	HRESULT Play_Animation(_float fTimeDelta, CAnimModel* pAnimModel);
	HRESULT Render(class CShader* pShader, _uint iMeshIndex);

	_float GetTickPerSecond();
	void SetTickPerSecond(_float fSecond);

	_float GetDuration();
	void SetDuration(_float fDuration);

	_float GetPlayTime();
	void SetPlayTime(_float fPlayTime);
	void SetPlayTime(_float fPlayTime, int iNextIndex);

	_float GetTimeLimit(int iIndex); 
	void SetTimeLimit(_float TimeLimit, int iIndex); 

	bool GetAniEnd() {return m_bAniEnd;}

	void ResetKeyFrame();
	bool GetChangeBool() { return m_bChangeAni; }
	void SetChangeBool(bool bChange) { m_bChangeAni = bChange;}

	_uint GetNextIndex() { return m_iNextAnimIndex; }
	void SetNextIndex(_uint iNextIndex);

	CAnimation* GetNextAnimation() { return m_Animations[m_iNextAnimIndex]; }
	
	HRESULT LoadBinary(const char* FileName);

private:
	_float4x4					m_PivotMatrix;

private:
	_uint									m_iNumMeshes = 0;
	vector<class CAnimMeshContainer*>			m_Meshes;
	typedef vector<class AnimCMeshContainer*>	MESHES;

private:
	_uint									m_iNumMaterials = 0;
	vector<MATERIALDESC>					m_Materials;

private:
	vector<class CHierarchyNode*>			m_HierarchyNodes;

private:
	_uint								m_iCurrentAnimIndex = 0;
	_uint								m_iNumAnimations = 0;
	_uint								m_iNextAnimIndex = 0;
	vector<class CAnimation*>			m_Animations;
	bool m_bAniEnd = false;
	bool m_bChangeAni = false;
	CBinary* m_pBinary;

private:
	HRESULT Ready_LMeshContainers(_fmatrix PivotMatrix, CBinary* pBinary);
	HRESULT Ready_LMaterials(CBinary* pBinary);
	HRESULT Ready_LHierarchyNodes(class CHierarchyNode* pParent, _uint iDepth, CBinary* pBinary);
	HRESULT Ready_LAnimations(CBinary* pBinary);

public:
	static CAnimModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const char* pLoadName, _fmatrix PivotMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg = nullptr);
	virtual void Free() override;
};

END