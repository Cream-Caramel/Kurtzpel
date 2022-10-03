#include "..\Public\Animation.h"
#include "Channel.h"
#include "AnimModel.h"
#include "HierarchyNode.h"

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation & rhs)
	: m_fDuration(rhs.m_fDuration)
	, m_Channels(rhs.m_Channels)
	, m_iNumChannels(rhs.m_iNumChannels)
	, m_fTickPerSecond(rhs.m_fTickPerSecond)
	, m_fPlayTime(rhs.m_fPlayTime)
	, m_ChannelOldKeyFrames(rhs.m_ChannelOldKeyFrames)
	, m_fTimeLimit(rhs.m_fTimeLimit)
{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	strcpy_s(m_szName, sizeof(char) * 256, rhs.m_szName);

}

HRESULT CAnimation::Initialize_Prototype(aiAnimation * pAIAnimation, CBinary* pBinary)
{
	strcpy_s(m_szName, pAIAnimation->mName.data);

	char* temp = new char[256];

	for (int i = 0; i < 256; ++i)
	{
		temp[i] = m_szName[i];
	}

	pBinary->m_BinaryVector->AnimName.AnimName.push_back(temp);

	m_fDuration = pAIAnimation->mDuration;
	m_fTickPerSecond = pAIAnimation->mTicksPerSecond;

	/* 현재 애니메이션에서 제어해야할 뼈들의 갯수를 저장한다. */
	m_iNumChannels = pAIAnimation->mNumChannels;

	pBinary->m_BinaryVector->Durations.Durations.push_back(m_fDuration);
	pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds.push_back(m_fTickPerSecond);
	pBinary->m_BinaryVector->NumChannels.NumChannels.push_back(m_iNumChannels);
	pBinary->m_BinaryVector->TimeLimits.TimeLimits.push_back(m_fTimeLimit);

	/* 현재 애니메이션에서 제어해야할 뼈정보들을 생성하여 보관한다. */
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		CChannel*		pChannel = CChannel::Create(pAIAnimation->mChannels[i], pBinary);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

	return S_OK;
}

HRESULT CAnimation::Initialize_Prototype(CBinary * pBinary)
{
	strcpy_s(m_szName, sizeof(char) * 256, pBinary->m_BinaryVector->AnimName.AnimName[pBinary->AnimNameIndex++]);

	m_fDuration = pBinary->m_BinaryVector->Durations.Durations[pBinary->DurationsIndex++];
	m_fTickPerSecond = pBinary->m_BinaryVector->TickPerSeconds.TickPerSeconds[pBinary->TickPerSecondsIndex++];

	m_iNumChannels = pBinary->m_BinaryVector->NumChannels.NumChannels[pBinary->NumChannelsIndex++];
	m_fTimeLimit = pBinary->m_BinaryVector->TimeLimits.TimeLimits[pBinary->TimeLimitIndex++];
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		CChannel*		pChannel = CChannel::Create(pBinary);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

	return S_OK;
}

HRESULT CAnimation::Initialize(CAnimModel* pModel)
{
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		m_ChannelKeyFrames.push_back(0);
		m_ChannelOldKeyFrames.push_back(0);
		CHierarchyNode*		pNode = pModel->Get_HierarchyNode(m_Channels[i]->Get_Name());

		if (nullptr == pNode)
			return E_FAIL;

		m_HierarchyNodes.push_back(pNode);

		Safe_AddRef(pNode);
	}

	return S_OK;
}

bool CAnimation::Play_Animation(_float fTimeDelta, CAnimModel* pAnimModel)
{
	_uint		iChannelIndex = 0;

	m_bAnimEnd = false;

	if (pAnimModel->GetChangeBool())
	{
		for (auto& pChannel : m_Channels)
		{
			pChannel->Interpolation(fTimeDelta, pAnimModel->GetNextIndex(), m_fPlayTime, m_ChannelOldKeyFrames[iChannelIndex], m_HierarchyNodes[iChannelIndex], pAnimModel);
			
			++iChannelIndex;
		}
		return m_bAnimEnd;
	}

	m_fPlayTime += m_fTickPerSecond * fTimeDelta;

	if (m_fPlayTime >= m_fDuration)
	{
		m_fPlayTime = 0.f;
		m_bAnimEnd = true;
		ResetChennel();
		return m_bAnimEnd;
	}
	
	for (auto& pChannel : m_Channels)
	{
		m_ChannelKeyFrames[iChannelIndex] = pChannel->Update_Transformation(m_fPlayTime, m_ChannelKeyFrames[iChannelIndex], m_HierarchyNodes[iChannelIndex]);
		m_ChannelOldKeyFrames[iChannelIndex] = m_ChannelKeyFrames[iChannelIndex];
		++iChannelIndex;
	}

	return m_bAnimEnd;
}

void CAnimation::ResetChennel()
{
	for (auto& iCurrentKeyFrame : m_ChannelKeyFrames)
		iCurrentKeyFrame = 0;
}

CChannel * CAnimation::GetChannels(char * sName)
{
	for (auto& iter : m_Channels)
	{
		if (!strcmp(sName, iter->Get_Name()))
		{
			return iter;
		}
	}
	return nullptr;
}


CAnimation * CAnimation::Create(aiAnimation * pAIAnimation, CBinary* pBinary)
{
	CAnimation*			pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize_Prototype(pAIAnimation, pBinary)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation * CAnimation::Create(CBinary * pBinary)
{
	CAnimation*			pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize_Prototype(pBinary)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation * CAnimation::Clone(CAnimModel* pModel)
{
	CAnimation*			pInstance = new CAnimation(*this);

	if (FAILED(pInstance->Initialize(pModel)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimation::Free()
{
	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();


	for (auto& pHierarchyNode : m_HierarchyNodes)
		Safe_Release(pHierarchyNode);

	m_HierarchyNodes.clear();
}
