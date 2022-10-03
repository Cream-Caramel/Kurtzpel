#pragma once
#include "Base.h"
#include "Client_Defines.h"
BEGIN(Client)

class CRelease_Manager final : public CBase
{
	DECLARE_SINGLETON(CRelease_Manager)
private:
	CRelease_Manager();
	virtual ~CRelease_Manager() = default;

public:
	void Pushtchar(_tchar* _tchar) { m_tchars.push_back(_tchar);}

private:
	list<_tchar*> m_tchars;

public:
	virtual void Free() override;
};
END
