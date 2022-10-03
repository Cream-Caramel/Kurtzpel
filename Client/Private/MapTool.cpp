#include "stdafx.h"
#include "..\Public\MapTool.h"


CMapTool::CMapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pDevice = pDevice;
	m_pContext = pContext;
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CMapTool::AddBaseTile(CBaseTile * pBaseTile)
{
	m_pBaseTile = pBaseTile; Safe_AddRef(m_pBaseTile);
}

void CMapTool::ShowMapWindow(_float fTimeDelta)
{
	ImGui::ColorEdit4("MyColor", (float*)&m_fTileColor, ImGuiColorEditFlags_NoDragDrop);
	_float4 fTileColor = { m_fTileColor[0],m_fTileColor[1], m_fTileColor[2], m_fTileColor[3] };
	m_pBaseTile->SetTileColor(fTileColor);

	ImGui::SliderInt("TileX", &m_iVtxNumX, 1, 256);
	ImGui::SliderInt("TileZ", &m_iVexNumZ, 1, 256);
	if (ImGui::Button("CreateTile"))
	{
		m_pBaseTile->Create_Tile(m_iVtxNumX + 1, m_iVexNumZ + 1);
	}


}

void CMapTool::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	if (m_pBaseTile != nullptr)
		Safe_Release(m_pBaseTile);
}


