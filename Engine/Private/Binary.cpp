#include "..\Public\Binary.h"

CBinary::CBinary()
{
	m_BinaryData = new BINARYDATA;
	m_BinaryVector = new BINARYVECTORDATA;
}

void CBinary::Free()
{
	for (int i = 0; i < m_BinaryVector->HierarchyName.HierarchyName.size(); ++i)
	{
		Safe_Delete_Array(m_BinaryVector->HierarchyName.HierarchyName[i]);
	}

	for (int i = 0; i < m_BinaryVector->MeshName.MeshName.size(); ++i)
	{
		Safe_Delete_Array(m_BinaryVector->MeshName.MeshName[i]);
	}

	for (int i = 0; i < m_BinaryVector->ChannelName.ChannelName.size(); ++i)
	{
		Safe_Delete_Array(m_BinaryVector->ChannelName.ChannelName[i]);
	}

	for (int i = 0; i < m_BinaryVector->TexturePath.TexturePath.size(); ++i)
	{
		Safe_Delete_Array(m_BinaryVector->TexturePath.TexturePath[i]);
	}

	for (int i = 0; i < m_BinaryVector->MeshBoneName.MeshBoneName.size(); ++i)
	{
		Safe_Delete_Array(m_BinaryVector->MeshBoneName.MeshBoneName[i]);
	}

	for (int i = 0; i < m_BinaryVector->AnimName.AnimName.size(); ++i)
	{
		Safe_Delete_Array(m_BinaryVector->AnimName.AnimName[i]);
	}
	m_BinaryVector->HierarchyName.HierarchyName.clear();
	m_BinaryVector->BlendWeights.BlendWeights.clear();
	m_BinaryVector->ChannelName.ChannelName.clear();
	m_BinaryVector->Durations.Durations.clear();
	m_BinaryVector->HierarchyTransform.HierarchyTransform.clear();
	m_BinaryVector->KeyFrames.KeyFrames.clear();
	m_BinaryVector->MaterialIndex.iMaterialIndex.clear();
	m_BinaryVector->MeshName.MeshName.clear();
	m_BinaryVector->NumChannels.NumChannels.clear();
	m_BinaryVector->NumChildRen.iNumChildren.clear();
	m_BinaryVector->NumKeyFrames.NumKeyFrames.clear();
	m_BinaryVector->NumPrimitives.iNumPrimitives.clear();
	m_BinaryVector->NumVertices.NumVertices.clear();
	m_BinaryVector->OffSetMatrix.OffSetMatrix.clear();
	m_BinaryVector->pIndices.pIndices.clear();
	m_BinaryVector->TexturePath.TexturePath.clear();
	m_BinaryVector->TickPerSeconds.TickPerSeconds.clear();
	m_BinaryVector->VertexIndex.VertexIndex.clear();
	m_BinaryVector->VerticesInfo.VerticesInfo.clear();
	m_BinaryVector->NumBones.iNumBones.clear();
	m_BinaryVector->MeshBoneName.MeshBoneName.clear();
	m_BinaryVector->NonAniVertexInfo.NonAniVerticesInfo.clear();
	m_BinaryVector->AnimName.AnimName.clear();
	Safe_Delete_Array(m_BinaryData);
	Safe_Delete(m_BinaryVector);
	
}
