#include "..\Public\Navigation.h"

CNavigation::CNavigation(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{

}

CNavigation::CNavigation(const CNavigation & rhs)
	: CComponent(rhs)
	, m_Cells(rhs.m_Cells)
	, m_NavigationDesc(rhs.m_NavigationDesc)
{
	for (auto& pCell : m_Cells)
		Safe_AddRef(pCell);
}

HRESULT CNavigation::Initialize_Prototype(const char* pFilePath)
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	Load_Cell(pFilePath);

	return S_OK;
}

HRESULT CNavigation::Initialize(void * pArg)
{

	if (nullptr != pArg)
		memcpy(&m_NavigationDesc, pArg, sizeof(NAVIGATIONDESC));

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;	



	return S_OK;
}

_bool CNavigation::isMove(_fvector vPosition)
{
	_int	iNeighborIndex = -1;

	/* 현재 쎌 안에서 움직였다. */
	/* 나간방향에 이웃이 있다면. 이웃의 인ㄷ게스를 받아오고. 
	이웃이 없다면 안채워온다. */
	if (true == m_Cells[m_NavigationDesc.iCurrentIndex]->isIn(vPosition, &iNeighborIndex))
		return true;

	/* 현재 셀을 나갔다. */
	else
	{
		/* 나간방향에 이웃이 있었다면. */
		if (0 <= iNeighborIndex)
		{
			while (true)
			{
				if (0 > iNeighborIndex)
					return false;				

				if (true == m_Cells[iNeighborIndex]->isIn(vPosition, &iNeighborIndex))
				{
					/* 커런트 인덱스를 이웃의 인덱스로 바꿔준다. */
					m_NavigationDesc.iCurrentIndex = iNeighborIndex;

					return true;
				}
			}
			
		}

		/* 나간방향에 이웃이 없었다면. */
		else
			return false;
	}
	return _bool();
}


void CNavigation::Load_Cell(const char* pFileName)
{
	string FileSave = pFileName;

	string temp = "../Data/NavigationData/";

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
		return;
	}

	DWORD		dwByte = 0;

	int NavigationSize;
	ReadFile(hFile, &NavigationSize, sizeof(int), &dwByte, nullptr);

	for (int i = 0; i < NavigationSize; ++i)
	{
		int iCellIndex;
		ReadFile(hFile, &iCellIndex, sizeof(int), &dwByte, nullptr);

		_float3 pPoints[3];
		ReadFile(hFile, &pPoints[0], sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &pPoints[1], sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &pPoints[2], sizeof(_float3), &dwByte, nullptr);

		_float3 pNormals[3];
		ReadFile(hFile, &pNormals[0], sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &pNormals[1], sizeof(_float3), &dwByte, nullptr);
		ReadFile(hFile, &pNormals[2], sizeof(_float3), &dwByte, nullptr);

		int pNeighborIndex[3];
		ReadFile(hFile, &pNeighborIndex[0], sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &pNeighborIndex[1], sizeof(int), &dwByte, nullptr);
		ReadFile(hFile, &pNeighborIndex[2], sizeof(int), &dwByte, nullptr);



		if (0 == dwByte)	// 더이상 읽을 데이터가 없을 경우
		{
			break;
		}

		CCell*			pCell = CCell::Create(m_pDevice, m_pContext, iCellIndex, pPoints, pNormals, pNeighborIndex);
		m_Cells.push_back(pCell);

	}
	// 3. 파일 소멸
	CloseHandle(hFile);
}

_float CNavigation::Get_PosY(_fvector vPos)
{
	
	_vector PointA = XMVectorSetW(XMLoadFloat3(&m_Cells[m_NavigationDesc.iCurrentIndex]->Get_Point(CCell::POINT_A)), 1);
	_vector PointB = XMVectorSetW(XMLoadFloat3(&m_Cells[m_NavigationDesc.iCurrentIndex]->Get_Point(CCell::POINT_B)), 1);
	_vector PointC = XMVectorSetW(XMLoadFloat3(&m_Cells[m_NavigationDesc.iCurrentIndex]->Get_Point(CCell::POINT_C)), 1);
	
	_vector Plane = XMPlaneFromPoints(PointA, PointB, PointC);

	_float x = XMVectorGetX(vPos);
	_float z = XMVectorGetZ(vPos);

	_float a = XMVectorGetX(Plane);
	_float b = XMVectorGetY(Plane);
	_float c = XMVectorGetZ(Plane);
	_float d = XMVectorGetW(Plane);

	/* y = (-ax - cz - d) / b */

	return (-a * x - c * z - d) / b;
}



#ifdef _DEBUG

HRESULT CNavigation::Render()
{
	if (m_bNaviRender)
	{
		for (auto& pCell : m_Cells)
		{
			if (nullptr != pCell)
				pCell->Render_Cell();
		}

		m_Cells[m_NavigationDesc.iCurrentIndex]->Render_Cell(0.05f, _float4(1.f, 0.f, 0.f, 1.f));
	}

	return S_OK;
}

#endif // _DEBUG

CNavigation * CNavigation::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char* pFilePath)
{
	CNavigation*			pInstance = new CNavigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pFilePath)))
	{
		MSG_BOX(TEXT("Failed To Created : CNavigation"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent * CNavigation::Clone(void * pArg)
{
	CNavigation*			pInstance = new CNavigation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CNavigation"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

	for (auto& pCell : m_Cells)
		Safe_Release(pCell);

	m_Cells.clear();
}
