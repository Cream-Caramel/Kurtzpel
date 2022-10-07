#pragma once

#include "Base.h"
#include "Channel.h"
#include "Binary.h"

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& rhs);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize_Prototype(CBinary* pBinary);
	HRESULT Initialize(class CAnimModel* pModel);
	bool Play_Animation(_float fTimeDelta,  CAnimModel* pAnimModel);
	void ResetChennel();

public:
	const char* Get_Name() const {
		return m_szName;
	}

	void Set_Name(const char* AnimName)
	{
		strcpy_s(m_szName, sizeof(char) * 256, AnimName);
	}

	_float& GetTickPerSecond() { return m_fTickPerSecond; }
	void SetTickPerSecond(_float fSecond) { m_fTickPerSecond = fSecond; }

	_float& GetDuration() { return m_fDuration; }
	void SetDuration(_float fDuration) { m_fDuration = fDuration; }

	_float& GetPlayTime() { return m_fPlayTime; }
	void SetPlayTime(_float fPlayTime) { m_fPlayTime = fPlayTime; }

	_float& GetTimeLimit(int iIndex) { return m_TimeLimit[iIndex]; }
	void SetTimeLimit(_float fTimeLimit, int iIndex) { m_TimeLimit[iIndex] = fTimeLimit; }

	int Get_ChannelSize() { return m_Channels.size(); }
	CChannel* GetChannel(int Index) { return m_Channels[Index]; }
	CChannel* GetChannels(char* sName);

	

private:
	/* 이 애니메이션을 구동하기위해 사용되는 뼈의 갯수. */
	char				m_szName[256] = "";

	_uint						m_iNumChannels = 0;
	vector<class CChannel*>		m_Channels;

	/* 애니메이션 재생하는데 걸리는 전체시간. */
	_float						m_fDuration = 0.f;

	/* 애니메이션의 초당 재생 속도. */
	_float						m_fTickPerSecond = 0.f;

	_float						m_fPlayTime = 0.f;

	vector<_float> m_TimeLimit;

	bool m_bAnimEnd = false;

	vector<class CHierarchyNode*>	m_HierarchyNodes;
	vector<_uint>					m_ChannelKeyFrames;
	vector<_uint>					m_ChannelOldKeyFrames;

public:
	static CAnimation* Create(CBinary* pBinary);
	CAnimation* Clone(class CAnimModel* pModel);
	virtual void Free() override;
};

END