/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/client/gameclient.h>
#include <game/client/prediction/entities/character.h>

#include "camera.h"
#include "controls.h"
#include "nameplates.h"

void CNamePlates::RenderNameplate(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo)
{
	int ClientId = pPlayerInfo->m_ClientId;

	vec2 Position;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
		Position = m_pClient->m_aClients[ClientId].m_RenderPos;
	else
		Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), Client()->IntraGameTick(g_Config.m_ClDummy));

	RenderNameplatePos(Position, pPlayerInfo, 1.0f);
}

void CNamePlates::RenderNameplatePos(vec2 Position, const CNetObj_PlayerInfo *pPlayerInfo, float Alpha, bool ForceAlpha)
{
	int ClientId = pPlayerInfo->m_ClientId;

	bool OtherTeam = m_pClient->IsOtherTeam(ClientId);

	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	float FontSizeClan = 18.0f + 20.0f * g_Config.m_ClNameplatesClanSize / 100.0f;

	TextRender()->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_NO_FIRST_CHARACTER_X_BEARING | ETextRenderFlags::TEXT_RENDER_FLAG_NO_LAST_CHARACTER_ADVANCE);
	float YOffset = Position.y - 38;
	ColorRGBA rgb = ColorRGBA(1.0f, 1.0f, 1.0f);

	// render players' key presses
	int ShowDirection = g_Config.m_ClShowDirection;
#if defined(CONF_VIDEORECORDER)
	if(IVideo::Current())
		ShowDirection = g_Config.m_ClVideoShowDirection;
#endif
	if((ShowDirection && ShowDirection != 3 && !pPlayerInfo->m_Local) || (ShowDirection >= 2 && pPlayerInfo->m_Local) || (ShowDirection == 3 && Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK && ClientId == m_pClient->m_aLocalIds[!g_Config.m_ClDummy]))
	{
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		Graphics()->QuadsSetRotation(0);

		const float ShowDirectionImgSize = 22.0f;
		YOffset -= ShowDirectionImgSize;
		vec2 ShowDirectionPos = vec2(Position.x - 11.0f, YOffset);

		bool DirLeft = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId].m_Cur.m_Direction == -1;
		bool DirRight = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId].m_Cur.m_Direction == 1;
		bool Jump = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId].m_Cur.m_Jumped & 1;

		if(pPlayerInfo->m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		{
			DirLeft = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction == -1;
			DirRight = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction == 1;
			Jump = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Jump == 1;
		}
		if(Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK && pPlayerInfo->m_ClientId == m_pClient->m_aLocalIds[!g_Config.m_ClDummy])
		{
			DirLeft = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Direction == -1;
			DirRight = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Direction == 1;
			Jump = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Jump == 1;
		}
		if(DirLeft)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsSetRotation(pi);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x - 30.f, ShowDirectionPos.y);
		}
		else if(DirRight)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x + 30.f, ShowDirectionPos.y);
		}
		if(Jump)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsSetRotation(pi * 3 / 2);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x, ShowDirectionPos.y);
		}
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		Graphics()->QuadsSetRotation(0);
	}

	// render name plate
	if((!pPlayerInfo->m_Local || g_Config.m_ClNameplatesOwn) && g_Config.m_ClNameplates)
	{
		float a = 1;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);

		const char *pName = m_pClient->m_aClients[pPlayerInfo->m_ClientId].m_aName;
		if(str_comp(pName, m_aNamePlates[ClientId].m_aName) != 0 || FontSize != m_aNamePlates[ClientId].m_NameTextFontSize)
		{
			mem_copy(m_aNamePlates[ClientId].m_aName, pName, sizeof(m_aNamePlates[ClientId].m_aName));
			m_aNamePlates[ClientId].m_NameTextFontSize = FontSize;

			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0, 0, FontSize, TEXTFLAG_RENDER);
			Cursor.m_LineWidth = -1;

			// create nameplates at standard zoom
			float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
			Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
			RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);

			m_aNamePlates[ClientId].m_NameTextWidth = TextRender()->TextWidth(FontSize, pName, -1, -1.0f);

			TextRender()->RecreateTextContainer(m_aNamePlates[ClientId].m_NameTextContainerIndex, &Cursor, pName);
			Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			const char *pClan = m_pClient->m_aClients[ClientId].m_aClan;
			if(str_comp(pClan, m_aNamePlates[ClientId].m_aClanName) != 0 || FontSizeClan != m_aNamePlates[ClientId].m_ClanNameTextFontSize)
			{
				mem_copy(m_aNamePlates[ClientId].m_aClanName, pClan, sizeof(m_aNamePlates[ClientId].m_aClanName));
				m_aNamePlates[ClientId].m_ClanNameTextFontSize = FontSizeClan;

				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, 0, 0, FontSizeClan, TEXTFLAG_RENDER);
				Cursor.m_LineWidth = -1;

				// create nameplates at standard zoom
				float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
				Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
				RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);

				m_aNamePlates[ClientId].m_ClanNameTextWidth = TextRender()->TextWidth(FontSizeClan, pClan, -1, -1.0f);

				TextRender()->RecreateTextContainer(m_aNamePlates[ClientId].m_ClanNameTextContainerIndex, &Cursor, pClan);
				Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
			}
		}

		float tw = m_aNamePlates[ClientId].m_NameTextWidth;
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Teams.Team(ClientId))
			rgb = m_pClient->GetDDTeamColor(m_pClient->m_Teams.Team(ClientId), 0.75f);

		ColorRGBA TColor;
		ColorRGBA TOutlineColor;

		if(OtherTeam && !ForceAlpha)
		{
			TOutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.2f * g_Config.m_ClShowOthersAlpha / 100.0f);
			TColor = ColorRGBA(rgb.r, rgb.g, rgb.b, g_Config.m_ClShowOthersAlpha / 100.0f);
		}
		else
		{
			TOutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.5f * a);
			TColor = ColorRGBA(rgb.r, rgb.g, rgb.b, a);
		}
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS)
		{
			if(m_pClient->m_aClients[ClientId].m_Team == TEAM_RED)
				TColor = ColorRGBA(1.0f, 0.5f, 0.5f, a);
			else if(m_pClient->m_aClients[ClientId].m_Team == TEAM_BLUE)
				TColor = ColorRGBA(0.7f, 0.7f, 1.0f, a);
		}

		TOutlineColor.a *= Alpha;
		TColor.a *= Alpha;

		if(m_aNamePlates[ClientId].m_NameTextContainerIndex.Valid())
		{
			YOffset -= FontSize;
			TextRender()->RenderTextContainer(m_aNamePlates[ClientId].m_NameTextContainerIndex, TColor, TOutlineColor, Position.x - tw / 2.0f, YOffset);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			YOffset -= FontSizeClan;
			if(m_aNamePlates[ClientId].m_ClanNameTextContainerIndex.Valid())
				TextRender()->RenderTextContainer(m_aNamePlates[ClientId].m_ClanNameTextContainerIndex, TColor, TOutlineColor, Position.x - m_aNamePlates[ClientId].m_ClanNameTextWidth / 2.0f, YOffset);
		}

		if(g_Config.m_ClNameplatesFriendMark && m_pClient->m_aClients[ClientId].m_Friend)
		{
			YOffset -= FontSize;
			char aFriendMark[] = "♥";

			ColorRGBA Color;

			if(OtherTeam && !ForceAlpha)
				Color = ColorRGBA(1.0f, 0.0f, 0.0f, g_Config.m_ClShowOthersAlpha / 100.0f);
			else
				Color = ColorRGBA(1.0f, 0.0f, 0.0f, a);

			Color.a *= Alpha;

			TextRender()->TextColor(Color);
			float XOffSet = TextRender()->TextWidth(FontSize, aFriendMark, -1, -1.0f) / 2.0f;
			TextRender()->Text(Position.x - XOffSet, YOffset, FontSize, aFriendMark, -1.0f);
		}

		if(g_Config.m_Debug || g_Config.m_ClNameplatesIds) // render client id when in debug as well
		{
			YOffset -= FontSize;
			char aBuf[128];
			str_from_int(pPlayerInfo->m_ClientId, aBuf);
			float XOffset = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f) / 2.0f;
			TextRender()->TextColor(rgb);
			TextRender()->Text(Position.x - XOffset, YOffset, FontSize, aBuf, -1.0f);
		}
	}

	if((g_Config.m_Debug || g_Config.m_ClNameplatesStrong || g_Config.m_DcDrawStats || g_Config.m_DcDrawDJ || g_Config.m_DcDrawHookD) && g_Config.m_ClNameplates)
	{
		bool Following = (m_pClient->m_Snap.m_SpecInfo.m_Active && !GameClient()->m_MultiViewActivated && m_pClient->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW);
		if(m_pClient->m_Snap.m_LocalClientId != -1 || Following)
		{
			int SelectedId = Following ? m_pClient->m_Snap.m_SpecInfo.m_SpectatorId : m_pClient->m_Snap.m_LocalClientId;
			const CGameClient::CSnapState::CCharacterInfo &Selected = m_pClient->m_Snap.m_aCharacters[SelectedId];
			const CGameClient::CSnapState::CCharacterInfo &Other = m_pClient->m_Snap.m_aCharacters[ClientId];
			if(Selected.m_HasExtendedData && Other.m_HasExtendedData)
			{
				float StatAlpha = 1.0;
				if(g_Config.m_ClNameplatesAlways == 0)
					StatAlpha = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);

				if(OtherTeam && !ForceAlpha)
					StatAlpha = g_Config.m_ClShowOthersAlpha / 100.0f;

				StatAlpha *= Alpha;
				ColorRGBA StatColor = ColorRGBA(1, 1, 1, StatAlpha);

				if(SelectedId == ClientId)
					TextRender()->TextColor(rgb);
				else
				{
					const float StatImageSize = 32.0f;
					float ScaleX, ScaleY;
					RenderTools()->GetSpriteScale(SPRITE_HOOK_STRONG, ScaleX, ScaleY);

					std::vector<std::pair<int, IGraphics::CTextureHandle>> stats_to_draw;

					if(g_Config.m_DcDrawStats)
					{
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_MOVEMENTS_DISABLED)
						{
							stats_to_draw.push_back({SPRITE_HUD_LIVE_FROZEN, m_pClient->m_HudSkin.m_SpriteHudLiveFrozen});
						}
						if(Other.m_ExtendedData.m_FreezeEnd == -1)
						{
							stats_to_draw.push_back({SPRITE_HUD_DEEP_FROZEN, m_pClient->m_HudSkin.m_SpriteHudDeepFrozen});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_HOOK)
						{
							stats_to_draw.push_back({SPRITE_HUD_ENDLESS_HOOK, m_pClient->m_HudSkin.m_SpriteHudEndlessHook});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_JUMP)
						{
							stats_to_draw.push_back({SPRITE_HUD_ENDLESS_JUMP, m_pClient->m_HudSkin.m_SpriteHudEndlessJump});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_JETPACK)
						{
							stats_to_draw.push_back({SPRITE_HUD_JETPACK, m_pClient->m_HudSkin.m_SpriteHudJetpack});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_HOOK_HIT_DISABLED)
						{
							stats_to_draw.push_back({SPRITE_HUD_HOOK_HIT_DISABLED, m_pClient->m_HudSkin.m_SpriteHudHookHitDisabled});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_HAMMER_HIT_DISABLED)
						{
							stats_to_draw.push_back({SPRITE_HUD_HAMMER_HIT_DISABLED, m_pClient->m_HudSkin.m_SpriteHudHammerHitDisabled});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_COLLISION_DISABLED)
						{
							stats_to_draw.push_back({SPRITE_HUD_COLLISION_DISABLED, m_pClient->m_HudSkin.m_SpriteHudCollisionDisabled});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_TELEGUN_GUN)
						{
							stats_to_draw.push_back({SPRITE_HUD_TELEPORT_GUN, m_pClient->m_HudSkin.m_SpriteHudTeleportGun});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_TELEGUN_LASER)
						{
							stats_to_draw.push_back({SPRITE_HUD_TELEPORT_LASER, m_pClient->m_HudSkin.m_SpriteHudTeleportLaser});
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_TELEGUN_GRENADE)
						{
							stats_to_draw.push_back({SPRITE_HUD_TELEPORT_GRENADE, m_pClient->m_HudSkin.m_SpriteHudTeleportGrenade});
						}
					}
					bool drawStrong = g_Config.m_ClNameplatesStrong;

					int XOffset = stats_to_draw.size() + drawStrong - 1;
					if(stats_to_draw.size() > 0 || drawStrong)
						YOffset -= StatImageSize;// * ScaleY;

					if(drawStrong)
					{
						Graphics()->TextureClear();
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_STRONGWEAK].m_Id);
						Graphics()->QuadsBegin();
						ColorRGBA StrongWeakStatusColor;
						int StrongWeakSpriteID;
						if(Selected.m_ExtendedData.m_StrongWeakId > Other.m_ExtendedData.m_StrongWeakId)
						{
							StrongWeakStatusColor = color_cast<ColorRGBA>(ColorHSLA(6401973));
							StrongWeakSpriteID = SPRITE_HOOK_STRONG;
						}
						else
						{
							StrongWeakStatusColor = color_cast<ColorRGBA>(ColorHSLA(41131));
							StrongWeakSpriteID = SPRITE_HOOK_WEAK;
						}
						float ClampedAlpha = 1;
						if(g_Config.m_ClNameplatesAlways == 0)
							ClampedAlpha = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);

						if(OtherTeam && !ForceAlpha)
							StrongWeakStatusColor.a = g_Config.m_ClShowOthersAlpha / 100.0f;
						else
							StrongWeakStatusColor.a = ClampedAlpha;

						StrongWeakStatusColor.a *= Alpha;
						Graphics()->SetColor(StrongWeakStatusColor);
						RenderTools()->SelectSprite(StrongWeakSpriteID);
						RenderTools()->GetSpriteScale(StrongWeakSpriteID, ScaleX, ScaleY);
						TextRender()->TextColor(StrongWeakStatusColor);

						RenderTools()->DrawSprite(Position.x - StatImageSize * XOffset * 0.5f, YOffset + (StatImageSize / 2.0f), StatImageSize / 0.7f);
						Graphics()->QuadsEnd();
						//YOffset -= StatImageSize * ScaleY;
						XOffset -= 2;
					}
					// DClient Stats
					if(g_Config.m_DcDrawStats)
					{
						for(size_t i = 0; i < stats_to_draw.size(); i++)
						{
							Graphics()->TextureClear();
							Graphics()->TextureSet(stats_to_draw[i].second);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(StatColor);

							//RenderTools()->SelectSprite(stats_to_draw[i].first);
							//RenderTools()->GetSpriteScale(stats_to_draw[i].first, ScaleX, ScaleY);
							//ScaleX = 1.0f;
							//ScaleY = 1.0f;

							//RenderTools()->DrawSprite(Position.x - StatImageSize * ScaleX * XOffset * 0.5f, YOffset + (StatImageSize / 2.0f) * ScaleY, StatImageSize);
							IGraphics::CQuadItem StatQuad(Position.x - StatImageSize * (XOffset + 1.0f) * 0.5f, YOffset, StatImageSize, StatImageSize);
							Graphics()->QuadsDrawTL(&StatQuad, 1);
							Graphics()->QuadsEnd();
							XOffset -= 2;
						}
					}
					if(g_Config.m_DcDrawDJ && !(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_JUMP))
					{
						int djs_used = clamp(Other.m_ExtendedData.m_JumpedTotal - 1, 0, 5);
						int djs_left = clamp(Other.m_ExtendedData.m_Jumps - 1, 0, 5) - djs_used;
						if(djs_used > 0 || djs_left > 0)
							YOffset -= StatImageSize;
						XOffset = djs_left + djs_used - 1;
						if(djs_left > 0)
						{
							Graphics()->TextureClear();
							Graphics()->TextureSet(m_pClient->m_HudSkin.m_SpriteHudAirjump);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(StatColor);

							//RenderTools()->GetSpriteScale(SPRITE_HUD_AIRJUMP, ScaleX, ScaleY);
							for(; djs_left > 0; djs_left--)
							{
								//RenderTools()->DrawSprite(Position.x - StatImageSize * ScaleX * XOffset * 0.5f, YOffset - (StatImageSize / 2.0f) * ScaleY, StatImageSize);
								IGraphics::CQuadItem DJQuad(Position.x - StatImageSize * (XOffset + 1.0f) * 0.5f, YOffset, StatImageSize, StatImageSize);
								Graphics()->QuadsDrawTL(&DJQuad, 1);
								XOffset -= 2;
							}
							Graphics()->QuadsEnd();
						}
						if(djs_used > 0)
						{
							Graphics()->TextureClear();
							Graphics()->TextureSet(m_pClient->m_HudSkin.m_SpriteHudAirjumpEmpty);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(StatColor);

							//RenderTools()->GetSpriteScale(SPRITE_HUD_AIRJUMP_EMPTY, ScaleX, ScaleY);
							for(; djs_used > 0; djs_used--)
							{
								//RenderTools()->DrawSprite(Position.x - StatImageSize * ScaleX * XOffset * 0.5f, YOffset - (StatImageSize / 2.0f) * ScaleY, StatImageSize);
								IGraphics::CQuadItem DJQuad(Position.x - StatImageSize * (XOffset + 1.0f) * 0.5f, YOffset, StatImageSize, StatImageSize);
								Graphics()->QuadsDrawTL(&DJQuad, 1);
								XOffset -= 2;
							}
							Graphics()->QuadsEnd();
						}

						if(djs_left > 0 || djs_used > 0)
							YOffset -= StatImageSize * ScaleY;
					}
				}
				if(g_Config.m_Debug || g_Config.m_ClNameplatesStrong == 2)
				{
					YOffset -= FontSize;
					char aBuf[12];
					str_from_int(Other.m_ExtendedData.m_StrongWeakId, aBuf);
					float XOffset = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f) / 2.0f;
					TextRender()->Text(Position.x - XOffset, YOffset, FontSize, aBuf, -1.0f);
				}
				if(g_Config.m_DcDrawHookD)
				{
					// TODO: Make this use predicted data
					bool endless = Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_HOOK;
					const CNetObj_Character* data = &Other.m_Cur;
					if(!endless && in_range(data->m_HookedPlayer, MAX_CLIENTS - 1))
					{
						float p = clamp(1.0f - (float)data->m_HookTick / (float)(SERVER_TICK_SPEED + SERVER_TICK_SPEED / 5), 0.0f, 1.0f);
						Graphics()->TextureSet(m_pClient->m_GameSkin.m_aSpriteWeaponCursors[0]);
						Graphics()->QuadsBegin();
						Graphics()->SetColor(StatColor);

						vec2 pos = (/*vec2(data->m_X, data->m_Y)*/ Position + m_pClient->m_aClients[data->m_HookedPlayer].m_RenderPos) / 2;
						const vec2 size = vec2(g_Config.m_DcHookDSize, g_Config.m_DcHookDSize) / 2.0f;

						float angle = (0.5f + 2*p);
						if(angle > 2)
						{
							angle -= 2;
						}
						angle = M_PI * angle;
						vec2 v = vec2(cosf(angle)*2, -sinf(angle)*2);
						if((p > 0.125 && p < 0.375) || (p > 0.625 && p < 0.875))
						{
							v.x = p > 0.5 ? 1 : -1;
							v.y = -sinf(angle);
						}
						else
						{
							v.x = cosf(angle);
							v.y = (p > 0.25 && p < 0.75) ? 1 : -1;
						}
						IGraphics::CFreeformItem CircleQuads[3];
						int numQuads = 0;
						if(p > 0.5f)
						{
							CircleQuads[numQuads] = IGraphics::CFreeformItem(-1, -1, 0, -1, -1, 1, 0, 1);
							numQuads++;
							if(p > 0.75f)
							{
								CircleQuads[numQuads] = IGraphics::CFreeformItem(v.x, v.y, 1, v.y, 0, 0, 1, 0);
								numQuads++;
								CircleQuads[numQuads] = IGraphics::CFreeformItem(0, 0, 1, 0, 0, 1, 1, 1);
								numQuads++;
							}
							else
							{
								CircleQuads[numQuads] = IGraphics::CFreeformItem(0, 0, v.x, v.y, 0, 1, v.x, 1);
								numQuads++;
							}
						}
						else
						{
							if(p > 0.25f)
							{
								CircleQuads[numQuads] = IGraphics::CFreeformItem(-1, 0, 0, 0, -1, v.y, v.x, v.y);
								numQuads++;
								CircleQuads[numQuads] = IGraphics::CFreeformItem(-1, -1, 0, -1, -1, 0, 0, 0);
								numQuads++;
							}
							else
							{
								CircleQuads[numQuads] = IGraphics::CFreeformItem(v.x, -1, 0, -1, v.x, v.y, 0, 0);
								numQuads++;
							}
						}
						for(int i = 0; i < numQuads; i++)
						{
							Graphics()->QuadsSetSubsetFree(
								(CircleQuads[i].m_X0 + 1) / 2.0f,
								(CircleQuads[i].m_Y0 + 1) / 2.0f,
								(CircleQuads[i].m_X1 + 1) / 2.0f,
								(CircleQuads[i].m_Y1 + 1) / 2.0f,
								(CircleQuads[i].m_X2 + 1) / 2.0f,
								(CircleQuads[i].m_Y2 + 1) / 2.0f,
								(CircleQuads[i].m_X3 + 1) / 2.0f,
								(CircleQuads[i].m_Y3 + 1) / 2.0f
							);
							CircleQuads[i].m_X0 = CircleQuads[i].m_X0 * size.x + pos.x;
							CircleQuads[i].m_X1 = CircleQuads[i].m_X1 * size.x + pos.x;
							CircleQuads[i].m_X2 = CircleQuads[i].m_X2 * size.x + pos.x;
							CircleQuads[i].m_X3 = CircleQuads[i].m_X3 * size.x + pos.x;
							CircleQuads[i].m_Y0 = CircleQuads[i].m_Y0 * size.y + pos.y;
							CircleQuads[i].m_Y1 = CircleQuads[i].m_Y1 * size.y + pos.y;
							CircleQuads[i].m_Y2 = CircleQuads[i].m_Y2 * size.y + pos.y;
							CircleQuads[i].m_Y3 = CircleQuads[i].m_Y3 * size.y + pos.y;
							Graphics()->QuadsDrawFreeform(&CircleQuads[i], 1);
						}
						Graphics()->QuadsEnd();
					}
				}
			}
		}
	}

	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->TextOutlineColor(TextRender()->DefaultTextOutlineColor());

	TextRender()->SetRenderFlags(0);
}

void CNamePlates::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	int ShowDirection = g_Config.m_ClShowDirection;
#if defined(CONF_VIDEORECORDER)
	if(IVideo::Current())
		ShowDirection = g_Config.m_ClVideoShowDirection;
#endif
	if(!g_Config.m_ClNameplates && ShowDirection == 0)
		return;

	// get screen edges to avoid rendering offscreen
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	// expand the edges to prevent popping in/out onscreen
	//
	// it is assumed that the nameplate and all its components fit into a 800x800 box placed directly above the tee
	// this may need to be changed or calculated differently in the future
	ScreenX0 -= 400;
	ScreenX1 += 400;
	//ScreenY0 -= 0;
	ScreenY1 += 800;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_apPlayerInfos[i];
		if(!pInfo)
		{
			continue;
		}

		vec2 *pRenderPos;
		if(m_pClient->m_aClients[i].m_SpecCharPresent)
		{
			// Each player can also have a spec char whose nameplate is displayed independently
			pRenderPos = &m_pClient->m_aClients[i].m_SpecChar;
			// don't render offscreen
			if(!(pRenderPos->x < ScreenX0) && !(pRenderPos->x > ScreenX1) && !(pRenderPos->y < ScreenY0) && !(pRenderPos->y > ScreenY1))
			{
				RenderNameplatePos(m_pClient->m_aClients[i].m_SpecChar, pInfo, 0.4f, true);
			}
		}
		if(m_pClient->m_Snap.m_aCharacters[i].m_Active)
		{
			// Only render nameplates for active characters
			pRenderPos = &m_pClient->m_aClients[i].m_RenderPos;
			// don't render offscreen
			if(!(pRenderPos->x < ScreenX0) && !(pRenderPos->x > ScreenX1) && !(pRenderPos->y < ScreenY0) && !(pRenderPos->y > ScreenY1))
			{
				RenderNameplate(
					&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
					&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
					pInfo);
			}
		}
	}
}

void CNamePlates::ResetNamePlates()
{
	for(auto &NamePlate : m_aNamePlates)
	{
		TextRender()->DeleteTextContainer(NamePlate.m_NameTextContainerIndex);
		TextRender()->DeleteTextContainer(NamePlate.m_ClanNameTextContainerIndex);

		NamePlate.Reset();
	}
}

void CNamePlates::OnWindowResize()
{
	ResetNamePlates();
}

void CNamePlates::OnInit()
{
	ResetNamePlates();

	// Quad for the direction arrows above the player
	m_DirectionQuadContainerIndex = Graphics()->CreateQuadContainer(false);
	RenderTools()->QuadContainerAddSprite(m_DirectionQuadContainerIndex, 0.f, 0.f, 22.f);
	Graphics()->QuadContainerUpload(m_DirectionQuadContainerIndex);
}
