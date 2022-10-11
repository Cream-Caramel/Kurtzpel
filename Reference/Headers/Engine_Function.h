#pragma once

namespace Engine
{

	template<typename T>
	void Safe_Delete(T& Temp)
	{
		if (Temp)
		{
			delete Temp;
			Temp = nullptr;
		}
	}

	template<typename T>
	void Safe_Delete_Array(T& Temp)
	{
		if (Temp)
		{
			delete[] Temp;
			Temp = nullptr;
		}
	}

	template<typename T>
	unsigned long Safe_AddRef(T& Temp)
	{
		unsigned long dwRefCnt = 0;

		if (nullptr != Temp)
		{
			dwRefCnt = Temp->AddRef();
		}

		return dwRefCnt;
	}

	template<typename T>
	unsigned long Safe_Release(T& Temp)
	{
		unsigned long dwRefCnt = 0;

		if (Temp)
		{
			dwRefCnt = Temp->Release();
			Temp = nullptr;
		}

		return dwRefCnt;
	}

	static char* ChangeToChar(const _tchar* _tchar)
	{
		char* _char;

		int _tcharSize = WideCharToMultiByte(CP_ACP, 0, _tchar, -1, NULL, 0, NULL, NULL);

		_char = new char[_tcharSize];

		WideCharToMultiByte(CP_ACP, 0, _tchar, -1, _char, _tcharSize, 0, 0);

		return _char;
	}

	static _tchar* ChangeTotChar(const char* str)
	{
		_tchar* pStr;

		int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);

		pStr = new _tchar[strSize];

		MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, pStr, strSize);

		return pStr;
	}



}
