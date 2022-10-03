#include "../Default/stdafx.h"
#include "..\Public\ImGui_Manager.h"
#include "GameInstance.h"
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>
#include "MoveCube.h"
#include "PosCube.h"
#include "LookCube.h"
#include "UITool.h"
#include "UI.h"
#include "CameraTool.h"
#include "MapTool.h"
#include "ModelTool.h"
#include "Mesh.h"
#include "AnimTool.h"
#include "AnimMesh.h"


IMPLEMENT_SINGLETON(CImGui_Manager)

CImGui_Manager::CImGui_Manager()
	:m_bShow_Test_Window(false)
{
}


HRESULT CImGui_Manager::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pDevice = pDevice;
	m_pContext = pContext;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	temp = io.ConfigFlags;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Dear ImGui style
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(pDevice,pContext);

	m_TabName.push_back("MainWindow");
	m_TabName.push_back("UITool");
	m_TabName.push_back("CameraTool");
	m_TabName.push_back("MapTool");
	m_TabName.push_back("ModelTool");
	m_TabName.push_back("AnimTool");

	m_pUITool = new CUITool(m_pDevice, m_pContext);
	
	m_pUITool->Initialize();

	m_pCameraTool = new CCameraTool(m_pDevice, m_pContext);

	m_pModelTool = new CModelTool(m_pDevice, m_pContext);
	m_pModelTool->Initialize();

	m_pMapTool = new CMapTool(m_pDevice, m_pContext);
	GI->Clone_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Tile"));

	m_pAnimTool = new CAnimTool(m_pDevice, m_pContext);
	m_pAnimTool->Initialize();

	return S_OK;

}

void CImGui_Manager::Tick(_float fTimeDelta)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_fTimeDelta = fTimeDelta;

	Get_KeyInput();

	if (m_bRender)
	{
		if (m_bMainWindow)
			MainWindow(fTimeDelta);

		/*if (m_bExampleWindow)
			ShowExampleWindow();*/

		if (m_bTextureWindow)
			m_pUITool->ShowTextureWindow(fTimeDelta);

		if (m_bCameraEditWindow)
			m_pCameraTool->ShowCameraEditWindow(fTimeDelta);

		if (m_bModelEditWindow)
			m_pModelTool->ShowModelEditWindow(fTimeDelta);

		if (m_bAnimModelEditWindow)
			m_pAnimTool->ShowAnimModelEditWindow(fTimeDelta);
	}

	ImGui::EndFrame();
}

void CImGui_Manager::Render()
{
	if (m_bRender)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

}

void CImGui_Manager::Get_KeyInput()
{
	if (GI->Key_Down(DIK_F2))
		m_bCameraMove = !m_bCameraMove;

	if (GI->Key_Down(DIK_F1))
		m_bRender = !m_bRender;

	if (GI->Key_Down(DIK_F3))
	{
		m_bKeyInput = !m_bKeyInput;
	}

	if (m_bKeyInput)
	{
		switch (m_eTool)
		{
		case Client::CImGui_Manager::Tool_Main:
			break;
		case Client::CImGui_Manager::Tool_UI:
			m_pUITool->UIKeyInput();
			break;
		case Client::CImGui_Manager::Tool_Camera:
			m_pCameraTool->CameraKeyInput();
			break;
		case Client::CImGui_Manager::Tool_Map:
			break;
		case Client::CImGui_Manager::Tool_Model:
			break;
		case Client::CImGui_Manager::Tool_AnimModel:
			break;
		case Client::CImGui_Manager::Tool_END:
			break;
		default:
			break;
		}
	}

}

void CImGui_Manager::MainWindow(_float fTimeDelta)
{
	// TransformWindow
	
	ImGui::Begin("ImGuiAHS");                          // Create a window called "Hello, world!" and append into it.


	if (ImGui::Button("WinSizeUp"))
	{
		m_fWinSizeX += 70.f;
		m_fWinSizeY += 50.f;
	}

	ImGui::SameLine();
	if (ImGui::Button("WinSizeDown"))
	{
		m_fWinSizeX -= 70.f;
		m_fWinSizeY -= 50.f;
	}

	ImGui::SetWindowSize(ImVec2{ m_fWinSizeX, m_fWinSizeY });

	ImGui::Checkbox("CameraMove", &m_bCameraMove);
	ImGui::SameLine();
	ImGui::Checkbox("KeyInput", &m_bKeyInput);
	
	static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
	
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("MainWindow"))
		{
			m_eTool = Tool_Main;
			ChangeTool();		
			ShowExampleWindow(fTimeDelta);
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("UITool"))
		{
			m_eTool = Tool_UI;
			ChangeTool();
			m_pUITool->ShowUIToolWindow(fTimeDelta);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("CameraTool"))
		{
			m_eTool = Tool_Camera;
			ChangeTool();
			m_pCameraTool->ShowCameraWindow(fTimeDelta);
			ImGui::Text("CameraTool");

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("MapTool"))
		{
			m_eTool = Tool_Map;
			ChangeTool();
			m_pMapTool->ShowMapWindow(fTimeDelta);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("ModelTool"))
		{
			m_eTool = Tool_Model;
			ChangeTool();
			m_pModelTool->ShowModelWindow(fTimeDelta);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("AnimTool"))
		{
			m_eTool = Tool_AnimModel;
			ChangeTool();
			m_pAnimTool->ShowAnimWindow(fTimeDelta);
			ImGui::EndTabItem();
		}

	
		ImGui::EndTabBar();

		
	}

	ImGui::End();

}

void CImGui_Manager::ShowExampleWindow(_float fTimeDelta)
{
	
	ImGui::Text("Example");

	if (ImGui::Button("CheckBoxButton"))
	{
		m_bTest = !m_bTest;
	}

	ImGui::SameLine();
	ImGui::Checkbox("CheckBox", &m_bTest);

	ImGui::ColorEdit3("Color List", (float*)&m_ColorEdit);

	if (ImGui::Button("ChangeTextColor"))
	{
		m_TextColor.x = m_ColorEdit.x;
		m_TextColor.y = m_ColorEdit.y;
		m_TextColor.z = m_ColorEdit.z;
	}

	ImGui::SameLine();

	if (ImGui::Button("ChangeBackBufferColor"))
	{
		m_fBackBufferColor.x = m_ColorEdit.x;
		m_fBackBufferColor.y = m_ColorEdit.y;
		m_fBackBufferColor.z = m_ColorEdit.z;
	}

	ImGui::SameLine();

	if (ImGui::Button("ChangeButtonColor"))
	{
		m_ButtonColor.x = m_ColorEdit.x;
		m_ButtonColor.y = m_ColorEdit.y;
		m_ButtonColor.z = m_ColorEdit.z;
	}


	ImGui::TextColored(m_TextColor, "TextColor");

	ImGui::SameLine();
	ImGui::ColorButton("ButtonColor", m_ButtonColor);

	static int iRadio = 1;
	ImGui::RadioButton("Radio 1", &iRadio, 1); ImGui::SameLine();
	ImGui::RadioButton("Radio 2", &iRadio, 2); ImGui::SameLine();
	ImGui::RadioButton("Radio 3", &iRadio, 3);

	ImGui::Text("RadioSelected : %d", iRadio);

	const char* ComboBoxItem[] = { "Item1", "Item2", "Item3", "Item4", "Item5" };
	static int ItemIndex = 0;
	ImGui::Combo("ComboBox", &ItemIndex, ComboBoxItem, IM_ARRAYSIZE(ComboBoxItem));

	static char Inputstr[128] = "Typing Here!";
	ImGui::InputText("Input text", Inputstr, IM_ARRAYSIZE(Inputstr));

	ImGui::Text(Inputstr);


	static int DragNum = 0;
	ImGui::DragInt("Drag int", &DragNum, 1);

	static int SliderNum = 0;
	ImGui::SliderInt("Slider int", &SliderNum, 0, 100);

	if (ImGui::Button("Close Me"))
		m_bExampleWindow = false;

}

void CImGui_Manager::AddBaseTile(CBaseTile * pBaseTile)
{
	m_pMapTool->AddBaseTile(pBaseTile);
}

void CImGui_Manager::AddModelObject(CMesh * pMesh)
{
	m_pModelTool->AddModelObject(pMesh);
}

void CImGui_Manager::AddAnimMesh(CAnimMesh * pAnimMesh)
{
	m_pAnimTool->AddAnimMesh(pAnimMesh);
}

bool CImGui_Manager::GetAnimPlay()
{
	return m_pAnimTool->GetPlay();
}

int CImGui_Manager::Get_AniIndex()
{
	return m_pAnimTool->Get_AniIndex();
}

void CImGui_Manager::ChangeTool()
{
	switch (m_eTool)
	{
	case Client::CImGui_Manager::Tool_Main:
		m_bTextureWindow = false;
		m_bCameraEditWindow = false;
		m_bToolCamera = false;
		m_bModelEditWindow = false;
		m_bMapToolWindow = false;
		m_bAnimModelEditWindow = false;
		m_bAnimTool = false;
		
		break;
	case Client::CImGui_Manager::Tool_UI:
		m_bTextureWindow = true;
		m_bCameraEditWindow = false;
		m_bToolCamera = false;
		m_bModelEditWindow = false;
		m_bMapToolWindow = false;
		m_bAnimTool = false;
		m_bAnimModelEditWindow = false;
		break;

	case Client::CImGui_Manager::Tool_Camera:
		m_bCameraEditWindow = true;
		m_bTextureWindow = false;
		m_bToolCamera = true;	
		m_bModelEditWindow = false;
		m_bMapToolWindow = false;
		m_bAnimTool = false;
		m_bAnimModelEditWindow = false;
		break;
	case Client::CImGui_Manager::Tool_Map:
		m_bTextureWindow = false;
		m_bCameraEditWindow = false;
		m_bToolCamera = false;
		m_bModelEditWindow = false;
		m_bMapToolWindow = true;
		m_bAnimTool = false;
		m_bAnimModelEditWindow = false;
		break;

	case Client::CImGui_Manager::Tool_Model:
		m_bTextureWindow = false;
		m_bCameraEditWindow = false;
		m_bToolCamera = false;
		m_bModelEditWindow = true;
		m_bMapToolWindow = false;
		m_bAnimModelEditWindow = false;
		m_bAnimTool = false;
		break;

	case Client::CImGui_Manager::Tool_AnimModel:
		m_bTextureWindow = false;
		m_bCameraEditWindow = false;
		m_bToolCamera = false;
		m_bModelEditWindow = false;
		m_bMapToolWindow = false;
		m_bAnimTool = true;
		m_bAnimModelEditWindow = true;
	case Client::CImGui_Manager::Tool_END:
		break;
	default:
		break;
	}
}


void CImGui_Manager::PushUI(CUI * pUI)
{
	m_pUITool->PushUI(pUI);
}

void CImGui_Manager::SetCameraFree(CCamera_Free * pCamera)
{
	m_pCameraTool->SetCameraFree(pCamera);
}

void CImGui_Manager::SetLookAtCube(CLookAtCube * pLookAt)
{
	m_pCameraTool->SetLookAtCube(pLookAt);
}

void CImGui_Manager::AddMoveCube(CMoveCube * MoveCube)
{
	m_pCameraTool->AddMoveCube(MoveCube);
}

void CImGui_Manager::Free()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_pUITool);
	Safe_Release(m_pCameraTool);
	Safe_Release(m_pMapTool);
	Safe_Release(m_pModelTool);
	Safe_Release(m_pAnimTool);

}
