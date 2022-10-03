#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "AnimMesh.h"
BEGIN(Client)

class CAnimTool final : public CBase
{
public:
	CAnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimTool() = default;

public:
	void Initialize();
	void ShowAnimWindow(_float fTimeDelta);

#pragma region AnimModelObject
public:
	void ShowAnimModelObjectWindow(_float fTimeDelta);
	void AddAnimMesh(CAnimMesh* pAnimMesh) { m_AnimMesh.push_back(pAnimMesh); Safe_AddRef(pAnimMesh); }

private:
	int Find_ModelObjectIndex(string sTag);
	bool FInd_ModelObjectMap(string sTag);
	bool Find_ModelObjectList(char* ListName);
	void ModelFix();
	void DeleteObject();
	void SaveObject(char* FileName);
	void LoadObject(char* FileName);
	void SaveObjectList();
	void LoadObjectList();
	void CreateModelObject();

private:
	_float m_fPosX = 0.f;
	_float m_fPosY = 0.f;
	_float m_fPosZ = 0.f;
	_float m_fSize = 1.f;
	_float m_fAngleX = 0.f;
	_float m_fAngleY = 0.f;
	_float m_fAngleZ = 0.f;
	int m_iSelectedModel = 0;
	vector<const char*> m_ModelName;

	_uint m_iSelectedObjectList = 0;
	vector<const char*> m_ModelObjectList;

	string m_NowLevels;

private:
	vector<CAnimMesh*> m_AnimMesh;
	map<string, int> m_ModelObjectIndex;
#pragma endregion AnimModelObject
	
	
#pragma region AnimationEdit
public:
	void ShowAnimEditWindow(_float fTimeDelta);
	bool GetPlay() { return m_bPlay; }
	void LoadAnimation();
	void IndexFix();
	void AnimFix();
	int Get_AniIndex() { return m_iSelectedAnimation; }
private:
	_float m_fDuration = 0.f;
	_float m_fTickPerSecond = 0.f;
	_float m_fPlayTime = 0.f;
	_float m_fTimeLimit = 0.f;
	bool m_bPlay = false;
	int m_iAniIndex1 = 0;
	int m_iAniIndex2 = 0;
	int m_iSelectedAnimation = 0;
	vector<const char*> m_Animations;
#pragma endregion AnimationEdit

#pragma region AnimModelEdit
public:
	typedef struct tagModelData
	{
		_tchar* ProtoName;
		char* ModelName;
	}MODELDATA;

public:
	void ShowAnimModelEditWindow(_float fTimeDelta);

private:
	HRESULT CreateModelData(char* sPath, char* sName);
	void DeleteModelData();
	void SaveModelData(char* DatName);
	void LoadModelData(char* DatName);
	void SaveListDate();
	void LoadListDate();
	bool FindModelName(char* ModelName);
	bool Find_ModelDataList(char* ListName);

private:
	int m_iSelectedModelData = 0;
	vector<const char*> m_ModelDataName;
	vector<MODELDATA> m_ModelData;

	_uint m_iSelectedList = 0;
	vector<const char*> m_ModelDataList;

	string m_NowLevel;
#pragma endregion AnimModelEdit

private:
	list<const char*> m_TabName;
	
private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;


public:
	virtual void Free() override;
};
END
