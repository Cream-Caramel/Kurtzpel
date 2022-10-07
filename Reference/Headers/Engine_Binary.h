#include "Engine_Defines.h"

#pragma once
namespace Engine
{
	typedef struct tagNumMeshes
	{
		unsigned int iNumMeshes;
	}NUMMESHES;

	typedef struct tagNumMaterials
	{
		unsigned int iNumMaterials;
	}NUMMATERIALS;

	typedef struct tagTextureWideFullPath
	{
		vector<_tchar*> TexturePath;
	}TEXTUREPATH;

	typedef struct tagNumChildren
	{
		vector<unsigned int> iNumChildren;
	}NUMCHILDREN;

	typedef struct tagHierarchyName
	{
		vector<char*> HierarchyName;
	}HIERARCHYNAME;

	typedef struct tagHierarchyTransform
	{
		vector<_float4x4> HierarchyTransform;
	}HIERARCHYTRANSFORM;

	typedef struct tagNumAnimations
	{
		unsigned int iNumAnimations;
	}NUMANIMATIONS;

	typedef struct tagMeshName
	{
		vector<char*> MeshName;
	}MESHNAME;

	typedef struct tagMaterialIndex
	{
		vector<unsigned int> iMaterialIndex;
	}MATERIALINDEX;

	typedef struct tagNumPrimitives
	{
		vector<unsigned int> iNumPrimitives;
	}NUMPRIMITIVES;

	typedef struct tagIndices
	{
		vector<FACEINDICES32> pIndices;
	}INDICES;

	typedef struct tagNumBones
	{
		vector<unsigned int> iNumBones;
	}NUMBONES;


	typedef struct tagOffSetMatrix
	{
		vector<_float4x4> OffSetMatrix;
	}OFFSETMATRIX;

	typedef struct tagNumVertices
	{
		vector<unsigned int> NumVertices;
	}NUMVERTICES;

	typedef struct tagVerticesInfo
	{
		vector<VTXANIMMODEL> VerticesInfo;
	}VERTICESINFO;

	typedef struct tagNonVerticesInfo
	{
		vector<VTXMODEL> NonAniVerticesInfo;
	}NONANIVERTICESINFO;

	typedef struct tagNumWeights
	{
		vector<unsigned int> iNumWeights;
	}NUMWEIGHTS;

	typedef struct tagVertexIndex
	{
		vector<unsigned int> VertexIndex;
	}VERTEXINDEX;

	typedef struct tagBlendWeights
	{
		vector<_float> BlendWeights;
	}BLENDWEIGHTS;

	typedef struct tagDurations
	{
		vector<_float> Durations;
	}DURATIONS;

	typedef struct tagTickPerSeconds
	{
		vector<_float> TickPerSeconds;
	}TICKPERSECONDS;

	typedef struct tagNumChannels
	{
		vector<unsigned int> NumChannels;
	}NUMCHANNELS;

	typedef struct tagChannelName
	{
		vector<char*> ChannelName;
	}CHANNELNAME;

	typedef struct tagNumKeyFrames
	{
		vector<unsigned int> NumKeyFrames;
	}NUMKEYFRAMES;

	typedef struct tagKeyFrames
	{
		vector<KEYFRAME> KeyFrames;
	}KEYFRAMES;

	typedef struct tagMeshBoneName
	{
		vector<char*> MeshBoneName;
	}MESHBONENAME;

	typedef struct tagAnimName
	{
		vector<char*> AnimName;
	}ANIMNAME;

	typedef struct tagTimeLimits
	{
		vector<vector<_float>> TimeLimits;
	}TIMELIMITS;

	typedef struct tagBinaryData
	{
		NUMMESHES NumMeshes;
		NUMMATERIALS NumMaterials;		
		NUMANIMATIONS NumAnimations;				
		

	}BINARYDATA;

	typedef struct tagBinaryVectorData
	{
		HIERARCHYTRANSFORM HierarchyTransform;
		TEXTUREPATH TexturePath;
		NUMCHILDREN NumChildRen;
		HIERARCHYNAME HierarchyName;
		MESHNAME MeshName;
		MATERIALINDEX MaterialIndex;
		NUMPRIMITIVES NumPrimitives;
		INDICES pIndices;
		OFFSETMATRIX OffSetMatrix;
		NUMVERTICES NumVertices;
		VERTICESINFO VerticesInfo;
		VERTEXINDEX VertexIndex;
		BLENDWEIGHTS BlendWeights;
		DURATIONS Durations;
		TICKPERSECONDS TickPerSeconds;
		NUMCHANNELS NumChannels;
		CHANNELNAME ChannelName;
		NUMKEYFRAMES NumKeyFrames;
		KEYFRAMES KeyFrames;
		NUMBONES NumBones;
		NUMWEIGHTS NumWeights;
		MESHBONENAME MeshBoneName;
		NONANIVERTICESINFO NonAniVertexInfo;
		ANIMNAME AnimName;
		TIMELIMITS TimeLimits;
	}BINARYVECTORDATA;
}