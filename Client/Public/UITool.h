#include "Client_Defines.h"
#include "Base.h"
#include "GameInstance.h"
#include "UI.h"
BEGIN(Engine)
class CTexture;
END

BEGIN(Client)
class CUI;

class CUITool final : public CBase
{
public:
	CUITool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CUITool() = default;

public:
	typedef struct TexInfo
	{
		wstring TexPath;
		wstring TexName;
		_uint TexNums;
	}TEXINFO;

public:
	HRESULT Initialize();
#pragma region UITool
public:
	void ShowUIToolWindow(_float fTimeDelta);
	void PushUI(CUI* pUI) { m_UI.push_back(pUI); }
	TEXINFO* GetTexInfo(wstring TexName);
	bool FInd_UIObjectMap(string sTag);
	int Find_UIObjectIndex(string sTag);
public:
	HRESULT AddUIObject(CUI::UIINFO* tex);

public:
	void UIFix();
	void InfoPrint();
	void UIKeyInput();
	void SaveUI(char* DatName);
	void LoadUI(char* DatName);
	void SaveUIList();
	void LoadUIList();
	bool FindUIList(char* ListName);
private:
	_uint m_iSelectedUI = 0;
	_uint m_iSelectedUIList = 0;
	vector<const char*> m_UIList;
	vector<const char*> m_UIDataList;
	vector<CUI*> m_UI;
	map<string, int> m_UIObjectIndex;

	string m_NowLevels;
#pragma endregion UITool

#pragma region Texture
public:
	void ShowTextureWindow(_float fTimeDelta);
public:
	HRESULT PushTexData(char* TexturePath, char* TextureName, int TextureNums);
	HRESULT CreateResourceView(wstring TextureTag, const _tchar * pTextureFilePath, _uint iNumTextures);
	vector<ID3D11ShaderResourceView*> FindResourceView(wstring TextureName);
	HRESULT CheckViewName(wstring TextureName);
	void TexFrame();
	void Animation(_float fTimeDelta);
	void SaveTexture(char* DatName);
	void LoadTexture(char* DatName);
	void SaveTexList();
	void LoadTexList();
	bool FindTextureList(char* ListName);
	_tchar* ChangeC(const char* temp)
	{
		_tchar* pStr;

		int strSize = MultiByteToWideChar(CP_ACP, 0, temp, -1, NULL, NULL);

		pStr = new _tchar[strSize];

		MultiByteToWideChar(CP_ACP, 0, temp, strlen(temp) + 1, pStr, strSize);
		return pStr;
	}

	_tchar* Change(char* temp)
	{
		_tchar* pStr;

		int strSize = MultiByteToWideChar(CP_ACP, 0, temp, -1, NULL, NULL);

		pStr = new _tchar[strSize];

		MultiByteToWideChar(CP_ACP, 0, temp, strlen(temp) + 1, pStr, strSize);
		return pStr;
	}

	

private:
	bool m_bAnimation = false;
	int m_iFrameNum = 0;
	float m_fAniAcc = 0.f;
	float m_fAniSpeed = 1.f;
	int m_iUIfX = 0;
	int m_iUIfY = 0;
	int m_iUISizeX = 0;
	int m_iUISizeY = 0;

	map<wstring, vector<ID3D11ShaderResourceView*>> m_ResourceView;
	vector<const char*> m_TexList;
	vector<const char*> m_TexDataList;

	int m_iSelectedTexture = 0;
	int m_iSelectedTextureData = 0;
	list<TEXINFO*> m_TexInfo;

	string m_NowLevel;


#pragma endregion Texture
private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;

public:
	virtual void Free() override;
};

END