#pragma once

#include "Base.h"
#include "Binary.h"
BEGIN(Engine)
class CAnimModel;

class CChannel final : public CBase
{
public:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(aiNodeAnim*	pAIChannel, CBinary* pBinary);
	HRESULT Initialize(CBinary* pBinary);
	_uint Update_Transformation(_float fPlayTime, _uint iCurrentKeyFrame, class CHierarchyNode* pNode);

public:
	const char* Get_Name() const {
		return m_szName;
	}

	void Reset_KeyFrameIndex() {
		m_iCurrentKeyFrame = 0;
	}

	int Get_KeyFrameSize() { return m_iNumKeyFrames; }
	KEYFRAME GetKeyFrame(int Index)	{ return m_KeyFrames[Index]; }

	_uint Interpolation(_float fTimeDelta, _uint iNextIndex, _float fPlayTime, _uint iCurrentKeyFrame, class CHierarchyNode* pNode, CAnimModel* pAnimModel);

	KEYFRAME	GetFirstKeyFrame() { return m_KeyFrames[0]; }

private:
	char							m_szName[256] = "";

	_uint							m_iNumKeyFrames = 0;
	vector<KEYFRAME>				m_KeyFrames;

	/* 현재 재생되고 있던 키프레임. */
	_uint							m_iCurrentKeyFrame = 0;

	_float m_fInterpolationTime = 0.f;

public:
	static CChannel* Create(aiNodeAnim*	pAIChannel, CBinary* pBinary);
	static CChannel* Create(CBinary* pBinary);
	virtual void Free() override;
};

END