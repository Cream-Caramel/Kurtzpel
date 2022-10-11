#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTexture(const CTexture& rhs);
	virtual ~CTexture() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pTextureFilePath, _uint iNumTextures);
	virtual HRESULT Initialize(void* pArg);

	_uint Get_TextureNums() { return m_iNumTextures; }

	ID3D11ShaderResourceView* Get_Texture(_uint iTextureNum)
	{
		if (iTextureNum >= m_iNumTextures)
			return nullptr;
		else
			return m_SRVs[iTextureNum];
	}


public:
	HRESULT Set_SRV(class CShader* pShader, const char* pConstantName, _uint iTextureIndex = 0);

private:	
	/* ���̴��� ���� ���ε� �� �� �ִ� ��ü */
	vector<ID3D11ShaderResourceView*>				m_SRVs;
	typedef vector<ID3D11ShaderResourceView*>		SRVS;

	_uint			m_iNumTextures = 0;

public:
	static CTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pTextureFilePath, _uint iNumTextures = 1);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free();
};

END