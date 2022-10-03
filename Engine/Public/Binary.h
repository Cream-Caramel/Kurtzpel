#pragma once
#include "Base.h"
#include "Engine_Defines.h"
#include "Engine_Binary.h"

BEGIN(Engine)
class ENGINE_DLL CBinary final : public CBase
{
public:
	CBinary();
	virtual ~CBinary() = default;

public:
	void ResetIndex()
	{
		HierarchyTransformIndex = 0;
		TexturePathIndex = 0;
		NumChildRenIndex = 0;
		HierarchyNameIndex = 0;
		MeshNameIndex = 0;
		MaterialIndex = 0;
		NumPrimitivesIndex = 0;
		pIndicesIndex = 0;
		OffSetMatrixIndex = 0;
		NumVerticesIndex = 0;
		VerticesInfoIndex = 0;
		VertexIndex = 0;
		BlendWeightsIndex = 0;
		DurationsIndex = 0;
		TickPerSecondsIndex = 0;
		NumChannelsIndex = 0;
		ChannelNameIndex = 0;
		NumKeyFramesIndex = 0;
		KeyFramesIndex = 0;
		NumBonesIndex = 0;
		NumWeightsIndex = 0;
		MeshBoneNameIndex = 0;
		NonVertexInfoIndex = 0;
		AnimNameIndex = 0;
		TimeLimitIndex = 0;
	}

public:
	BINARYDATA* m_BinaryData;
	BINARYVECTORDATA* m_BinaryVector;

public:
	int HierarchyTransformIndex = 0;
	int TexturePathIndex = 0;
	int NumChildRenIndex = 0;
	int HierarchyNameIndex = 0;
	int MeshNameIndex = 0;
	int MaterialIndex = 0;
	int NumPrimitivesIndex = 0;
	int pIndicesIndex = 0;
	int OffSetMatrixIndex = 0;
	int NumVerticesIndex = 0;
	int VerticesInfoIndex = 0;
	int VertexIndex = 0;
	int BlendWeightsIndex = 0;
	int DurationsIndex = 0;
	int TickPerSecondsIndex = 0;
	int NumChannelsIndex = 0;
	int ChannelNameIndex = 0;
	int NumKeyFramesIndex = 0;
	int KeyFramesIndex = 0;
	int NumBonesIndex = 0;
	int NumWeightsIndex = 0;
	int MeshBoneNameIndex = 0;
	int NonVertexInfoIndex = 0;
	int AnimNameIndex = 0;
	int TimeLimitIndex = 0;

private:
	virtual void Free() override;
};

END

