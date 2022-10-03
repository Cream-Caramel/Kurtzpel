#include "stdafx.h"
#include "..\Public\CameraTool.h"
#include "GameInstance.h"
#include "PosCube.h"
#include "MoveCube.h"
#include "LookCube.h"
#include "LookAtCube.h"

CCameraTool::CCameraTool(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	m_pDevice = pDevice;
	m_pContext = pContext;
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CCameraTool::Initalize()
{
}

void CCameraTool::ShowCameraWindow(_float fTimeDelta)
{
	if (m_pMoveCube == nullptr)
	{
		CMoveCube::COLORCUBEDESC ColorCubeDesc;
		ZeroMemory(&ColorCubeDesc, sizeof(CMoveCube::COLORCUBEDESC));
		ColorCubeDesc.vColor = _float4(1.f, 0.f, 0.f, 1.f);

		GI->Add_GameObjectToLayer(TEXT("Prototype_GameObject_MoveCube"), LEVEL_GAMEPLAY, L"Layer_Cube", &ColorCubeDesc);
	}

	if (m_pLookAt == nullptr)
	{
		CLookAtCube::COLORCUBEDESC ColorCubeDesc;
		ZeroMemory(&ColorCubeDesc, sizeof(CMoveCube::COLORCUBEDESC));
		ColorCubeDesc.vColor = _float4(64.f, 0.f, 128.f, 1.f);

		GI->Add_GameObjectToLayer(TEXT("Prototype_GameObject_LookAtCube"), LEVEL_GAMEPLAY, L"Layer_Cube", &ColorCubeDesc);
	}
}

void CCameraTool::CameraKeyInput()
{
	if (GI->Key_Up(DIK_Z))
	{
		CPosCube::POSCUBEDESC PosCubeDesc;
		ZeroMemory(&PosCubeDesc, sizeof(CMoveCube::COLORCUBEDESC));
		PosCubeDesc.vPos = m_pMoveCube->Get_Pos();
		PosCubeDesc.vColor = _float4(0.f, 1.f, 0.f, 1.f);

		char Temp[30];
		_itoa_s(m_iPosCubeIndex, Temp, 10);

		char* a = new char[30];
		for (int i = 0; i < 30; ++i)
		{
			a[i] = Temp[i];
		}
		++m_iPosCubeIndex;
		m_PosCubeName.push_back(a);

		string sTag = Temp;
		PosCubeDesc.sTag = sTag;

		GI->Add_GameObjectToLayer(TEXT("Prototype_GameObject_PosCube"), LEVEL_GAMEPLAY, L"Layer_Cube", &PosCubeDesc);
	}

	if (GI->Key_Up(DIK_X))
	{
		CLookCube::LOOKCUBEDESC LookCubeDesc;
		ZeroMemory(&LookCubeDesc, sizeof(CMoveCube::COLORCUBEDESC));
		LookCubeDesc.vPos = m_pMoveCube->Get_Pos();
		LookCubeDesc.vColor = _float4(1.f, 1.f, 0.f, 1.f);

		char Temp[30];
		_itoa_s(m_iLookCubeIndex, Temp, 10);

		char* a = new char[30];
		for (int i = 0; i < 30; ++i)
		{
			a[i] = Temp[i];
		}
		++m_iLookCubeIndex;
		m_LookCubeName.push_back(a);

		string sTag = Temp;
		LookCubeDesc.sTag = sTag;

		GI->Add_GameObjectToLayer(TEXT("Prototype_GameObject_LookCube"), LEVEL_GAMEPLAY, L"Layer_Cube", &LookCubeDesc);
	}
}

void CCameraTool::ShowCameraEditWindow(_float fTimeDelta)
{
	ImGui::Begin("CameraEdit");                          // Create a window called "Hello, world!" and append into it.

	ImGui::SetWindowSize(ImVec2{ 700.f, 500.f });

	if (ImGui::Button("PosEdit"))
	{
		m_bLookEdit = false;
		m_bPosEdit = true;
	}
	ImGui::SameLine();

	if (ImGui::Button("LookEdit"))
	{
		m_bPosEdit = false;
		m_bLookEdit = true;
	}

	ImGui::SameLine();

	if (m_bPosEdit)
		ImGui::TextColored(ImVec4{ 0.f,1.f,0.f,1.f }, "PosEdit");
	else if (m_bLookEdit)
		ImGui::TextColored(ImVec4{ 1.f,1.f,0.f,1.f }, "LookEdit");

	if (ImGui::BeginListBox("PosCubeList", ImVec2(200, 150)))
	{

		for (int i = 0; i < m_PosCubeName.size(); i++)
		{
			const bool is_selected = (m_iSelectedPosCube == i);
			if (ImGui::Selectable(m_PosCubeName[i], is_selected))
				m_iSelectedPosCube = i;

			if (is_selected)
				ImGui::SetItemDefaultFocus();

		}
		ImGui::EndListBox();
	}

	ImGui::SameLine();

	if (ImGui::BeginListBox("LookCubeList", ImVec2(200, 150)))
	{
		for (int i = 0; i < m_LookCubeName.size(); i++)
		{
			const bool is_selected = (m_iSelectedLookCube == i);
			if (ImGui::Selectable(m_LookCubeName[i], is_selected))
				m_iSelectedLookCube = i;

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}
	if (!m_bPlay)
		DataFix();

	if (ImGui::Button("Play"))
	{
		if (!m_bPlay)
			m_pCamera->Set_Pos(m_PosCube[0]->Get_Pos());
		m_pLookAt->Set_Pos(m_LookCube[0]->Get_Pos());
		m_bPlay = true;
		m_bLookPlay = true;
		m_bStop = false;
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop"))
	{
		m_bStop = true;
	}

	if (ImGui::Button("Reset"))
	{
		m_bPlay = false;
		m_bLookPlay = false;
		m_bStop = true;
		m_pCamera->Set_Pos(m_PosCube[0]->Get_Pos());
		m_pLookAt->Set_Pos(m_LookCube[0]->Get_Pos());
		m_iNowPosCube = 0;
		m_iNowLookCube = 0;
	}

	if (m_bPlay && !m_bStop)
		PlayScene(fTimeDelta);


	ImGui::End();
}

void CCameraTool::DataFix()
{
	if (m_bPosEdit)
	{
		if (m_PosCube.size() >= 1)
		{
			_float3 fPos = m_PosCube[m_iSelectedPosCube]->Get_Pos();
			float PosX = fPos.x;
			ImGui::SliderFloat("PosX", &PosX, -200.f, 200.f, "%.2f", 0);

			float PosY = fPos.y;
			ImGui::SliderFloat("PosY", &PosY, -200.f, 200.f, "%.2f", 0);

			float PosZ = fPos.z;
			ImGui::SliderFloat("PosZ", &PosZ, -200.f, 200.f, "%.2f", 0);
			_float3 temp = { PosX,PosY,PosZ };
			m_PosCube[m_iSelectedPosCube]->Set_Pos(temp);

			float Speed = m_PosCube[m_iSelectedPosCube]->Get_CamSpeed();
			ImGui::SliderFloat("Speed", &Speed, 0.f, 100.f, "%.f", 0);
			m_PosCube[m_iSelectedPosCube]->Set_CamSpeed(Speed);

			float StopLimit = m_PosCube[m_iSelectedPosCube]->Get_StopLimit();
			ImGui::SliderFloat("StopLimit", &StopLimit, 0.f, 20.f, "%.f", 0);
			m_PosCube[m_iSelectedPosCube]->Set_StopLimit(StopLimit);
			if (StopLimit >= 1.f)
				m_PosCube[m_iSelectedPosCube]->Set_Stop();



		}
	}

	if (m_bLookEdit)
	{
		if (m_LookCube.size() >= 1)
		{
			_float3 fPos = m_LookCube[m_iSelectedLookCube]->Get_Pos();
			float PosX = fPos.x;
			ImGui::SliderFloat("PosX", &PosX, -200.f, 200.f, "%.2f", 0);

			float PosY = fPos.y;
			ImGui::SliderFloat("PosY", &PosY, -200.f, 200.f, "%.2f", 0);

			float PosZ = fPos.z;
			ImGui::SliderFloat("PosZ", &PosZ, -200.f, 200.f, "%.2f", 0);
			_float3 temp = { PosX,PosY,PosZ };
			m_LookCube[m_iSelectedLookCube]->Set_Pos(temp);

			float Speed = m_LookCube[m_iSelectedLookCube]->Get_CamSpeed();
			ImGui::SliderFloat("Speed", &Speed, 0.f, 100.f, "%.f", 0);
			m_LookCube[m_iSelectedLookCube]->Set_CamSpeed(Speed);

			float StopLimit = m_LookCube[m_iSelectedLookCube]->Get_StopLimit();
			ImGui::SliderFloat("StopLimit", &StopLimit, 0.f, 20.f, "%.f", 0);
			m_LookCube[m_iSelectedLookCube]->Set_StopLimit(StopLimit);
			if (StopLimit >= 1.f)
				m_LookCube[m_iSelectedLookCube]->Set_Stop();
		}
	}
}

void CCameraTool::PlayScene(_float fTimeDelta)
{
	m_pCamera->LookAt(m_pLookAt->Get_Pos());
	if (!m_PosCube[m_iNowPosCube]->Get_Stop())
	{
		_vector vTempPos = XMLoadFloat3(&m_PosCube[m_iNowPosCube]->Get_Pos());
		vTempPos = XMVectorSetW(vTempPos, 1.f);
		if (m_pCamera->Move(vTempPos, m_PosCube[m_iNowPosCube]->Get_CamSpeed(), fTimeDelta, m_PosCube[m_iNowPosCube]->Get_CamSpeed() * 0.02))
		{
			++m_iNowPosCube;
			if (m_iNowPosCube == m_PosCube.size())
			{
				m_bPlay = false;
				m_iNowPosCube = 0;
				for (auto& iter : m_PosCube)
					iter->PlayEnd();

				m_iNowLookCube = 0;
				for (auto& iter : m_LookCube)
					iter->PlayEnd();
			}
		}
	}
	else
		m_PosCube[m_iNowPosCube]->Stop(fTimeDelta);

	if (m_bLookPlay)
	{
		if (!m_LookCube[m_iNowLookCube]->Get_Stop())
		{
			_vector vTempPos = XMLoadFloat3(&m_LookCube[m_iNowLookCube]->Get_Pos());
			vTempPos = XMVectorSetW(vTempPos, 1.f);
			if (m_pLookAt->Move(vTempPos, m_LookCube[m_iNowLookCube]->Get_CamSpeed(), fTimeDelta, m_LookCube[m_iNowLookCube]->Get_CamSpeed() * 0.02))
			{
				++m_iNowLookCube;
				if (m_iNowLookCube == m_LookCube.size())
				{
					m_bLookPlay = false;
				}
			}
		}
		else
			m_LookCube[m_iNowLookCube]->Stop(fTimeDelta);
	}
}


void CCameraTool::Free()
{
	Safe_Release(m_pCamera);
	//if (m_pMoveCube != nullptr)
	Safe_Release(m_pMoveCube);
	//if(m_pLookAt != nullptr)
	Safe_Release(m_pLookAt);

	for (auto& iter : m_PosCubeName)
	{
		delete iter;
	}

	for (auto& iter : m_LookCubeName)
	{
		delete iter;
	}

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
