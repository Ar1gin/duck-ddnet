#include <base/color.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>

#include "fow.h"

CFOW::CFOW()
{
	m_FOWEnable.m_pFOW = this;
	m_FOWDisable.m_pFOW = this;
	m_FOWShadow.m_pFOW = this;
}

void CFOW::OnReset()
{
	m_FOWDisable.OnRender();
	m_Enabled = false;
}

void CFOW::OnRender()
{
	m_Enabled =
		!GameClient()->m_Snap.m_SpecInfo.m_Active &&
		!GameClient()->m_GameInfo.m_AllowZoom &&
		g_Config.m_DcUnlockZoom &&
		g_Config.m_DcFogOfWar &&
		GameClient()->m_Camera.m_Zoom > 1.0;

	if(!m_Enabled)
		return;

	const int Width = Graphics()->ScreenWidth();
	const int Height = Graphics()->ScreenHeight();

	float CurrentWidth;
	float CurrentHeight;
	float TargetWidth;
	float TargetHeight;
	RenderTools()->CalcScreenParams(Graphics()->ScreenAspect(), GameClient()->m_Camera.m_Zoom, &CurrentWidth, &CurrentHeight);
	RenderTools()->CalcScreenParams(Graphics()->ScreenAspect(), 1.0f, &TargetWidth, &TargetHeight);
	const int ViewWidth = (int)(Width * TargetWidth / CurrentWidth);
	const int ViewHeight = (int)(Height * TargetHeight / CurrentHeight);

	if(ViewWidth > Width || ViewHeight > Height || ViewWidth < 0 || ViewHeight < 0)
	{
		m_Enabled = false;
		return;
	}

	m_ClipX = (Width - ViewWidth) / 2;
	m_ClipY = (Height - ViewHeight) / 2;
	m_ClipW = ViewWidth;
	m_ClipH = ViewHeight;
}

void CFOW::CFOWShadow::OnRender()
{
	if(!m_pFOW->m_Enabled)
		return;

	const ColorRGBA Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_DcFogOfWarColor, true));

	Graphics()->MapScreen(0, 0, Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
	Graphics()->DrawRect(0, 0, m_pFOW->m_ClipX, Graphics()->ScreenHeight(), Color, 0, 0);
	Graphics()->DrawRect(m_pFOW->m_ClipX + m_pFOW->m_ClipW, 0, Graphics()->ScreenWidth() - m_pFOW->m_ClipX - m_pFOW->m_ClipW, Graphics()->ScreenHeight(), Color, 0, 0);
	Graphics()->DrawRect(m_pFOW->m_ClipX, 0, m_pFOW->m_ClipW, m_pFOW->m_ClipY, Color, 0, 0);
	Graphics()->DrawRect(m_pFOW->m_ClipX, m_pFOW->m_ClipY + m_pFOW->m_ClipH, m_pFOW->m_ClipW, Graphics()->ScreenHeight() - m_pFOW->m_ClipY - m_pFOW->m_ClipH, Color, 0, 0);
}

void CFOW::CFOWEnable::OnRender()
{
	if(!m_pFOW->m_Enabled)
		return;

	Graphics()->ClipEnable(
		m_pFOW->m_ClipX,
		m_pFOW->m_ClipY,
		m_pFOW->m_ClipW,
		m_pFOW->m_ClipH);
}

void CFOW::CFOWDisable::OnRender()
{
	if(!m_pFOW->m_Enabled)
		return;

	Graphics()->ClipDisable();
}
