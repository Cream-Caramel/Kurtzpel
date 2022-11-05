#pragma	 once

#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& rhs);
	virtual ~CGameObject() = default;

public:
	class CComponent* Get_ComponentPtr(const _tchar* pComponentTag);
	_float Get_CamDistance() const {
		return m_fCamDistance;
	}

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual void Collision(CGameObject* pOther, string sTag) {};
	bool Get_Dead() { return m_bDead; }
	void Set_Dead() { m_bDead = true; }

	void Set_Tag(string sTag) { m_sTag = sTag; }
	string Get_Tag() { return m_sTag; }

	_float& Get_Hp() { return m_fNowHp; }
	_float& Get_MaxHp() { return m_fMaxHp; }
	void Set_Hp(_float fHp) { m_fNowHp = fHp; }
	void Minus_Hp(_float fHp) { m_fNowHp -= fHp; }
	_float Get_Mp() { return m_fNowMp; }
	_float& Get_MaxMp() { return m_fMaxMp; }
	void Set_Mp(_float fMp) { m_fNowMp = fMp; }
	_float Get_Damage() { return m_fDamage; }
	void Set_Damage(_float fDamage) { m_fDamage = fDamage; }
	void Set_Collision(_bool bCollision) { m_bCollision = bCollision; }
	void Set_MaxHit(_uint iMaxHit) { m_iMaxHit = iMaxHit; m_iNumHit = 0; }
	_bool Can_Hit();
	_bool Get_bCollision() { return m_bCollision; }
	

protected:
	ID3D11Device*			m_pDevice = nullptr;
	ID3D11DeviceContext*	m_pContext = nullptr;
	_bool m_bColliderRender = false; //콜라이더를 랜더할것인지
	_bool m_bColliderRenderAll = false;
protected: /* 객체에게 추가된 컴포넌트들을 키로 분류하여 보관한다. */
	map<const _tchar*, class CComponent*>			m_Components;
	typedef map<const _tchar*, class CComponent*>	COMPONENTS;

protected:
	_float				m_fCamDistance = 0.f;
	_float m_fCollisionAcc = 0.f; // 충돌쿨타임 누적시간
	_float m_fColiisionTime; // 충돌 쿨타임
	_bool				m_bDead = false;
	_bool				m_bCollision = false;
	
	_bool				m_bMotionChange = true;
	string m_sTag = "";
	_float m_fMaxHp;
	_float m_fNowHp;
	_float m_fMaxMp;
	_float m_fNowMp;
	_float m_fDamage;
	_uint m_iMaxHit = 0;
	_uint m_iNumHit = 0;
	_bool m_bNormalTex = false;
	

protected:
	HRESULT Add_Component(_uint iLevelIndex, const _tchar* pPrototypeTag, const _tchar* pComponentTag, class CComponent** ppOut, void* pArg = nullptr);
	

private:
	class CComponent* Find_Component(const _tchar* pComponentTag);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END