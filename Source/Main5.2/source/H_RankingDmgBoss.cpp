#include "StdAfx.h"
#include "H_RankingDmgBoss.h"
#include "CBInterface.h"
#include "ZzzInfomation.h"
#include "TextClient.h"

#if H_RANKINGDMGBOSS

namespace
{
	const DWORD BOSS_LIFE_SMOOTH_MAX_ELAPSED = 50;
	const float BOSS_LIFE_SMOOTH_SPEED = 2.5f;

	bool IsBlockedByWindow()
	{
		return (gInterface->CheckWindow(CB_Interface::MoveList) ||
			gInterface->CheckWindow(CB_Interface::CreateGuild) ||
			gInterface->CheckWindow(CB_Interface::BloodCastle) ||
			gInterface->CheckWindow(CB_Interface::DevilSquare) ||
			gInterface->CheckWindow(CB_Interface::Character) ||
			gInterface->CheckWindow(CB_Interface::FastMenu) ||
			gInterface->CheckWindow(CB_Interface::CashShop) ||
			gInterface->CheckWindow(CB_Interface::SkillTree) ||
			gInterface->CheckWindow(CB_Interface::FullMap) ||
			gInterface->CheckWindow(CB_Interface::Inventory));
	}

	DWORD GetRankingTextColor(int index)
	{
		switch (index)
		{
		case 0:
			return eGold;
		case 1:
			return eBlue;
		case 2:
			return eOrange;
		default:
			return eWhite;
		}
	}

	void FormatBossLife(QWORD value, char* output, int size)
	{
		if (output == 0 || size <= 0)
		{
			return;
		}

		char reversed[64];
		int length = 0;
		int group = 0;

		do
		{
			if (group == 3 && length < (sizeof(reversed) - 1))
			{
				reversed[length++] = '.';
				group = 0;
			}

			if (length >= (sizeof(reversed) - 1))
			{
				break;
			}

			reversed[length++] = char('0' + (value % 10));
			value /= 10;
			group++;
		} while (value > 0);

		int outLength = ((length < (size - 1)) ? length : (size - 1));

		for (int n = 0; n < outLength; n++)
		{
			output[n] = reversed[(length - 1) - n];
		}

		output[outLength] = 0;
	}

	int GetBossLifePercent(QWORD bossCurLife, QWORD bossMaxLife)
	{
		if (bossMaxLife == 0)
		{
			return 0;
		}

		int percent = (int)(((double)bossCurLife * 100.0) / (double)bossMaxLife);

		if (percent < 0)
		{
			percent = 0;
		}
		else if (percent > 100)
		{
			percent = 100;
		}

		return percent;
	}

	void FormatDamagePercent(QWORD damage, QWORD bossMaxLife, char* output, int size)
	{
		if (output == 0 || size <= 0)
		{
			return;
		}

		if (bossMaxLife == 0)
		{
			sprintf_s(output, size, "0.00%%");
			return;
		}

		double percent = (((double)damage * 100.0) / (double)bossMaxLife);

		if (percent < 0.0)
		{
			percent = 0.0;
		}

		sprintf_s(output, size, "%.2f%%", percent);
	}

	float GetBossLifeRate(QWORD bossCurLife, QWORD bossMaxLife)
	{
		if (bossMaxLife == 0)
		{
			return 0.0f;
		}

		float rate = (float)((double)bossCurLife / (double)bossMaxLife);

		if (rate < 0.0f)
		{
			rate = 0.0f;
		}
		else if (rate > 1.0f)
		{
			rate = 1.0f;
		}

		return rate;
	}

	int GetBossLifeColorIndex(float bossLifeRate)
	{
		int colorIndex = (int)((bossLifeRate * 10.0f) - 0.0001f);

		if (colorIndex < 0)
		{
			colorIndex = 0;
		}
		else if (colorIndex > 9)
		{
			colorIndex = 9;
		}

		return colorIndex;
	}

	float ClampColorValue(float value)
	{
		if (value < 0.0f)
		{
			return 0.0f;
		}

		if (value > 1.0f)
		{
			return 1.0f;
		}

		return value;
	}

	void GetBossLifeSegmentColor(int segment, float& red, float& green, float& blue)
	{
		switch (segment)
		{
		case 0:
			red = 0.85f; green = 0.08f; blue = 0.08f;
			break;
		case 1:
			red = 0.92f; green = 0.20f; blue = 0.10f;
			break;
		case 2:
			red = 0.95f; green = 0.38f; blue = 0.08f;
			break;
		case 3:
			red = 0.96f; green = 0.55f; blue = 0.08f;
			break;
		case 4:
			red = 0.95f; green = 0.72f; blue = 0.05f;
			break;
		case 5:
			red = 0.90f; green = 0.86f; blue = 0.10f;
			break;
		case 6:
			red = 0.58f; green = 0.86f; blue = 0.14f;
			break;
		case 7:
			red = 0.20f; green = 0.82f; blue = 0.20f;
			break;
		case 8:
			red = 0.10f; green = 0.82f; blue = 0.58f;
			break;
		default:
			red = 0.12f; green = 0.72f; blue = 0.95f;
			break;
		}
	}

	void GetBossLifeSegmentState(float bossLifeRate, int& colorIndex, int& nextColorIndex, bool& hasNextColor, float& segmentRate)
	{
		float scaledLifeRate = (bossLifeRate * 10.0f);

		colorIndex = GetBossLifeColorIndex(bossLifeRate);

		hasNextColor = (colorIndex > 0);
		nextColorIndex = ((colorIndex > 0) ? (colorIndex - 1) : 0);

		if (bossLifeRate >= 1.0f)
		{
			segmentRate = 1.0f;
		}
		else
		{
			segmentRate = (scaledLifeRate - (float)colorIndex);
		}

		if (segmentRate < 0.0f)
		{
			segmentRate = 0.0f;
		}
		else if (segmentRate > 1.0f)
		{
			segmentRate = 1.0f;
		}
	}
}

H_DMGBOSS_CLASS gDmgBoss;

H_DMGBOSS_CLASS::H_DMGBOSS_CLASS()
{
	this->DmgClear();
}

void H_DMGBOSS_CLASS::DmgGetInfo(PMSG_RANKING_DMG_BOSS_RECV* lpMsg)
{
	if (lpMsg == 0)
	{
		return;
	}

	if (lpMsg->count == 0)
	{
		gInterface->Data[eTopDame].Close();
		this->DmgClear();
		return;
	}

	bool resetRenderLife = (this->m_ClassMonster != lpMsg->ClassMonster || this->m_BossMaxLife != lpMsg->BossMaxLife);

	this->m_Count = 0;
	memset(this->m_Data, 0, sizeof(this->m_Data));
	this->m_ClassMonster = lpMsg->ClassMonster;
	this->m_BossCurLife = lpMsg->BossCurLife;
	this->m_BossMaxLife = lpMsg->BossMaxLife;
	this->m_LastUpdateTick = GetTickCount();

	float bossLifeRate = GetBossLifeRate(this->m_BossCurLife, this->m_BossMaxLife);

	if (resetRenderLife || this->m_RenderBossLifeTick == 0)
	{
		this->m_RenderBossLifeRate = bossLifeRate;
	}
	else if (this->m_RenderBossLifeRate < bossLifeRate)
	{
		this->m_RenderBossLifeRate = bossLifeRate;
	}

	int count = ((lpMsg->count > H_RANKINGDMGBOSS_MAX) ? H_RANKINGDMGBOSS_MAX : lpMsg->count);

	for (int n = 0; n < count; n++)
	{
		H_DMGBOSS_DATA* lpInfo = (H_DMGBOSS_DATA*)(((BYTE*)lpMsg) + sizeof(PMSG_RANKING_DMG_BOSS_RECV) + (sizeof(H_DMGBOSS_DATA) * n));

		memcpy(this->m_Data[n].Name, lpInfo->Name, sizeof(this->m_Data[n].Name));
		this->m_Data[n].Name[(sizeof(this->m_Data[n].Name) - 1)] = 0;
		this->m_Data[n].Damage = lpInfo->Damage;
		this->m_Count++;
	}

	if (this->m_Count > 0)
	{
		gInterface->Data[eTopDame].Open();
	}
}

float H_DMGBOSS_CLASS::GetRenderBossLifeRate()
{
	float targetRate = GetBossLifeRate(this->m_BossCurLife, this->m_BossMaxLife);
	DWORD tick = GetTickCount();

	if (this->m_RenderBossLifeTick == 0)
	{
		this->m_RenderBossLifeTick = tick;
		this->m_RenderBossLifeRate = targetRate;
		return this->m_RenderBossLifeRate;
	}

	DWORD elapsed = (tick - this->m_RenderBossLifeTick);

	if (elapsed > BOSS_LIFE_SMOOTH_MAX_ELAPSED)
	{
		elapsed = BOSS_LIFE_SMOOTH_MAX_ELAPSED;
	}

	this->m_RenderBossLifeTick = tick;

	if (this->m_RenderBossLifeRate > targetRate)
	{
		float smoothFactor = ((float)elapsed / 1000.0f) * BOSS_LIFE_SMOOTH_SPEED;

		if (smoothFactor > 1.0f)
		{
			smoothFactor = 1.0f;
		}

		this->m_RenderBossLifeRate += ((targetRate - this->m_RenderBossLifeRate) * smoothFactor);

		if ((this->m_RenderBossLifeRate - targetRate) < 0.001f)
		{
			this->m_RenderBossLifeRate = targetRate;
		}
	}
	else
	{
		this->m_RenderBossLifeRate = targetRate;
	}

	return this->m_RenderBossLifeRate;
}

void H_DMGBOSS_CLASS::DmgDraw()
{
	if (!gInterface->Data[eTopDame].OnShow)
	{
		return;
	}

	if (this->m_Count <= 0 || (GetTickCount() - this->m_LastUpdateTick) > 5000)
	{
		gInterface->Data[eTopDame].Close();
		this->DmgClear();
		return;
	}

	if (IsBlockedByWindow())
	{
		return;
	}

	float windowWidth = 168.0f;
	float windowHeight = (float)(55 + (this->m_Count * 12));
	float startX = (MAX_WIN_WIDTH - windowWidth) - 472.0f;
	float startY = (MAX_WIN_HEIGHT - 305.0f);
	char bossLifeText[64];
	char bossTitle[128];
	int bossLifePercent = 0;

	const char* monsterName = ((this->m_ClassMonster == ((WORD)-1)) ? "Boss" : getMonsterName(this->m_ClassMonster));

	if (monsterName == 0 || monsterName[0] == 0)
	{
		monsterName = "Boss";
	}

	FormatBossLife(this->m_BossCurLife, bossLifeText, sizeof(bossLifeText));
	bossLifePercent = GetBossLifePercent(this->m_BossCurLife, this->m_BossMaxLife);
	sprintf_s(bossTitle, sizeof(bossTitle), "%s - %s HP (%d%%)", monsterName, bossLifeText, bossLifePercent);

	if (!gInterface->gDrawWindowCustom(&startX, &startY, windowWidth, windowHeight, eTopDame, "%s", bossTitle))
	{
		return;
	}

	float drawX = gInterface->Data[eTopDame].X;
	float drawY = gInterface->Data[eTopDame].Y;
	float lifeBarX = drawX + 10.0f;
	float lifeBarY = drawY + 1.0f;
	float lifeBarWidth = windowWidth - 20.0f;
	float lifeBarHeight = 20.0f;
	float bossLifeRate = this->GetRenderBossLifeRate();

	gInterface->DrawBarForm(lifeBarX, lifeBarY, lifeBarWidth, lifeBarHeight, 0.12f, 0.12f, 0.12f, 0.95f);
	gInterface->DrawBarForm(lifeBarX + 1.0f, lifeBarY + 1.0f, lifeBarWidth - 2.0f, lifeBarHeight - 2.0f, 0.02f, 0.02f, 0.02f, 0.80f);

	if (bossLifeRate > 0.0f)
	{
		float innerBarX = (lifeBarX + 1.0f);
		float innerBarY = (lifeBarY + 1.0f);
		float innerBarWidth = (lifeBarWidth - 2.0f);
		float innerBarHeight = (lifeBarHeight - 2.0f);
		int colorIndex = 0;
		int nextColorIndex = 0;
		bool hasNextColor = false;
		float segmentRate = 0.0f;
		float filledWidth = 0.0f;
		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;
		float nextRed = 0.0f;
		float nextGreen = 0.0f;
		float nextBlue = 0.0f;

		GetBossLifeSegmentState(bossLifeRate, colorIndex, nextColorIndex, hasNextColor, segmentRate);

		filledWidth = (innerBarWidth * segmentRate);

		GetBossLifeSegmentColor(colorIndex, red, green, blue);
		if (hasNextColor != 0)
		{
			GetBossLifeSegmentColor(nextColorIndex, nextRed, nextGreen, nextBlue);

			gInterface->DrawBarForm(innerBarX, innerBarY, innerBarWidth, innerBarHeight, nextRed, nextGreen, nextBlue, 0.48f);
			gInterface->DrawBarForm(innerBarX, innerBarY, innerBarWidth, (innerBarHeight * 0.45f), ClampColorValue(nextRed + 0.12f), ClampColorValue(nextGreen + 0.12f), ClampColorValue(nextBlue + 0.12f), 0.18f);
		}

		gInterface->DrawBarForm(innerBarX, innerBarY, filledWidth, innerBarHeight, red, green, blue, 0.78f);
		gInterface->DrawBarForm(innerBarX, innerBarY, filledWidth, (innerBarHeight * 0.45f), ClampColorValue(red + 0.15f), ClampColorValue(green + 0.15f), ClampColorValue(blue + 0.15f), 0.28f);
	}

	gInterface->DrawFormat(eGold, (int)(drawX + 16), (int)(drawY + 27), 18, 1, "#");
	gInterface->DrawFormat(eGold, (int)(drawX + 34), (int)(drawY + 27), 88, 1, gTextClient.txtClient_Ranking[6]);
	gInterface->DrawFormat(eGold, (int)(drawX + 95), (int)(drawY + 27), 60, 4, gTextClient.txtClient_Ranking[9]);

	for (int n = 0; n < this->m_Count; n++)
	{
		DWORD textColor = GetRankingTextColor(n);
		char damageText[32];

		FormatDamagePercent(this->m_Data[n].Damage, this->m_BossMaxLife, damageText, sizeof(damageText));

		gInterface->DrawFormat(textColor, (int)(drawX + 16), (int)(drawY + 42 + (n * 12)), 18, 1, "%d", (n + 1));
		gInterface->DrawFormat(textColor, (int)(drawX + 34), (int)(drawY + 42 + (n * 12)), 88, 1, "%s", this->m_Data[n].Name);
		gInterface->DrawFormat(textColor, (int)(drawX + 95), (int)(drawY + 42 + (n * 12)), 60, 4, "%s", damageText);
	}
}

void H_DMGBOSS_CLASS::DmgClear()
{
	this->m_ClassMonster = (WORD)-1;
	this->m_Count = 0;
	this->m_LastUpdateTick = 0;
	this->m_BossCurLife = 0;
	this->m_BossMaxLife = 0;
	this->m_RenderBossLifeRate = 0.0f;
	this->m_RenderBossLifeTick = 0;
	memset(this->m_Data, 0, sizeof(this->m_Data));
}

#endif
