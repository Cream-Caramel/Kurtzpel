#pragma once
#include "Client_Defines.h"
#include "Base.h"
BEGIN(Client)

class CModelTool final : public CBase
{
public:
	CModelTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CModelTool() = default;


#pragma region ModelObject
public:
	void Initialize();
	void ShowModelWindow(_float fTimeDelta);
	void AddModelObject(CMesh* pMesh) { m_Meshes.push_back(pMesh); Safe_AddRef(pMesh); }

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
	vector<CMesh*> m_Meshes;
	vector<const char*> m_ModelName;
	map<string, int> m_ModelObjectIndex;

	_uint m_iSelectedObjectList = 0;
	vector<const char*> m_ModelObjectList;

	string m_NowLevels;

#pragma endregion ModelObject

#pragma region ModelData
public:
	typedef struct tagModelData
	{
		_tchar* ProtoName;
		char* ModelName;
	}MODELDATA;

public:
	void ShowModelEditWindow(_float fTimeDelta);

private:
	HRESULT CreateModelData(char* sPath, char* sName);
	bool Find_ModelDataList(char* ListName);
	void DeleteModelData();
	bool FindModelName(char* ModelName);
	void SaveModelData(char* DatName);
	void LoadModelData(char* DatName);
	void SaveListDate();
	void LoadListDate();
private:
	int m_iSelectedModelData = 0;
	vector<const char*> m_ModelDataName;
	vector<MODELDATA> m_ModelData;

	_uint m_iSelectedList = 0;
	vector<const char*> m_ModelDataList;

	string m_NowLevel;
	
#pragma endregion ModelData



private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;

public:
	virtual void Free() override;
};

END

