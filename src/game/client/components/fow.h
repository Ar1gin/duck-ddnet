#ifndef GAME_CLIENT_COMPONENTS_FOW_H
#define GAME_CLIENT_COMPONENTS_FOW_H

#include <engine/graphics.h>
#include <game/client/component.h>

class CFOW : CComponent
{
	friend class CGameClient;

public:
	CFOW();
	virtual int Sizeof() const override { return sizeof(*this); }

	virtual void OnReset() override;
	virtual void OnRender() override;

private:
	class CFOWEnable : public CComponent
	{
	public:
		CFOW *m_pFOW;
		virtual int Sizeof() const override { return sizeof(*this); }
		virtual void OnInterfacesInit(CGameClient *pClient) override { if(!GameClient()) CComponent::OnInterfacesInit(pClient); }
		virtual void OnRender() override;
	};

	class CFOWDisable : public CComponent
	{
	public:
		CFOW *m_pFOW;
		virtual int Sizeof() const override { return sizeof(*this); }
		virtual void OnInterfacesInit(CGameClient *pClient) override { if(!GameClient()) CComponent::OnInterfacesInit(pClient); }
		virtual void OnRender() override;
	};

	class CFOWShadow : public CComponent
	{
	public:
		CFOW *m_pFOW;
		virtual int Sizeof() const override { return sizeof(*this); }
		virtual void OnRender() override;
	};

	CFOWEnable m_FOWEnable;
	CFOWDisable m_FOWDisable;
	CFOWShadow m_FOWShadow;

	bool m_Enabled;

	int m_ClipX;
	int m_ClipY;
	int m_ClipW;
	int m_ClipH;
};

#endif
