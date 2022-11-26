#include "..\Public\PipeLine.h"

IMPLEMENT_SINGLETON(CPipeLine)

CPipeLine::CPipeLine()
{
}

void CPipeLine::Set_Transform(TRANSFORMSTATE eTransformState, _fmatrix TransformMatrix)
{
	XMStoreFloat4x4(&m_TransformMatrix[eTransformState], TransformMatrix);
}

_float3 CPipeLine::Get_CamDir(DIRSTATE eState)
{
	_float3 Dir;
	_matrix WorldMatrix = XMLoadFloat4x4(&m_TransformInverseMatrix[D3DTS_VIEW]);
	switch (eState)
	{
	case Engine::CPipeLine::DIR_RIGHT:	
		XMStoreFloat3(&Dir, WorldMatrix.r[0]);
		return Dir;
	case Engine::CPipeLine::DIR_UP:
		XMStoreFloat3(&Dir, WorldMatrix.r[1]);
		return Dir;
	case Engine::CPipeLine::DIR_LOOK:
		XMStoreFloat3(&Dir, WorldMatrix.r[2]);
		return Dir;
	}
	return _float3{ 0.f,0.f,0.f };
	
}

void CPipeLine::Update()
{
	for (_uint i = 0; i < D3DTS_END; ++i)
		XMStoreFloat4x4(&m_TransformInverseMatrix[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformMatrix[i])));

	memcpy(&m_vCamPosition, &m_TransformInverseMatrix[D3DTS_VIEW].m[3][0], sizeof(_float4));
}

void CPipeLine::Free()
{

}
