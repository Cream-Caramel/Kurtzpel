#include "..\Public\Picking.h"
#include "Transform.h"
#include "PipeLine.h"

IMPLEMENT_SINGLETON(CPicking)

CPicking::CPicking()
{
}


HRESULT CPicking::Initialize(HWND hWnd, _uint iWinCX, _uint iWinCY, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pDevice = pDevice;

	Safe_AddRef(m_pDevice);

	m_pContext = pContext;

	Safe_AddRef(m_pContext);

	m_hWnd = hWnd;

	m_iWinCX = iWinCX;

	m_iWinCY = iWinCY;

	return S_OK;
}

void CPicking::Tick()
{
	POINT			ptMouse;

	GetCursorPos(&ptMouse);

	ScreenToClient(m_hWnd, &ptMouse);

	_float3			vMousePos;

	/* 투영공간상의 마우스 좌표를 구한다. */
	vMousePos.x = _float(ptMouse.x / (m_iWinCX * 0.5f) - 1);
	vMousePos.y = _float(ptMouse.y / (m_iWinCY * -0.5f) + 1);
	vMousePos.z = 0.f;

	/* 뷰스페이스 상의 좌표를 구한다. */
	_float4x4		ProjMatrixInv;
	ProjMatrixInv = CPipeLine::Get_Instance()->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_PROJ);
	XMStoreFloat3(&vMousePos,XMVector3TransformCoord(XMLoadFloat3(&vMousePos),XMLoadFloat4x4(&ProjMatrixInv)));


	/* 뷰스페이스 상의 마우스 레이와 레이포스를 구하자. */
	m_vRayPos = _float3(0.f, 0.f, 0.f);
	XMStoreFloat3(&m_vRayDir, XMLoadFloat3(&vMousePos) - XMLoadFloat3(&m_vRayPos));

	/* 월드스페이스 상의 마우스 레이와 레이포스를 구하자. */
	_float4x4		ViewMatrixInv;
	ViewMatrixInv = CPipeLine::Get_Instance()->Get_TransformFloat4x4_Inverse(CPipeLine::D3DTS_VIEW);
	XMStoreFloat3(&m_vRayPos,XMVector3TransformCoord(XMLoadFloat3(&m_vRayPos), XMLoadFloat4x4(&ViewMatrixInv)));
	XMStoreFloat3(&m_vRayDir,XMVector3TransformNormal(XMLoadFloat3(&m_vRayDir), XMLoadFloat4x4(&ViewMatrixInv)));

}

void CPicking::Compute_LocalRayInfo(_float3 * pRayDir, _float3 * pRayPos, CTransform * pTransform)
{
	_float4x4		WorldMatrixInv;
	XMStoreFloat4x4(&WorldMatrixInv, pTransform->Get_WorldMatrixInverse());
	//_vector a = ;
	//_vector b = );
	XMStoreFloat3(pRayPos, XMVector3TransformCoord(XMLoadFloat3(&m_vRayPos), XMLoadFloat4x4(&WorldMatrixInv)));
	XMStoreFloat3(pRayDir, XMVector3TransformNormal(XMLoadFloat3(&m_vRayDir), XMLoadFloat4x4(&WorldMatrixInv)));
}

void CPicking::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
