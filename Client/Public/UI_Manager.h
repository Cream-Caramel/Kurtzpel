#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN(Client)

class CUI_Manager final : public CBase
{
	DECLARE_SINGLETON(CUI_Manager)
public:
	CUI_Manager();
	virtual ~CUI_Manager() = default;

public:
	virtual void Free() override;
};

END

