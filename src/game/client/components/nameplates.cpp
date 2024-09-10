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

void CNamePlates::RenderNameplate(vec2 Position, const CNetObj_PlayerInfo *pPlayerInfo, float Alpha, bool ForceAlpha)
{
	SPlayerNamePlate &NamePlate = m_aNamePlates[pPlayerInfo->m_ClientId];
	const auto &ClientData = m_pClient->m_aClients[pPlayerInfo->m_ClientId];
	const bool OtherTeam = m_pClient->IsOtherTeam(pPlayerInfo->m_ClientId);

	const float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	const float FontSizeClan = 18.0f + 20.0f * g_Config.m_ClNameplatesClanSize / 100.0f;

	TextRender()->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_NO_FIRST_CHARACTER_X_BEARING | ETextRenderFlags::TEXT_RENDER_FLAG_NO_LAST_CHARACTER_ADVANCE);
	float YOffset = Position.y - 38;
	ColorRGBA rgb = ColorRGBA(1.0f, 1.0f, 1.0f);

	// render players' key presses
	int ShowDirection = g_Config.m_ClShowDirection;
#if defined(CONF_VIDEORECORDER)
	if(IVideo::Current())
		ShowDirection = g_Config.m_ClVideoShowDirection;
#endif
	if((ShowDirection && ShowDirection != 3 && !pPlayerInfo->m_Local) || (ShowDirection >= 2 && pPlayerInfo->m_Local) || (ShowDirection == 3 && Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK && pPlayerInfo->m_ClientId == m_pClient->m_aLocalIds[!g_Config.m_ClDummy]))
	{
		bool DirLeft;
		bool DirRight;
		bool Jump;
		if(Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK && pPlayerInfo->m_ClientId == m_pClient->m_aLocalIds[!g_Config.m_ClDummy])
		{
			const auto &InputData = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy];
			DirLeft = InputData.m_Direction == -1;
			DirRight = InputData.m_Direction == 1;
			Jump = InputData.m_Jump == 1;
		}
		else if(pPlayerInfo->m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		{
			const auto &InputData = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy];
			DirLeft = InputData.m_Direction == -1;
			DirRight = InputData.m_Direction == 1;
			Jump = InputData.m_Jump == 1;
		}
		else
		{
			const auto &Character = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId];
			DirLeft = Character.m_Cur.m_Direction == -1;
			DirRight = Character.m_Cur.m_Direction == 1;
			Jump = Character.m_Cur.m_Jumped & 1;
		}

		if(OtherTeam && !ForceAlpha)
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, g_Config.m_ClShowOthersAlpha / 100.0f);
		else
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

		const float ShowDirectionImgSize = 22.0f;
		YOffset -= ShowDirectionImgSize;
		const vec2 ShowDirectionPos = vec2(Position.x - 11.0f, YOffset);
		if(DirLeft)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsSetRotation(pi);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x - 30.f, ShowDirectionPos.y);
		}
		else if(DirRight)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsSetRotation(0);
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
		float a;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);
		else
			a = 1.0f;

		if(str_comp(ClientData.m_aName, NamePlate.m_aName) != 0 || FontSize != NamePlate.m_NameTextFontSize)
		{
			str_copy(NamePlate.m_aName, ClientData.m_aName);
			NamePlate.m_NameTextFontSize = FontSize;

			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0, 0, FontSize, TEXTFLAG_RENDER);

			// create nameplates at standard zoom
			float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
			Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
			RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);
			TextRender()->RecreateTextContainer(NamePlate.m_NameTextContainerIndex, &Cursor, ClientData.m_aName);
			Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			if(str_comp(ClientData.m_aClan, NamePlate.m_aClan) != 0 || FontSizeClan != NamePlate.m_ClanTextFontSize)
			{
				str_copy(NamePlate.m_aClan, ClientData.m_aClan);
				NamePlate.m_ClanTextFontSize = FontSizeClan;

				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, 0, 0, FontSizeClan, TEXTFLAG_RENDER);

				// create nameplates at standard zoom
				float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
				Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
				RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);
				TextRender()->RecreateTextContainer(NamePlate.m_ClanTextContainerIndex, &Cursor, ClientData.m_aClan);
				Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
			}
		}

		if(g_Config.m_ClNameplatesTeamcolors)
		{
			const int Team = m_pClient->m_Teams.Team(pPlayerInfo->m_ClientId);
			if(Team)
			{
				rgb = m_pClient->GetDDTeamColor(Team, 0.75f);
			}
		}

		ColorRGBA TColor;
		ColorRGBA TOutlineColor;

		if(OtherTeam && !ForceAlpha)
		{
			TOutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.2f * g_Config.m_ClShowOthersAlpha / 100.0f);
			TColor = rgb.WithAlpha(g_Config.m_ClShowOthersAlpha / 100.0f);
		}
		else
		{
			TOutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.5f * a);
			TColor = rgb.WithAlpha(a);
		}
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS)
		{
			if(ClientData.m_Team == TEAM_RED)
				TColor = ColorRGBA(1.0f, 0.5f, 0.5f, a);
			else if(ClientData.m_Team == TEAM_BLUE)
				TColor = ColorRGBA(0.7f, 0.7f, 1.0f, a);
		}

		TOutlineColor.a *= Alpha;
		TColor.a *= Alpha;

		if(NamePlate.m_NameTextContainerIndex.Valid())
		{
			YOffset -= FontSize;
			TextRender()->RenderTextContainer(NamePlate.m_NameTextContainerIndex, TColor, TOutlineColor, Position.x - TextRender()->GetBoundingBoxTextContainer(NamePlate.m_NameTextContainerIndex).m_W / 2.0f, YOffset);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			YOffset -= FontSizeClan;
			if(NamePlate.m_ClanTextContainerIndex.Valid())
				TextRender()->RenderTextContainer(NamePlate.m_ClanTextContainerIndex, TColor, TOutlineColor, Position.x - TextRender()->GetBoundingBoxTextContainer(NamePlate.m_ClanTextContainerIndex).m_W / 2.0f, YOffset);
		}

		if(g_Config.m_ClNameplatesFriendMark && ClientData.m_Friend)
		{
			YOffset -= FontSize;
			ColorRGBA Color = ColorRGBA(1.0f, 0.0f, 0.0f, Alpha);
			if(OtherTeam && !ForceAlpha)
				Color.a *= g_Config.m_ClShowOthersAlpha / 100.0f;
			else
				Color.a *= a;

			const char *pFriendMark = "♥";
			TextRender()->TextColor(Color);
			TextRender()->Text(Position.x - TextRender()->TextWidth(FontSize, pFriendMark) / 2.0f, YOffset, FontSize, pFriendMark);
		}

		if(g_Config.m_Debug || g_Config.m_ClNameplatesIds) // render client id when in debug as well
		{
			YOffset -= FontSize;
			char aBuf[12];
			str_format(aBuf, sizeof(aBuf), "%d", pPlayerInfo->m_ClientId);
			TextRender()->TextColor(rgb);
			TextRender()->Text(Position.x - TextRender()->TextWidth(FontSize, aBuf) / 2.0f, YOffset, FontSize, aBuf);
		}
	}

	if((g_Config.m_Debug || g_Config.m_ClNameplatesStrong || g_Config.m_DcDrawStats || g_Config.m_DcDrawDJ || g_Config.m_DcDrawHookD) && g_Config.m_ClNameplates)
	{
		const bool Following = (m_pClient->m_Snap.m_SpecInfo.m_Active && !GameClient()->m_MultiViewActivated && m_pClient->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW);
		if(m_pClient->m_Snap.m_LocalClientId != -1 || Following)
		{
			const int SelectedId = Following ? m_pClient->m_Snap.m_SpecInfo.m_SpectatorId : m_pClient->m_Snap.m_LocalClientId;
			const CGameClient::CSnapState::CCharacterInfo &Selected = m_pClient->m_Snap.m_aCharacters[SelectedId];
			const CGameClient::CSnapState::CCharacterInfo &Other = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId];
			if(Selected.m_HasExtendedData && Other.m_HasExtendedData)
			{
				float StatAlpha = 1.0;
				if(g_Config.m_ClNameplatesAlways == 0)
					StatAlpha = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);

				if(OtherTeam && !ForceAlpha)
					StatAlpha = g_Config.m_ClShowOthersAlpha / 100.0f;

				StatAlpha *= Alpha;
				ColorRGBA StatColor = ColorRGBA(1, 1, 1, StatAlpha);

				if(SelectedId == pPlayerInfo->m_ClientId)
				{
					TextRender()->TextColor(rgb);
				}
				else
				{
					const float StatImageSize = 32.0f;
					float ScaleX, ScaleY;
					RenderTools()->GetSpriteScale(SPRITE_HOOK_STRONG, ScaleX, ScaleY);

					std::vector<std::pair<int, IGraphics::CTextureHandle>> StatsToDraw;

					if(g_Config.m_DcDrawStats)
					{
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_MOVEMENTS_DISABLED)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_LIVE_FROZEN, m_pClient->m_HudSkin.m_SpriteHudLiveFrozen);
						}
						if(Other.m_ExtendedData.m_FreezeEnd == -1)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_DEEP_FROZEN, m_pClient->m_HudSkin.m_SpriteHudDeepFrozen);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_HOOK)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_ENDLESS_HOOK, m_pClient->m_HudSkin.m_SpriteHudEndlessHook);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_JUMP)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_ENDLESS_JUMP, m_pClient->m_HudSkin.m_SpriteHudEndlessJump);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_JETPACK)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_JETPACK, m_pClient->m_HudSkin.m_SpriteHudJetpack);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_HOOK_HIT_DISABLED)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_HOOK_HIT_DISABLED, m_pClient->m_HudSkin.m_SpriteHudHookHitDisabled);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_HAMMER_HIT_DISABLED)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_HAMMER_HIT_DISABLED, m_pClient->m_HudSkin.m_SpriteHudHammerHitDisabled);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_COLLISION_DISABLED)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_COLLISION_DISABLED, m_pClient->m_HudSkin.m_SpriteHudCollisionDisabled);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_TELEGUN_GUN)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_TELEPORT_GUN, m_pClient->m_HudSkin.m_SpriteHudTeleportGun);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_TELEGUN_LASER)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_TELEPORT_LASER, m_pClient->m_HudSkin.m_SpriteHudTeleportLaser);
						}
						if(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_TELEGUN_GRENADE)
						{
							StatsToDraw.emplace_back(SPRITE_HUD_TELEPORT_GRENADE, m_pClient->m_HudSkin.m_SpriteHudTeleportGrenade);
						}
					}
					bool DrawStrong = g_Config.m_ClNameplatesStrong;

					int XOffset = StatsToDraw.size() + DrawStrong - 1;
					if(StatsToDraw.size() > 0 || DrawStrong)
						YOffset -= StatImageSize; // * ScaleY;

					if(DrawStrong)
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

						if(OtherTeam && !ForceAlpha)
							StrongWeakStatusColor.a = g_Config.m_ClShowOthersAlpha / 100.0f;
						else if(g_Config.m_ClNameplatesAlways == 0)
							StrongWeakStatusColor.a = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);
						else
							StrongWeakStatusColor.a = 1.0f;

						StrongWeakStatusColor.a *= Alpha;
						Graphics()->SetColor(StrongWeakStatusColor);
						RenderTools()->SelectSprite(StrongWeakSpriteID);
						RenderTools()->GetSpriteScale(StrongWeakSpriteID, ScaleX, ScaleY);
						TextRender()->TextColor(StrongWeakStatusColor);

						RenderTools()->DrawSprite(Position.x - StatImageSize * XOffset * 0.5f, YOffset + (StatImageSize / 2.0f), StatImageSize / 0.7f);
						Graphics()->QuadsEnd();
						XOffset -= 2;
					}
					// DClient Stats
					if(g_Config.m_DcDrawStats)
					{
						for(auto &i : StatsToDraw)
						{
							Graphics()->TextureClear();
							Graphics()->TextureSet(i.second);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(StatColor);

							// RenderTools()->SelectSprite(stats_to_draw[i].first);
							// RenderTools()->GetSpriteScale(stats_to_draw[i].first, ScaleX, ScaleY);
							// ScaleX = 1.0f;
							// ScaleY = 1.0f;

							// RenderTools()->DrawSprite(Position.x - StatImageSize * ScaleX * XOffset * 0.5f, YOffset + (StatImageSize / 2.0f) * ScaleY, StatImageSize);
							IGraphics::CQuadItem StatQuad(Position.x - StatImageSize * (XOffset + 1.0f) * 0.5f, YOffset, StatImageSize, StatImageSize);
							Graphics()->QuadsDrawTL(&StatQuad, 1);
							Graphics()->QuadsEnd();
							XOffset -= 2;
						}
					}
					if(g_Config.m_DcDrawDJ && !(Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_JUMP))
					{
						int DjsUsed = clamp(Other.m_ExtendedData.m_JumpedTotal, 0, 5);
						int DjsLeft = clamp(Other.m_ExtendedData.m_Jumps - 1, 0, 5) - DjsUsed;
						if(DjsUsed > 0 || DjsLeft > 0)
							YOffset -= StatImageSize;
						XOffset = DjsLeft + DjsUsed - 1;
						if(DjsLeft > 0)
						{
							Graphics()->TextureClear();
							Graphics()->TextureSet(m_pClient->m_HudSkin.m_SpriteHudAirjump);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(StatColor);

							// RenderTools()->GetSpriteScale(SPRITE_HUD_AIRJUMP, ScaleX, ScaleY);
							for(; DjsLeft > 0; DjsLeft--)
							{
								// RenderTools()->DrawSprite(Position.x - StatImageSize * ScaleX * XOffset * 0.5f, YOffset - (StatImageSize / 2.0f) * ScaleY, StatImageSize);
								IGraphics::CQuadItem DJQuad(Position.x - StatImageSize * (XOffset + 1.0f) * 0.5f, YOffset, StatImageSize, StatImageSize);
								Graphics()->QuadsDrawTL(&DJQuad, 1);
								XOffset -= 2;
							}
							Graphics()->QuadsEnd();
						}
						if(DjsUsed > 0)
						{
							Graphics()->TextureClear();
							Graphics()->TextureSet(m_pClient->m_HudSkin.m_SpriteHudAirjumpEmpty);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(StatColor);

							// RenderTools()->GetSpriteScale(SPRITE_HUD_AIRJUMP_EMPTY, ScaleX, ScaleY);
							for(; DjsUsed > 0; DjsUsed--)
							{
								// RenderTools()->DrawSprite(Position.x - StatImageSize * ScaleX * XOffset * 0.5f, YOffset - (StatImageSize / 2.0f) * ScaleY, StatImageSize);
								IGraphics::CQuadItem DJQuad(Position.x - StatImageSize * (XOffset + 1.0f) * 0.5f, YOffset, StatImageSize, StatImageSize);
								Graphics()->QuadsDrawTL(&DJQuad, 1);
								XOffset -= 2;
							}
							Graphics()->QuadsEnd();
						}

						if(DjsLeft > 0 || DjsUsed > 0)
							YOffset -= StatImageSize * ScaleY;
					}
				}
				if(g_Config.m_Debug || g_Config.m_ClNameplatesStrong == 2)
				{
					YOffset -= FontSize;
					char aBuf[12];
					str_format(aBuf, sizeof(aBuf), "%d", Other.m_ExtendedData.m_StrongWeakId);
					TextRender()->Text(Position.x - TextRender()->TextWidth(FontSize, aBuf) / 2.0f, YOffset, FontSize, aBuf);
				}
				if(g_Config.m_DcDrawHookD)
				{
					// TODO: Make this use predicted data
					bool Endless = Other.m_ExtendedData.m_Flags & CHARACTERFLAG_ENDLESS_HOOK;
					const CNetObj_Character *Data = &Other.m_Cur;
					if(!Endless && in_range(Data->m_HookedPlayer, MAX_CLIENTS - 1))
					{
						float p = clamp(1.0f - (float)Data->m_HookTick / (float)(SERVER_TICK_SPEED + SERVER_TICK_SPEED / 5), 0.0f, 1.0f);
						Graphics()->TextureSet(m_pClient->m_GameSkin.m_aSpriteWeaponCursors[0]);
						Graphics()->QuadsBegin();
						Graphics()->SetColor(StatColor);

						vec2 pos = (/*vec2(data->m_X, data->m_Y)*/ Position + m_pClient->m_aClients[Data->m_HookedPlayer].m_RenderPos) / 2;
						const vec2 Size = vec2(g_Config.m_DcHookDSize, g_Config.m_DcHookDSize) / 2.0f;

						float angle = (0.5f + 2 * p);
						if(angle > 2)
						{
							angle -= 2;
						}
						angle = M_PI * angle;
						vec2 v = vec2(cosf(angle) * 2, -sinf(angle) * 2);
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
						int NumQuads = 0;
						if(p > 0.5f)
						{
							CircleQuads[NumQuads] = IGraphics::CFreeformItem(-1, -1, 0, -1, -1, 1, 0, 1);
							NumQuads++;
							if(p > 0.75f)
							{
								CircleQuads[NumQuads] = IGraphics::CFreeformItem(v.x, v.y, 1, v.y, 0, 0, 1, 0);
								NumQuads++;
								CircleQuads[NumQuads] = IGraphics::CFreeformItem(0, 0, 1, 0, 0, 1, 1, 1);
								NumQuads++;
							}
							else
							{
								CircleQuads[NumQuads] = IGraphics::CFreeformItem(0, 0, v.x, v.y, 0, 1, v.x, 1);
								NumQuads++;
							}
						}
						else
						{
							if(p > 0.25f)
							{
								CircleQuads[NumQuads] = IGraphics::CFreeformItem(-1, 0, 0, 0, -1, v.y, v.x, v.y);
								NumQuads++;
								CircleQuads[NumQuads] = IGraphics::CFreeformItem(-1, -1, 0, -1, -1, 0, 0, 0);
								NumQuads++;
							}
							else
							{
								CircleQuads[NumQuads] = IGraphics::CFreeformItem(v.x, -1, 0, -1, v.x, v.y, 0, 0);
								NumQuads++;
							}
						}
						for(int i = 0; i < NumQuads; i++)
						{
							Graphics()->QuadsSetSubsetFree(
								(CircleQuads[i].m_X0 + 1) / 2.0f,
								(CircleQuads[i].m_Y0 + 1) / 2.0f,
								(CircleQuads[i].m_X1 + 1) / 2.0f,
								(CircleQuads[i].m_Y1 + 1) / 2.0f,
								(CircleQuads[i].m_X2 + 1) / 2.0f,
								(CircleQuads[i].m_Y2 + 1) / 2.0f,
								(CircleQuads[i].m_X3 + 1) / 2.0f,
								(CircleQuads[i].m_Y3 + 1) / 2.0f);
							CircleQuads[i].m_X0 = CircleQuads[i].m_X0 * Size.x + pos.x;
							CircleQuads[i].m_X1 = CircleQuads[i].m_X1 * Size.x + pos.x;
							CircleQuads[i].m_X2 = CircleQuads[i].m_X2 * Size.x + pos.x;
							CircleQuads[i].m_X3 = CircleQuads[i].m_X3 * Size.x + pos.x;
							CircleQuads[i].m_Y0 = CircleQuads[i].m_Y0 * Size.y + pos.y;
							CircleQuads[i].m_Y1 = CircleQuads[i].m_Y1 * Size.y + pos.y;
							CircleQuads[i].m_Y2 = CircleQuads[i].m_Y2 * Size.y + pos.y;
							CircleQuads[i].m_Y3 = CircleQuads[i].m_Y3 * Size.y + pos.y;
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
	// ScreenY0 -= 0;
	ScreenY1 += 800;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_apPlayerInfos[i];
		if(!pInfo)
		{
			continue;
		}

		if(m_pClient->m_aClients[i].m_SpecCharPresent)
		{
			// Each player can also have a spec char whose nameplate is displayed independently
			const vec2 RenderPos = m_pClient->m_aClients[i].m_SpecChar;
			// don't render offscreen
			if(in_range(RenderPos.x, ScreenX0, ScreenX1) && in_range(RenderPos.y, ScreenY0, ScreenY1))
			{
				RenderNameplate(RenderPos, pInfo, 0.4f, true);
			}
		}
		if(m_pClient->m_Snap.m_aCharacters[i].m_Active)
		{
			// Only render nameplates for active characters
			const vec2 RenderPos = m_pClient->m_aClients[i].m_RenderPos;
			// don't render offscreen
			if(in_range(RenderPos.x, ScreenX0, ScreenX1) && in_range(RenderPos.y, ScreenY0, ScreenY1))
			{
				RenderNameplate(RenderPos, pInfo, 1.0f, false);
			}
		}
	}
}

void CNamePlates::ResetNamePlates()
{
	for(auto &NamePlate : m_aNamePlates)
	{
		TextRender()->DeleteTextContainer(NamePlate.m_NameTextContainerIndex);
		TextRender()->DeleteTextContainer(NamePlate.m_ClanTextContainerIndex);

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
