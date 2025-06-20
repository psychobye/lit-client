#include "../main.h"
#include "../gui/gui.h"
#include "../game/game.h"
#include "../chatwindow.h"
#include "netgame.h"
#include "vehiclepool.h"

extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;
extern CGUI *pGUI;

void TextWithColors(ImVec2 pos, ImColor col, const char* szStr, const char* szStrWithoutColors);

void CText3DLabelsPool::DrawNonAttached(TEXT_LABELS* pLabel)
{
	if (!pLabel)
	{
		return;
	}
	if (!pNetGame)
	{
		return;
	}
	if (!pNetGame->GetPlayerPool())
	{
		return;
	}
	VECTOR pos;
	memcpy((void*)&pos, (const void*)&(pLabel->pos), sizeof(VECTOR));
	DrawTextLabel(pLabel, &pos);
}

void CText3DLabelsPool::DrawAttachedToPlayer(TEXT_LABELS* pLabel)
{
	if (!pLabel)
	{
		return;
	}
	VECTOR pos;
	memset((void*)&pos, 0, sizeof(VECTOR));

	PLAYERID playerId = pLabel->attachedToPlayerID;

	CPlayerPool* pPlayerPool = nullptr;
	if (pNetGame)
	{
		pPlayerPool = pNetGame->GetPlayerPool();
	}
	if (!pPlayerPool)
	{
		return;
	}

	if (!pPlayerPool->GetSlotState(playerId))
	{
		return;
	}

	CRemotePlayer* pPlayer = pPlayerPool->GetAt(playerId);
	if (!pPlayer)
	{
		return;
	}
	if (!pPlayer->GetPlayerPed())
	{
		return;
	}
	CPlayerPed* pPlayerPed = pPlayer->GetPlayerPed();
	if (!pPlayerPed->IsAdded())
	{
		return;
	}
	VECTOR boneOut;
	pPlayerPed->GetBonePosition(5, &boneOut);

	memcpy((void*)&pos, (const void*)&boneOut, sizeof(VECTOR));

	pos.X += pLabel->offsetCoords.X;
	pos.Y += pLabel->offsetCoords.Y;
	pos.Z += pLabel->offsetCoords.Z;

	DrawTextLabel(pLabel, &pos);
}

void CText3DLabelsPool::DrawAttachedToVehicle(TEXT_LABELS* pLabel)
{
	if (!pLabel)
	{
		return;
	}
	VECTOR pos;
	memset((void*)&pos, 0, sizeof(VECTOR));

	VEHICLEID vehId = pLabel->attachedToVehicleID;

	CVehiclePool* pVehiclePool = nullptr;
	if (pNetGame)
	{
		pVehiclePool = pNetGame->GetVehiclePool();
	}
	if (!pVehiclePool)
	{
		return;
	}

	if (!pVehiclePool->GetSlotState(vehId))
	{
		return;
	}

	CVehicle* pVehicle = pVehiclePool->GetAt(vehId);
	if (!pVehicle)
	{
		return;
	}
	if (!pVehicle->IsAdded())
	{
		return;
	}
	MATRIX4X4 mat;
	memset((void*)& mat, 0, sizeof(MATRIX4X4));

	pVehicle->GetMatrix(&mat);

	memcpy((void*)& pos, (const void*)& mat.pos, sizeof(VECTOR));

	pos.X += pLabel->offsetCoords.X;
	pos.Y += pLabel->offsetCoords.Y;
	pos.Z += pLabel->offsetCoords.Z;

	DrawTextLabel(pLabel, &pos);
}

void CText3DLabelsPool::DrawVehiclesInfo() {
	char inf[255];
	CVehiclePool* pVehiclePool = nullptr;
	if (pNetGame) {
		pVehiclePool = pNetGame->GetVehiclePool();
	}
	if (!pVehiclePool) {
		return;
	}
	for (int i = 0; i < MAX_VEHICLES; i++) {
		CVehicle* pVehicle = pVehiclePool->GetAt(i);
		if (pVehicle) {
			int distance = pVehicle->GetDistanceFromLocalPlayerPed();
			if (distance < 20) {
				VECTOR pos;
				memset((void*)&pos, 0, sizeof(VECTOR));

				MATRIX4X4 mat;
				memset((void*)&mat, 0, sizeof(MATRIX4X4));

				pVehicle->GetMatrix(&mat);

				memcpy((void*)&pos, (const void*)&mat.pos, sizeof(VECTOR));

				VECTOR Out;
				memset((void*)&Out, 0, sizeof(VECTOR));

				((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(g_libGTASA + 0x54EEC0 + 1))(&pos, &Out, 0, 0, 0, 0);
				if (Out.Z < 1.0f) {
					return;
				}

				ImVec2 vpos = ImVec2(Out.X, Out.Y);

				sprintf(inf, "Network ID: %d\nModel Index: %d\nHealth: %.2f\nDistance: %dm\nInvulnerable: %s",
					i,
					pVehicle->GetModelIndex(),
					pVehicle->GetHealth(),
					distance,
					pVehicle->m_bIsInvulnerable ? "Yes" : "No");

				TextWithColors(vpos, __builtin_bswap32(0xFFFFFFFF), inf, inf);
			}
		}
	}
}

#include "..//CDebugInfo.h"
float EntityGetDistanceFromCamera(ENTITY_TYPE* m_pEntity)
{
	if (!m_pEntity || m_pEntity->vtable == SA_ADDR(0x5C7358))
		return 100000.0f;

	float tmpX = (m_pEntity->mat->pos.X - *(float*)(SA_ADDR(0x8B1134)));
	float tmpY = (m_pEntity->mat->pos.Y - *(float*)(SA_ADDR(0x8B1138)));
	float tmpZ = (m_pEntity->mat->pos.Z - *(float*)(SA_ADDR(0x8B113C)));

	return sqrt(tmpX * tmpX + tmpY * tmpY + tmpZ * tmpZ);
}

#include <algorithm>
#include <string>
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return str;
}

void CText3DLabelsPool::DrawModelsInfo(uintptr_t pEntity) {
	ENTITY_TYPE* sEntity = (ENTITY_TYPE*)pEntity;
	if (!sEntity->mat) {
		return;
	}

	char inf[255];

	uint16_t usEntityID = *(uint16_t*)(pEntity + 34);
	uintptr_t* pTexturesInfo = (uintptr_t*)(g_libGTASA + 0x6706E4);
	uintptr_t pTextureInfo = (uintptr_t)pTexturesInfo[usEntityID];

	if (pTextureInfo)
	{
		char szTextureBufferInfo[0xFF] = { '\0' };
		char szTextureBuffer[0xFF] = { '\0' };

		float dff_size = (float)((float)((float)(pTextureInfo << 11) * 0.00097656) * 0.00097656);
		float tex_size = (float)((float)((float)pTextureInfo * 0.00097656) * 0.00097656);

		sprintf(szTextureBufferInfo, "DFF: %f MB\nTextures: %f MB | Network ID: %d\nPosition: %.2f, %.2f, %.2f\nSpeed: %.2f", dff_size, tex_size, usEntityID, sEntity->mat->pos.X, sEntity->mat->pos.Y, sEntity->mat->pos.Z,
			sqrt((sEntity->vecMoveSpeed.X * sEntity->vecMoveSpeed.X) + (sEntity->vecMoveSpeed.Y * sEntity->vecMoveSpeed.Y) + (sEntity->vecMoveSpeed.Z * sEntity->vecMoveSpeed.Z)) * 180);

		std::string s(szTextureBufferInfo);
		std::string from("4095");
		std::string to("4");
		std::string repl = ReplaceAll(s, from, to);
		strcpy(szTextureBufferInfo, repl.c_str());

		VECTOR TagPos;
		TagPos.X = sEntity->mat->pos.X;
		TagPos.Y = sEntity->mat->pos.Y;
		TagPos.Z = sEntity->mat->pos.Z;
		TagPos.Z += 0.25f + (EntityGetDistanceFromCamera(sEntity) * 0.0475f);

		VECTOR Out;

		// CSprite::CalcScreenCoors
		((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(SA_ADDR(0x54EEC0 + 1)))(&TagPos, &Out, 0, 0, 0, 0);

		if (Out.Z < 1.0f) {
			return;
		}

		ImVec2 pos = ImVec2(Out.X, Out.Y);
		//RenderText(pos, 0xFFFFFFFF, true, szTextureBufferInfo);
		TextWithColors(pos, __builtin_bswap32(0xFFFFFFFF), szTextureBufferInfo, szTextureBufferInfo);
	}
}

void CText3DLabelsPool::DrawTextLabel(TEXT_LABELS* pLabel, VECTOR* pPos)
{
	int hitEntity = 0;
	if (!pNetGame->GetPlayerPool()->GetLocalPlayer())
	{
		return;
	}
	CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	if (!pPed)
	{
		return;
	}
	if (!pPed->IsAdded())
	{
		return;
	}

	if (pLabel->useLineOfSight)
	{
		CAMERA_AIM* pCam = GameGetInternalAim();
		if (!pCam)
		{
			return;
		}

		hitEntity = ScriptCommand(&get_line_of_sight,
			pPos->X, pPos->Y, pPos->Z,
			pCam->pos1x, pCam->pos1y, pCam->pos1z,
			1, 0, 0, 0, 0);
	}

	if (!pLabel->useLineOfSight || hitEntity)
	{
		if (pPed->GetDistanceFromPoint(pPos->X, pPos->Y, pPos->Z) <= pLabel->drawDistance)
		{
			VECTOR Out;
			memset((void*)&Out, 0, sizeof(VECTOR));
			// CSprite::CalcScreenCoors
			((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(g_libGTASA + 0x54EEC0 + 1))(pPos, &Out, 0, 0, 0, 0);
			if (Out.Z < 1.0f)
			{
				return;
			}

			ImVec2 pos = ImVec2(Out.X, Out.Y);

			if (pLabel->m_fTrueX < 0.0f)
			{
				char* curBegin = pLabel->textWithoutColors;
				char* curPos = pLabel->textWithoutColors;
				while (*curPos != '\0')
				{
					if (*curPos == '\n')
					{
						float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
						if (width > pLabel->m_fTrueX)
						{
							pLabel->m_fTrueX = width;
						}

						curBegin = curPos + 1;
					}

					curPos++;
				}

				if (pLabel->m_fTrueX < 0.0f)
				{
					pLabel->m_fTrueX = ImGui::CalcTextSize(pLabel->textWithoutColors).x;
				}

			}

			//pos.x -= (pLabel->m_fTrueX / 2.0f);

			TextWithColors(pos, __builtin_bswap32(pLabel->color), pLabel->text, pLabel->textWithoutColors);
		}
	}
}

CText3DLabelsPool::CText3DLabelsPool()
{
	for(int i = 0; i < TEXT_LABEL_POOL_SIZE; i++)
	{
		m_pTextLabels[i] = nullptr;
		m_bSlotState[i] = false;
	}
}

CText3DLabelsPool::~CText3DLabelsPool()
{
	for (int i = 0; i < TEXT_LABEL_POOL_SIZE; i++)
	{
		if (m_pTextLabels[i] && m_bSlotState[i])
		{
			m_bSlotState[i] = false;

			m_pTextLabels[i]->text[0] = '\0';

			delete m_pTextLabels[i];
			m_pTextLabels[i] = nullptr;
		}
	}
}

void FilterColors(char* szStr)
{
	if(!szStr) return;

	char szNonColored[2048+1];
	int iNonColoredMsgLen = 0;

	for(int pos = 0; pos < strlen(szStr) && szStr[pos] != '\0'; pos++)
	{
		if(pos+7 < strlen(szStr))
		{
			if(szStr[pos] == '{' && szStr[pos+7] == '}')
			{
				pos += 7;
				continue;
			}
		}

		szNonColored[iNonColoredMsgLen] = szStr[pos];
		iNonColoredMsgLen++;
	}

	szNonColored[iNonColoredMsgLen] = 0;
	strcpy(szStr, szNonColored);
}

void CText3DLabelsPool::CreateTextLabel(int labelID, char* text, uint32_t color,
                                        float posX, float posY, float posZ, float drawDistance, bool useLOS,
                                        PLAYERID attachedToPlayerID, VEHICLEID attachedToVehicleID)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE) {
        return;
    }

    if (m_pTextLabels[labelID]) {
        delete m_pTextLabels[labelID];
        m_pTextLabels[labelID] = nullptr;
        m_bSlotState[labelID] = false;
    }
    auto* pTextLabel = new TEXT_LABELS;
    Log("CREATE LABEL");

    if (pTextLabel) {
        cp1251_to_utf8(pTextLabel->text, text);
        cp1251_to_utf8(pTextLabel->textWithoutColors, text);
        FilterColors(pTextLabel->textWithoutColors);

        pTextLabel->color = color;
        pTextLabel->pos = { posX, posY, posZ };
        pTextLabel->drawDistance = drawDistance;
        pTextLabel->useLineOfSight = useLOS;
        pTextLabel->attachedToPlayerID = attachedToPlayerID;
        pTextLabel->attachedToVehicleID = attachedToVehicleID;

        pTextLabel->m_fTrueX = -1.0f;

        if (attachedToVehicleID != INVALID_VEHICLE_ID || attachedToPlayerID != INVALID_PLAYER_ID) {
            pTextLabel->offsetCoords = { posX, posY, posZ };
        }

        m_pTextLabels[labelID] = pTextLabel;
        m_bSlotState[labelID] = true;
    }
}

void CText3DLabelsPool::Delete(int labelID)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
    {
        return;
    }

    Log("DELETE LABEL");

    if (m_pTextLabels[labelID])
    {
        delete m_pTextLabels[labelID];
        m_pTextLabels[labelID] = nullptr;
    }

    m_bSlotState[labelID] = false;
}

void CText3DLabelsPool::AttachToPlayer(int labelID, PLAYERID playerID, VECTOR pos)
{
	if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
	{
		return;
	}
	if (m_bSlotState[labelID] == true && m_pTextLabels[labelID])
	{
		//tempPlayerID = playerID;
		m_pTextLabels[labelID]->attachedToPlayerID = playerID;
		m_pTextLabels[labelID]->pos = pos;
		m_pTextLabels[labelID]->offsetCoords = pos;
	}
}

void CText3DLabelsPool::AttachToVehicle(int labelID, VEHICLEID vehicleID, VECTOR pos)
{
	if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
	{
		return;
	}
	if (m_bSlotState[labelID] == true && m_pTextLabels[labelID])
	{
		m_pTextLabels[labelID]->attachedToVehicleID = vehicleID;
		m_pTextLabels[labelID]->pos = pos;
		m_pTextLabels[labelID]->offsetCoords = pos;
	}
}

void CText3DLabelsPool::Update3DLabel(int labelID, uint32_t color, char* text)
{
	if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
	{
		return;
	}
	Log("UPDATE LABEL");
	if (m_bSlotState[labelID] == true && m_pTextLabels[labelID])
	{
		m_pTextLabels[labelID]->color = color;
		//m_pTextLabels[labelID]->text = text;
		cp1251_to_utf8(m_pTextLabels[labelID]->text, text);
	}
}

bool ProcessInlineHexColor(const char* start, const char* end, ImVec4& color);

void TextWithColors(ImVec2 pos, ImColor col, const char* szStr, const char* szStrWithoutColors = nullptr)
{
	if (pNetGame)
	{
		if (pNetGame->GetPlayerPool())
		{
			if (pNetGame->GetPlayerPool()->GetLocalPlayer())
			{
				CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
				if (pPlayer->GetPlayerPed())
				{
					if (pPlayer->GetPlayerPed()->GetActionTrigger() == ACTION_DEATH || pPlayer->GetPlayerPed()->IsDead())
					{
						return;
					}

					
				}
			}
		}
	}
	float fLineOffsets[100];

	int iCounter = 0;
	if (szStrWithoutColors)
	{
		const char* curBegin = szStrWithoutColors;
		const char* curPos = szStrWithoutColors;
		while (*curPos != '\0')
		{
			if (*curPos == '\n')
			{
				float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
				width /= 2.0f;

				fLineOffsets[iCounter] = width;
				iCounter++;

				curBegin = curPos + 1;
			}
			curPos++;
			if (*curPos == '\0')
			{
				float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
				width /= 2.0f;

				fLineOffsets[iCounter] = width;
				iCounter++;
			}
		}
	}
	
	iCounter = 0;
	char tempStr[4096];

	//va_list argPtr;
	//va_start(argPtr, fmt);
	//vsnprintf(tempStr, sizeof(tempStr), fmt, argPtr);
	//va_end(argPtr);

	ImVec2 vecPos = pos;

	strcpy(tempStr, szStr);
	tempStr[sizeof(tempStr) - 1] = '\0';

	bool pushedColorStyle = false;
	const char* textStart = tempStr;
	const char* textCur = tempStr;

	if (szStrWithoutColors)
	{
		vecPos.x -= fLineOffsets[0];
	}

	//vecPos.x -= fLineOffsets[0];
	while(textCur < (tempStr + sizeof(tempStr)) && *textCur != '\0')
	{
		if (*textCur == '{')
		{
			// Print accumulated text
			if (textCur != textStart)
			{
				//ImGui::TextUnformatted(textStart, textCur);
				pGUI->RenderText(vecPos, col, true, textStart, textCur);
				//ImGui::SameLine(0.0f, 0.0f);
				vecPos.x += ImGui::CalcTextSize(textStart, textCur).x;
			}

			// Process color code
			const char* colorStart = textCur + 1;
			do
			{
				++textCur;
			} while (*textCur != '\0' && *textCur != '}');

			// Change color
			if (pushedColorStyle)
			{
				//ImGui::PopStyleColor();
				pushedColorStyle = false;
			}

			ImVec4 textColor;
			if (ProcessInlineHexColor(colorStart, textCur, textColor))
			{
				//ImGui::PushStyleColor(ImGuiCol_Text, textColor);
				col = textColor;
				pushedColorStyle = true;
			}

			textStart = textCur + 1;
		}
		else if (*textCur == '\n')
		{
			// Print accumulated text an go to next line
			//ImGui::TextUnformatted(textStart, textCur);
			pGUI->RenderText(vecPos, col, true, textStart, textCur);
			iCounter++;

			vecPos.x = pos.x;//+= ImGui::CalcTextSize(textStart, textCur).x;
			if (szStrWithoutColors)
			{
				vecPos.x -= fLineOffsets[iCounter];
			}

			vecPos.y += pGUI->GetFontSize();
			textStart = textCur + 1;
		}

		++textCur;
	}

	if (textCur != textStart)
		//ImGui::TextUnformatted(textStart, textCur);
	{
		pGUI->RenderText(vecPos, col, true, textStart, textCur);
		vecPos.x += ImGui::CalcTextSize(textStart, textCur).x;
	}
	else
		//ImGui::NewLine();
		vecPos.y += pGUI->GetFontSize();

	//if(pushedColorStyle)
	//	ImGui::PopStyleColor();
}

#include "..//game/game.h"
extern CGame* pGame;
extern CGUI* pGUI;
void CText3DLabelsPool::Draw()
{
	if (pGame->m_bGui_enabled)
	{
		DrawVehiclesInfo();

		/*uint32_t ms_nNoOfVisibleEntities = *(uint32_t*)(g_libGTASA + 0x8C162C);
		uintptr_t* ms_aVisibleEntityPtrs = (uintptr_t*)(g_libGTASA + 0x8C0680);

		if (!ms_nNoOfVisibleEntities) {
			return;
		}

		do {
			uintptr_t m_pEntityPointer = (uintptr_t)ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities];
			if (m_pEntityPointer) {
				DrawModelsInfo(m_pEntityPointer);
			}
			--ms_nNoOfVisibleEntities;
		} while (ms_nNoOfVisibleEntities);*/
	}
	for (int i = 0; i < TEXT_LABEL_POOL_SIZE; i++)
	{
		if (!m_pTextLabels[i] || !m_bSlotState[i])
		{
			continue;
		}
		TEXT_LABELS* pLabel = m_pTextLabels[i];
		if (!pNetGame->GetPlayerPool())
		{
			continue;
		}
		if (pLabel->attachedToPlayerID != INVALID_PLAYER_ID)
		{
			if (pLabel->attachedToPlayerID == pNetGame->GetPlayerPool()->GetLocalPlayerID())
			{
				continue;
			}
			DrawAttachedToPlayer(pLabel);
			continue;
		}
		else if (pLabel->attachedToVehicleID != INVALID_VEHICLE_ID)
		{
			DrawAttachedToVehicle(pLabel);
			continue;
		}
		else
		{
			DrawNonAttached(pLabel);
			continue;
		}

	}
}