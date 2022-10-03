#include "stdafx.h"
#include "..\Public\UITool.h"
#include "UI.h"


CUITool::CUITool(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	m_pDevice = pDevice;
	m_pContext = pContext;
	Safe_AddRef(pDevice);
	Safe_AddRef(pContext);
}

HRESULT CUITool::Initialize()
{
	//m_UIDataList.push_back("UIList");
	//SaveUIList();
	LoadUIList();
	LoadTexList();

	return S_OK;
}

void CUITool::ShowUIToolWindow(_float fTimeDelta)
{
	ImGui::PushItemWidth(200.f);
	if (ImGui::BeginCombo(" ", m_UIDataList[m_iSelectedUIList], ImGuiComboFlags_HeightLargest)) {
		for (int i = 0; i < m_UIDataList.size(); ++i) {
			const bool isSelected = (m_iSelectedUIList == i);
			if (ImGui::Selectable(m_UIDataList[i], isSelected)) {
				m_iSelectedUIList = i;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("LoadUIList"))
	{
		char temp[256];
		strcpy_s(temp, sizeof(temp), m_UIDataList[m_iSelectedUIList]);
		if(m_TexList.size() > 0)
		LoadUI(temp);
	}

	ImGui::TextColored(ImVec4{ 0.f,1.f,0.f,1.f }, m_NowLevels.c_str());

	if (ImGui::BeginListBox("UI list", ImVec2(300, 150)))
	{

		for (int i = 0; i < m_UIList.size(); i++)
		{
			const bool is_selected = (m_iSelectedUI == i);
			if (ImGui::Selectable(m_UIList[i], is_selected))
				m_iSelectedUI = i;

			if (is_selected)
				ImGui::SetItemDefaultFocus();

		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add UI"))
	{
		_tchar* temp = ChangeC(m_TexList[m_iSelectedTexture]);
		TEXINFO* tex = GetTexInfo(temp);
		
		if (tex->TexName != L"")
		{
			CUI::UIINFO* UIInfo;
			UIInfo = new CUI::UIINFO;
			_tchar* UIPath = new _tchar[256];
			const _tchar* temp = tex->TexPath.c_str();
			_tchar* UIName = new _tchar[256];
			const _tchar* temp2 = tex->TexName.c_str();
			for (int i = 0; i < 256; ++i)
			{
				UIPath[i] = temp[i];
				UIName[i] = temp2[i];
			}
			UIInfo->TexPath = UIPath;
			UIInfo->TexName = UIName;
			UIInfo->TexNum = tex->TexNums;
			AddUIObject(UIInfo);

			Safe_Delete(UIPath);
			Safe_Delete(UIName);
			Safe_Delete(UIInfo);
		}
		
		delete temp;

	}

	ImGui::SameLine();
	if (ImGui::Button("Delete UI"))
	{		
		if (m_UIList.size() > 0)
		{
			m_UI[m_iSelectedUI]->Set_Dead();
			m_UI.erase(m_UI.begin() + m_iSelectedUI);
			Safe_Delete(m_UIList[m_iSelectedUI]);
			m_UIList.erase(m_UIList.begin() + m_iSelectedUI);
			if(m_iSelectedUI > 0)
			--m_iSelectedUI;
		}		
	}
	if (m_UIList.size() > 0)
	{
		UIFix();
	}
	for (auto& iter : m_UI)
	{	
		iter->Set_Select(false);
	}
	if (m_UI.size() > 0)
	{
		InfoPrint();
		ImGui::ImageButton(m_UI[m_iSelectedUI]->Get_SRV(0), ImVec2{ 100.f,100.f });

	}
	ImGui::PushItemWidth(300.f);
	static char UISaveName[256] = "";
	ImGui::InputText("Input SaveName", UISaveName, IM_ARRAYSIZE(UISaveName));
	ImGui::PopItemWidth();
	if (ImGui::Button("SaveUI"))
	{		
		if(strcmp(UISaveName, ""))
		SaveUI(UISaveName);
	}
}

CUITool::TEXINFO* CUITool::GetTexInfo(wstring TexName)
{
	for (auto& TexInfo : m_TexInfo)
	{
		if (TexInfo->TexName == TexName)
			return TexInfo;
	}
	return nullptr;
}

bool CUITool::FInd_UIObjectMap(string sTag)
{
	for (auto& iter : m_UIObjectIndex)
	{
		if (iter.first == sTag)
			return true;
	}
	return false;
}

void CUITool::UIKeyInput()
{
	if (m_UI[m_iSelectedUI])
	{
		if (GI->Key_Pressing(DIK_LEFT))
		{
			m_UI[m_iSelectedUI]->Set_Position(m_UI[m_iSelectedUI]->Get_PositionX() - 1.f, m_UI[m_iSelectedUI]->Get_PositionY());
		}

		if (GI->Key_Pressing(DIK_RIGHT))
		{
			m_UI[m_iSelectedUI]->Set_Position(m_UI[m_iSelectedUI]->Get_PositionX() + 1.f, m_UI[m_iSelectedUI]->Get_PositionY());
		}

		if (GI->Key_Pressing(DIK_UP))
		{
			m_UI[m_iSelectedUI]->Set_Position(m_UI[m_iSelectedUI]->Get_PositionX(), m_UI[m_iSelectedUI]->Get_PositionY() - 1.f);
		}

		if (GI->Key_Pressing(DIK_DOWN))
		{
			m_UI[m_iSelectedUI]->Set_Position(m_UI[m_iSelectedUI]->Get_PositionX(), m_UI[m_iSelectedUI]->Get_PositionY() + 1.f);
		}

		if (GI->Key_Pressing(DIK_NUMPAD1))
		{
			m_UI[m_iSelectedUI]->Set_Size(m_UI[m_iSelectedUI]->Get_SizeX() - 1.f, m_UI[m_iSelectedUI]->Get_SizeY() - 1.f);
		}

		if (GI->Key_Pressing(DIK_NUMPAD2))
		{
			m_UI[m_iSelectedUI]->Set_Size(m_UI[m_iSelectedUI]->Get_SizeX() + 1.f, m_UI[m_iSelectedUI]->Get_SizeY() + 1.f);
		}

		if (GI->Key_Pressing(DIK_NUMPAD4))
		{
			m_UI[m_iSelectedUI]->Set_SizeX(m_UI[m_iSelectedUI]->Get_SizeX() - 1.f);
		}

		if (GI->Key_Pressing(DIK_NUMPAD5))
		{
			m_UI[m_iSelectedUI]->Set_SizeX(m_UI[m_iSelectedUI]->Get_SizeX() + 1.f);
		}

		if (GI->Key_Pressing(DIK_NUMPAD7))
		{
			m_UI[m_iSelectedUI]->Set_SizeY(m_UI[m_iSelectedUI]->Get_SizeY() - 1.f);
		}

		if (GI->Key_Pressing(DIK_NUMPAD8))
		{
			m_UI[m_iSelectedUI]->Set_SizeY(m_UI[m_iSelectedUI]->Get_SizeY() + 1.f);
		}
	}
}


HRESULT CUITool::AddUIObject(CUI::UIINFO* UIInfo)
{
	if (FAILED(GI->Add_GameObjectToLayer(TEXT("Prototype_GameObject_UI"), LEVEL_GAMEPLAY, TEXT("Layer_UI"), &UIInfo)))
	{
		MSG_BOX(TEXT("Failed Create UI"));
	}
	
	string serch = m_TexList[m_iSelectedTexture];

	char Temp[10];
	_itoa_s(Find_UIObjectIndex(serch), Temp, 10);
	char* Index = new char[10];
	for (int i = 0; i < 10; ++i)
	{
		Index[i] = Temp[i];
	}

	const char* name = m_TexList[m_iSelectedTexture];
	
	char FullName[256] = "";
	strcpy_s(FullName, name);
	strcat_s(FullName, Index);

	Safe_Delete(Index);
	
	char* te = new char[256];
	
	for (int i = 0; i < 256; ++i)
	{
		te[i] = FullName[i];
	}

	const char* a = te;
	
	m_UIList.push_back(a);


	return E_FAIL;
}

void CUITool::UIFix()
{
	if (m_UIList.size() > 0)
	{
		m_iUIfX = (int)m_UI[m_iSelectedUI]->Get_PositionX();
		ImGui::SliderInt("UIPosX", &m_iUIfX, 1, g_iWinSizeX);
		m_UI[m_iSelectedUI]->Set_PositionX(float(m_iUIfX));

		m_iUIfY = (int)m_UI[m_iSelectedUI]->Get_PositionY();
		ImGui::SliderInt("UIPosY", &m_iUIfY, 1, g_iWinSizeY);
		m_UI[m_iSelectedUI]->Set_PositionY(float(m_iUIfY));

		m_iUISizeX = (int)m_UI[m_iSelectedUI]->Get_SizeX();
		ImGui::SliderInt("UISizeX", &m_iUISizeX, 1, g_iWinSizeX);
		m_UI[m_iSelectedUI]->Set_SizeX(float(m_iUISizeX));

		m_iUISizeY = (int)m_UI[m_iSelectedUI]->Get_SizeY();
		ImGui::SliderInt("UISizeY", &m_iUISizeY, 1, g_iWinSizeY);
		m_UI[m_iSelectedUI]->Set_SizeY(float(m_iUISizeY));
	}
}

void CUITool::InfoPrint()
{
	m_UI[m_iSelectedUI]->Set_Select(true);

	char Temp[30];
	_itoa_s(int(m_UI[m_iSelectedUI]->Get_PositionX()), Temp, 10);
	char* posX = new char[30];
	for (int i = 0; i < 30; ++i)
	{
		posX[i] = Temp[i];
	}

	ImGui::Text("PosX : ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4{ 255.f,220.f,0.f,255.f }, posX);


	_itoa_s(int(m_UI[m_iSelectedUI]->Get_PositionY()), Temp, 10);
	char* posY = new char[30];
	for (int i = 0; i < 30; ++i)
	{
		posY[i] = Temp[i];
	}

	ImGui::SameLine();
	ImGui::Text("PosY : ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4{ 255.f,220.f,0.f,255.f }, posY);

	_itoa_s(int(m_UI[m_iSelectedUI]->Get_SizeX()), Temp, 10);
	char* SizeX = new char[30];
	for (int i = 0; i < 30; ++i)
	{
		SizeX[i] = Temp[i];
	}

	ImGui::SameLine();
	ImGui::Text("SizeX : ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4{ 255.f,220.f,0.f,255.f }, SizeX);

	_itoa_s(int(m_UI[m_iSelectedUI]->Get_SizeY()), Temp, 10);
	char* SizeY = new char[30];
	for (int i = 0; i < 30; ++i)
	{
		SizeY[i] = Temp[i];
	}

	ImGui::SameLine();
	ImGui::Text("SizeY : ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4{ 255.f,220.f,0.f,255.f }, SizeY);



	delete(posX);
	delete(posY);
	delete(SizeX);
	delete(SizeY);
}

void CUITool::LoadUI(char* DatName)
{
	string FileSave = DatName;

	string temp = "../Data/UIData/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return;
	}

	//m_TexInfo.clear();

	// 2. 파일 쓰기

	for (auto& iter : m_UI)
	{
		iter->Set_Dead();		
	}
	m_UI.clear();

	for (auto& iter : m_UIList)
	{
		Safe_Delete(iter);
	}
	m_UIList.clear();

	for (auto& iter : m_UIObjectIndex)
	{
		iter.second = 1;
	}

	DWORD		dwByte = 0;

	while (true)
	{
		_tchar temp[256];
		_tchar temp2[256];
		ReadFile(hFile, temp, sizeof(_tchar) * 256, &dwByte, nullptr);
		ReadFile(hFile, temp2, sizeof(_tchar) * 256, &dwByte, nullptr);
		_tchar* UIPath = new _tchar[256];
		_tchar* UIName = new _tchar[256];
		for (int i = 0; i < 256; ++i)
		{
			UIPath[i] = temp[i];
			UIName[i] = temp2[i];
		}
		int UITexNum;
		_float UIPosX;
		_float UIPosY;
		_float UISizeX;
		_float UISizeY;
		ReadFile(hFile, &UITexNum, sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &UIPosX, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UIPosY, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UISizeX, sizeof(_float), &dwByte, nullptr);
		ReadFile(hFile, &UISizeY, sizeof(_float), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete(UIPath);
			Safe_Delete(UIName);
			break;
		}

		CUI::UIINFO* UIInfo;
		UIInfo = new CUI::UIINFO;
		UIInfo->TexPath = UIPath;
		UIInfo->TexName = UIName;
		UIInfo->TexNum = UITexNum;
		UIInfo->UIPosX = UIPosX;
		UIInfo->UIPosY = UIPosY;
		UIInfo->UISizeX = UISizeX;
		UIInfo->UISizeY = UISizeY;
	
		AddUIObject(UIInfo);

		Safe_Delete(UIInfo);
	
		Safe_Delete(UIPath);
		Safe_Delete(UIName);
	}

	m_NowLevels = DatName;
	// 3. 파일 소멸
	CloseHandle(hFile);

}

void CUITool::SaveUI(char* DatName)
{
	if (m_UIList.size() < 1)
	{
		MSG_BOX(TEXT("Don't have Data"));
		return;
	}
	string FileSave = DatName;

	string temp = "../Data/UIData/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_WRITE,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		CREATE_ALWAYS,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Save File"), _T("Fail"), MB_OK);
		return;
	}

	// 2. 파일 쓰기

	DWORD		dwByte = 0;

	for (auto& Data : m_UI)
	{
		CUI::UIINFO* UIInfo;
		UIInfo = Data->Get_UiInfo();

		_tchar* UIPath = new _tchar[256];
		_tchar* UIName = new _tchar[256];
		const _tchar* temp1 = UIInfo->TexPath;
		const _tchar* temp2 = UIInfo->TexName;

		for (int i = 0; i < 256; ++i)
		{
			UIPath[i] = temp1[i];
			UIName[i] = temp2[i];
		}
		int UITexNum = Data->Get_UITexNum();
		float UIPosX = Data->Get_PositionX();
		float UIPosY = Data->Get_PositionY();
		float UISizeX = Data->Get_SizeX();
		float UISizeY = Data->Get_SizeY();
		WriteFile(hFile, UIPath, sizeof(_tchar) * 256, &dwByte, nullptr);
		WriteFile(hFile, UIName, sizeof(_tchar) * 256, &dwByte, nullptr);
		WriteFile(hFile, &UITexNum, sizeof(int), &dwByte, nullptr);
		WriteFile(hFile, &UIPosX, sizeof(float), &dwByte, nullptr);
		WriteFile(hFile, &UIPosY, sizeof(float), &dwByte, nullptr);
		WriteFile(hFile, &UISizeX, sizeof(float), &dwByte, nullptr);
		WriteFile(hFile, &UISizeY, sizeof(float), &dwByte, nullptr);	

		Safe_Delete(UIPath);
		Safe_Delete(UIName);
	}
	char* ListTemp = new char[256];
	strcpy_s(ListTemp, sizeof(char) * 256, DatName);
	if (!FindUIList(ListTemp))
		m_UIDataList.push_back(ListTemp);
	else
		Safe_Delete(ListTemp);
	SaveUIList();

	char Temp2[256] = "";
	strcpy_s(DatName, sizeof(Temp2), Temp2);
	MSG_BOX(TEXT("Succeed Save"));
	// 3. 파일 소멸
	CloseHandle(hFile);
}

void CUITool::SaveUIList()
{
	string FileSave = "../Data/ListData/UIList.dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_WRITE,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		CREATE_ALWAYS,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Save File"), _T("Fail"), MB_OK);
		return;
	}

	DWORD		dwByte = 0;

	for (auto& Data : m_UIDataList)
	{

		char ListName[256] = { 0 };
		strcpy_s(ListName, sizeof(char) * 256, Data);

		WriteFile(hFile, ListName, sizeof(char) * 256, &dwByte, nullptr);
	}

	CloseHandle(hFile);
}

void CUITool::LoadUIList()
{
	string FileSave = "../Data/ListData/UIList.dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return;
	}

	DWORD		dwByte = 0;

	while (true)
	{
		char* ListName = new char[256];
		ReadFile(hFile, ListName, sizeof(char) * 256, &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete_Array(ListName);
			break;
		}

		string FileSave = "../Data/UIData/";

		string FullPath = FileSave + ListName + ".dat";

		wchar_t FilePath[256] = { 0 };

		for (int i = 0; i < FullPath.size(); i++)
		{
			FilePath[i] = FullPath[i];
		}


		HANDLE		hFileSearch = CreateFile(FilePath,			// 파일 경로와 이름 명시
			GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
			NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
			NULL,						// 보안 속성, 기본값	
			OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
			FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
			NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

		if (INVALID_HANDLE_VALUE != hFileSearch)
			m_UIDataList.push_back(ListName);
		else
			Safe_Delete_Array(ListName);
		CloseHandle(hFileSearch);
	}

	// 3. 파일 소멸
	CloseHandle(hFile);
}

bool CUITool::FindUIList(char * ListName)
{
	for (auto& iter : m_UIDataList)
	{
		if (!strcmp(iter, ListName))
			return true;
	}
	return false;
}

void CUITool::ShowTextureWindow(_float fTimeDelta)
{
	ImGui::Begin("TextureManager");                          // Create a window called "Hello, world!" and append into it.

	ImGui::SetWindowSize(ImVec2{ 500.f, 600.f });

	ImGui::PushItemWidth(200.f);
	if (ImGui::BeginCombo(" ", m_TexDataList[m_iSelectedTextureData], ImGuiComboFlags_HeightLargest)) {
		for (int i = 0; i < m_TexDataList.size(); ++i) {
			const bool isSelected = (m_iSelectedTextureData == i);
			if (ImGui::Selectable(m_TexDataList[i], isSelected)) {
				m_iSelectedTextureData = i;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("LoadModelList"))
	{
		char temp[256];
		strcpy_s(temp, sizeof(temp), m_TexDataList[m_iSelectedTextureData]);
		LoadTexture(temp);
	}

	ImGui::TextColored(ImVec4{ 0.f,1.f,0.f,1.f }, m_NowLevel.c_str());


	if (ImGui::BeginListBox("Texture list", ImVec2(300, 150)))
	{

		for (int i = 0; i < m_TexList.size(); i++)
		{
			const bool is_selected = (m_iSelectedTexture == i);
			if (ImGui::Selectable(m_TexList[i], is_selected))
				m_iSelectedTexture = i;

			if (is_selected)
				ImGui::SetItemDefaultFocus();

		}
		ImGui::EndListBox();
	}

	static char TexturePath[256] = "../Bin/Resources/Textures/UI/";
	ImGui::InputText("Input TexturePath", TexturePath, IM_ARRAYSIZE(TexturePath));

	static char TextureNames[256];
	ImGui::InputText("Input TextureName", TextureNames, IM_ARRAYSIZE(TextureNames));

	static int TextureNums = 1;
	ImGui::SliderInt("Texture Nums", &TextureNums, 1, 100);

	if (ImGui::Button("Add Texture"))
	{
		PushTexData(TexturePath, TextureNames, TextureNums);
	}

	ImGui::SameLine();
	if (ImGui::Button("Delete Texture"))
	{
		if (m_iSelectedTexture != 0)
		{
			int count = 1;
			for (auto& iter : m_TexInfo)
			{
				if (m_iSelectedTexture == count)
				{
					Safe_Delete(iter);
					m_TexInfo.remove(iter);
					break;
				}
				else
					++count;
			}
			delete (m_TexList[m_iSelectedTexture]);
			m_TexList.erase(m_TexList.begin() + m_iSelectedTexture);

			m_iSelectedTexture -= 1;
		}
	}
	ImGui::TextColored(ImVec4{ 0,150,0,1 }, "Now Frame : %d", m_iFrameNum);

	ImGui::SliderFloat("Animation Speed", &m_fAniSpeed, 0.01f, 1.f);

	if (m_TexList.size() > 0)
	{
		TexFrame();
		Animation(fTimeDelta);
	}
	static char DatName[256] = "";
	ImGui::InputText("Input DatName", DatName, IM_ARRAYSIZE(DatName));

	if (ImGui::Button("SaveTexture"))
	{
		SaveTexture(DatName);
	}

	ImGui::SameLine();

	ImGui::End();
}

HRESULT CUITool::CreateResourceView(wstring TextureTag, const _tchar * pTextureFilePath, _uint iNumTextures)
{
	if (FAILED(CheckViewName(TextureTag)))
	{
		MSG_BOX(TEXT("already Use TexName Please Change"));
	}

	vector<ID3D11ShaderResourceView*> vTemp;
	for (_uint i = 0; i < iNumTextures; ++i)
	{
		_tchar			szFullPath[256] = TEXT("");

		wsprintf(szFullPath, pTextureFilePath, i);

		_tchar			szDrive[256] = TEXT("");
		_tchar			szExt[256] = TEXT("");

		_wsplitpath_s(szFullPath, szDrive, 256, nullptr, 0, nullptr, 0, szExt, 256);

		HRESULT hr = 0;

		ID3D11ShaderResourceView*			pSRV = nullptr;

		if (false == lstrcmp(TEXT(".dds"), szExt))
			hr = DirectX::CreateDDSTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);

		else if (false == lstrcmp(TEXT(".tga"), szExt))
			hr = E_FAIL;

		else
			hr = DirectX::CreateWICTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);

		if (FAILED(hr))
			return E_FAIL;
		if (pSRV == nullptr)
			return E_FAIL;

		vTemp.push_back(pSRV);
	}
	m_ResourceView.emplace(TextureTag, vTemp);
	return S_OK;
}

vector<ID3D11ShaderResourceView*> CUITool::FindResourceView(wstring TextureName)
{
	for (auto& iter : m_ResourceView)
	{
		if (iter.first == TextureName)
		{
			return iter.second;
		}
	}
	vector<ID3D11ShaderResourceView*> temp;
	return temp;

}

HRESULT CUITool::CheckViewName(wstring TextureName)
{
	for (auto& View : m_ResourceView)
	{
		if (View.first == TextureName)
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

void CUITool::TexFrame()
{
	_tchar* TexName = ChangeC(m_TexList[m_iSelectedTexture]);
	if (m_iFrameNum > FindResourceView(TexName).size() - 1)
		m_iFrameNum = 0;

	ImGui::ImageButton(FindResourceView(TexName)[m_iFrameNum], ImVec2{ 150.f,150.f });

	ImGui::SameLine();
	if (ImGui::Button("-"))
	{
		if (m_iFrameNum > 0)
		{
			--m_iFrameNum;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("+"))
	{
		if (m_iFrameNum < FindResourceView(TexName).size() - 1)
		{
			++m_iFrameNum;
		}
	}
	delete TexName;
}

void CUITool::Animation(_float fTimeDelta)
{
	ImGui::SameLine();
	if (ImGui::Button("Play Animation"))
	{
		m_bAnimation = true;
		m_fAniAcc = 0.f;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop Animation"))
	{
		m_bAnimation = false;
		m_fAniAcc = 0.f;
	}

	if (m_bAnimation)
	{
		m_fAniAcc += 1.f * fTimeDelta;
		if (m_fAniAcc > m_fAniSpeed)
		{
			_tchar* TexName = ChangeC(m_TexList[m_iSelectedTexture]);
			if (m_iFrameNum == FindResourceView(TexName).size() - 1)
				m_iFrameNum = 0;
			++m_iFrameNum;
			m_fAniAcc = 0.f;
			Safe_Delete(TexName);
		}

	}
}

void CUITool::SaveTexture(char* DatName)
{
	if (m_TexInfo.size() < 1)
	{
		MSG_BOX(TEXT("Don't have Data"));
		return;
	}
	string FileSave = DatName;

	string temp = "../Data/TextureData/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}


	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_WRITE,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		CREATE_ALWAYS,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Save File"), _T("Fail"), MB_OK);
		return;
	}

	// 2. 파일 쓰기

	DWORD		dwByte = 0;


	for (auto& Data : m_TexInfo)
	{

		_tchar Path[256] = { 0 };

		for (int i = 0; i < 255; ++i)
		{
			if (i > Data->TexPath.size())
			{
				break;
			}
			else
			{
				Path[i] = Data->TexPath[i];
			}
		}
		_tchar Name[256] = { 0 };
		for (int i = 0; i < 255; ++i)
		{
			if (i > Data->TexName.size())
			{
				break;
			}
			else
			{
				Name[i] = Data->TexName[i];
			}
		}
		WriteFile(hFile, Path, sizeof(_tchar) * 256, &dwByte, nullptr);
		WriteFile(hFile, Name, sizeof(_tchar) * 256, &dwByte, nullptr);
		WriteFile(hFile, &Data->TexNums, sizeof(int), &dwByte, nullptr);

	}
	char* ListTemp = new char[256];
	strcpy_s(ListTemp, sizeof(char) * 256, DatName);

	if (!FindTextureList(ListTemp))
		m_TexDataList.push_back(ListTemp);
	else
		Safe_Delete(ListTemp);

	char Temp2[256] = "";
	strcpy_s(DatName, sizeof(Temp2), Temp2);

	SaveTexList();
	MSG_BOX(TEXT("Succeed Save"));
	// 3. 파일 소멸
	CloseHandle(hFile);
}

void CUITool::LoadTexture(char* DatName)
{

	string FileSave = DatName;

	string temp = "../Data/TextureData/";

	FileSave = temp + FileSave + ".dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}
	
	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return;
	}

	//m_TexInfo.clear();

	// 2. 파일 쓰기
	for (auto& iter : m_TexInfo)
	{
		Safe_Delete(iter);
	}

	m_TexInfo.clear();
	for (auto& iter : m_TexList)
	{
		Safe_Delete(iter);
	}
	m_TexList.clear();

	for (auto& pSRV : m_ResourceView)
	{

		for (auto& iter : pSRV.second)
		{
			iter->Release();
		}
		pSRV.second.clear();
	}

	m_ResourceView.clear();

	DWORD		dwByte = 0;


	while (true)
	{
		int itemp;
		_tchar Path[256];
		ReadFile(hFile, &Path, sizeof(_tchar) * 256, &dwByte, nullptr);
		wstring wPath = Path;
		_tchar Name[256];
		ReadFile(hFile, &Name, sizeof(_tchar) * 256, &dwByte, nullptr);
		wstring wName = Name;
		ReadFile(hFile, &itemp, sizeof(int), &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
			break;

		CUITool::TEXINFO* texInfo = new TEXINFO;

		texInfo->TexPath = wPath;
		texInfo->TexName = wName;
		texInfo->TexNums = itemp;

		char* _char;

		int _tcharSize = WideCharToMultiByte(CP_ACP, 0, Name, -1, NULL, 0, NULL, NULL);

		_char = new char[_tcharSize];

		WideCharToMultiByte(CP_ACP, 0, Name, -1, _char, _tcharSize, 0, 0);

		if (FAILED(CreateResourceView(wName, Path, texInfo->TexNums)))
		{
			MSG_BOX(TEXT("Please Check TexPath or TexNums"));
		}
		string temp = _char;
		m_TexList.push_back(_char);
		m_TexInfo.push_back(texInfo);
		if (!FInd_UIObjectMap(temp))
		{
			m_UIObjectIndex.emplace(temp, 1);
		}

	}

	// 3. 파일 소멸
	CloseHandle(hFile);

	m_iSelectedTexture = 0;
	m_iFrameNum = 0;
	m_NowLevel = DatName;

	char Temp2[256] = "";
	strcpy_s(DatName, sizeof(Temp2), Temp2);

}

void CUITool::SaveTexList()
{
	string FileSave = "../Data/ListData/TextureList.dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_WRITE,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		CREATE_ALWAYS,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Save File"), _T("Fail"), MB_OK);
		return;
	}

	DWORD		dwByte = 0;

	for (auto& Data : m_TexDataList)
	{

		char ListName[256] = { 0 };
		strcpy_s(ListName, sizeof(char) * 256, Data);

		WriteFile(hFile, ListName, sizeof(char) * 256, &dwByte, nullptr);
	}

	CloseHandle(hFile);
}

void CUITool::LoadTexList()
{
	string FileSave = "../Data/ListData/TextureList.dat";

	wchar_t FilePath[256] = { 0 };

	for (int i = 0; i < FileSave.size(); i++)
	{
		FilePath[i] = FileSave[i];
	}

	HANDLE		hFile = CreateFile(FilePath,			// 파일 경로와 이름 명시
		GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
		NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
		NULL,						// 보안 속성, 기본값	
		OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
		FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
		NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 팝업 창을 출력해주는 기능의 함수
		// 1. 핸들 2. 팝업 창에 띄우고자하는 메시지 3. 팝업 창 이름 4. 버튼 속성
		MessageBox(g_hWnd, _T("Load File"), _T("Fail"), MB_OK);
		return;
	}

	DWORD		dwByte = 0;

	while (true)
	{

		char* ListName = new char[256];
		ReadFile(hFile, ListName, sizeof(char) * 256, &dwByte, nullptr);

		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			Safe_Delete_Array(ListName);
			break;
		}

		string FileSave = "../Data/TextureData/";

		string FullPath = FileSave + ListName + ".dat";

		wchar_t FilePath[256] = { 0 };

		for (int i = 0; i < FullPath.size(); i++)
		{
			FilePath[i] = FullPath[i];
		}


		HANDLE		hFileSearch = CreateFile(FilePath,			// 파일 경로와 이름 명시
			GENERIC_READ,				// 파일 접근 모드 (GENERIC_WRITE 쓰기 전용, GENERIC_READ 읽기 전용)
			NULL,						// 공유방식, 파일이 열려있는 상태에서 다른 프로세스가 오픈할 때 허용할 것인가, NULL인 경우 공유하지 않는다
			NULL,						// 보안 속성, 기본값	
			OPEN_EXISTING,				// 생성 방식, CREATE_ALWAYS는 파일이 없다면 생성, 있다면 덮어 쓰기, OPEN_EXISTING 파일이 있을 경우에면 열기
			FILE_ATTRIBUTE_NORMAL,		// 파일 속성(읽기 전용, 숨기 등), FILE_ATTRIBUTE_NORMAL 아무런 속성이 없는 일반 파일 생성
			NULL);						// 생성도리 파일의 속성을 제공할 템플릿 파일, 우리는 사용하지 않아서 NULL

		if (INVALID_HANDLE_VALUE != hFileSearch)
			m_TexDataList.push_back(ListName);
		else
			Safe_Delete_Array(ListName);
		CloseHandle(hFileSearch);
	}
	// 3. 파일 소멸
	CloseHandle(hFile);
}

bool CUITool::FindTextureList(char * ListName)
{
	for (auto& iter : m_TexDataList)
	{
		if (!strcmp(iter, ListName))
			return true;
	}
	return false;
}

int CUITool::Find_UIObjectIndex(string sTag)
{
	for (auto& iter : m_UIObjectIndex)
	{
		if (iter.first == sTag)
			return iter.second++;
	}
}

HRESULT CUITool::PushTexData(char* TexturePath, char* TextureName, int TextureNums)
{
	string Path = TexturePath;
	string Name = TextureName;
	string Temp = "";
	string Temp2 = "";
	if (Path != Temp && Name != Temp2)
	{
		char* TempName = new char[256];
		strcpy_s(TempName, sizeof(char) * 256, TextureName);

		_tchar* pStr;

		int strSize = MultiByteToWideChar(CP_ACP, 0, TexturePath, -1, NULL, NULL);

		pStr = new _tchar[strSize];

		MultiByteToWideChar(CP_ACP, 0, TexturePath, strlen(TexturePath) + 1, pStr, strSize);

		_tchar* pStr2;

		int strSize2 = MultiByteToWideChar(CP_ACP, 0, TextureName, -1, NULL, NULL);

		pStr2 = new _tchar[strSize2];

		MultiByteToWideChar(CP_ACP, 0, TextureName, strlen(TextureName) + 1, pStr2, strSize2);

		TexInfo* tex = new TexInfo;
		tex->TexPath = pStr;
		tex->TexName = pStr2;
		tex->TexNums = TextureNums;

		if (FAILED(CreateResourceView(tex->TexName, tex->TexPath.c_str(), TextureNums)))
		{
			MSG_BOX(TEXT("Please Check TexPath or TexNums"));
			return E_FAIL;
		}
		m_TexList.push_back(TempName);
		m_TexInfo.push_back(tex);

		char Temp1[256] = "../Bin/Resources/Textures/UI/";
		char Temp2[256] = "";
		strcpy_s(TexturePath, sizeof(Temp1), Temp1);
		strcpy_s(TextureName, sizeof(Temp2), Temp2);
		TextureNums = 1;

		Safe_Delete(pStr);
		Safe_Delete(pStr2);

		return S_OK;

		m_UIObjectIndex.emplace(Name, 0);
	}
	MSG_BOX(TEXT("You Must Write TexPath, TexName"));
	return E_FAIL;
}

void CUITool::Free()
{
	for (auto& iter : m_UIDataList)
	{
		Safe_Delete(iter);
	}
	for (auto& iter : m_TexList)
	{
		Safe_Delete(iter);
	}
	m_TexList.clear();

	for (auto& iter : m_TexDataList)
	{
		Safe_Delete(iter);
	}
	m_TexDataList.clear();

	for (auto& iter : m_TexInfo)
	{
		Safe_Delete(iter);
	}
	m_TexInfo.clear();

	for (auto& iter : m_UI)
	{
		Safe_Release(iter);
	}
	m_UI.clear();

	for (auto& iter : m_UIList)
	{
		Safe_Delete(iter);
	}
	m_UIList.clear();

	for (auto& pSRV : m_ResourceView)
	{

		for (auto& iter : pSRV.second)
		{
			iter->Release();
		}
		pSRV.second.clear();
	}

	m_ResourceView.clear();

	m_UIObjectIndex.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

}