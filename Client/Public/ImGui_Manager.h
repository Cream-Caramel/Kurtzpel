#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "CameraTool.h"

BEGIN(Engine)
class CTexture;
END

BEGIN(Client)
class CUITool;
class CUI;
class CameraTool;
class CCamera_Free;
class CLookAtCube;
class CMoveCube;
class CMapTool;
class CModelTool;
class CMesh;
class CAnimMesh;
class CAnimTool;

class CImGui_Manager final : public CBase
{
	DECLARE_SINGLETON(CImGui_Manager)

public:
	CImGui_Manager();
	virtual ~CImGui_Manager() = default;

public:
	enum ToolType {Tool_Main, Tool_UI, Tool_Camera, Tool_Map, Tool_Model, Tool_AnimModel, Tool_END};

public:
	HRESULT Init(ID3D11Device* pDevice , ID3D11DeviceContext* pContext);
	void Tick(_float fTimeDelta);
	void Render();

public:
	_float4 Get_BackBufferColor() { return m_fBackBufferColor; }
	bool Get_CamaraMove() { return m_bCameraMove; }
	
	
	
	ToolType GetToolType() { return m_eTool; }

private:
	void Get_KeyInput();

private:
	void MainWindow(_float fTimeDelta);
	void ShowExampleWindow(_float fTimeDelta);
	
	
	ToolType m_eTool = Tool_Main;

#pragma region Tab
	void ChangeTool();
	list<const char*> m_TabName;
#pragma endregion Tab


	// MainWindow
#pragma region MainWindow
private:
	_float4 m_fBackBufferColor = { 0.f,0.f,0.f,1.f };
private:
	bool m_bMainWindow = true;
	bool m_bCameraMove = false;
	bool m_bKeyInput = false;
	bool m_bRender = true;
	float m_fWinSizeX = 700.f; // WinsizeX
	float m_fWinSizeY = 600.f; // WinsizeY
	float m_fTimeDelta;
#pragma endregion MainWindow

#pragma region Example

private:
	bool m_bShow_Test_Window;
	bool m_bTest;
	bool m_bExampleWindow = false;

	ImVec4 m_ColorEdit; // ColorEdit float
	ImVec4 m_TextColor = { 255,0,0,1 }; // TextColor
	ImVec4 m_ButtonColor = { 0,255,0,1 }; // ButtonColor
	
#pragma endregion Example

#pragma region UI
public:
	void PushUI(CUI* pUI);
private:
	CUITool* m_pUITool = nullptr;	
	bool m_bTextureWindow = false;
#pragma endregion UI

#pragma region Camera
public:
	void SetCameraFree(CCamera_Free* pCamera);
	void SetLookAtCube(CLookAtCube* pLookAt);
	void AddMoveCube(CMoveCube* m_MoveCube);
	bool GetToolCamera() { return m_bToolCamera; }
private:
	CCameraTool* m_pCameraTool = nullptr;
	bool m_bToolCamera = false;
#pragma endregion Camera


#pragma region CameraEdit
public:	
	void AddPosCube(CPosCube* pPos) { m_pCameraTool->AddPosCube(pPos); }
	void AddLookCube(CLookCube* pLook) { m_pCameraTool->AddLookCube(pLook); }
	int GetPosIndex() { return m_pCameraTool->GetPosIndex(); }
	int GetLookIndex() { return m_pCameraTool->GetLookIndex(); }
	bool GetPosEdit() { return m_pCameraTool->GetPosEdit(); }
	bool GetLookEdit() { return m_pCameraTool->GetLookEdit(); }
	bool GetPlay() { return m_pCameraTool->GetPlay(); }
private:
	bool m_bCameraEditWindow = false;
#pragma endregion CameraEdit

#pragma region Map
public:
	void AddBaseTile(CBaseTile* pBaseTile);
	bool Get_bMapToolWindow() { return m_bMapToolWindow; }
private:
	CMapTool* m_pMapTool = nullptr;
	bool m_bMapToolWindow = false;
#pragma endregion Map

#pragma region Model
public:
	void AddModelObject(CMesh* pMesh);

private:
	bool m_bModelEditWindow = false;
	CModelTool* m_pModelTool = nullptr;

#pragma endregion Model

#pragma region Animation
public:
	void AddAnimMesh(CAnimMesh* pAnimMesh);
	bool Get_AnimToolWindow() { return m_bAnimTool; }
	bool GetAnimPlay();
	int Get_AniIndex();
private:
	bool m_bAnimTool = false;
	bool m_bAnimModelEditWindow = false;
	CAnimTool* m_pAnimTool;
#pragma endregion Animation

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	CTexture* m_pTextureCom;
	ImGuiConfigFlags temp;

public:
	virtual void Free() override;
};

END

