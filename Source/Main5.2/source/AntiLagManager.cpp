#include "stdafx.h"
#include "AntiLagManager.h"
#include "NewUICommon.h"
#include "ZzzInterface.h"

namespace
{
	bool g_AntiLagHidden[ANTI_LAG_FEATURE_COUNT] = { false };

	const char* GetFeatureName(ANTI_LAG_FEATURE feature)
	{
		switch (feature)
		{
		case ANTI_LAG_HIDE_WINGS:
			return "Hide wings";
		case ANTI_LAG_HIDE_EFFECTS:
			return "Hide effects";
		case ANTI_LAG_HIDE_ITEMS:
			return "Hide ground item & zen";
		case ANTI_LAG_HIDE_PETS:
			return "Hide pets";
		case ANTI_LAG_HIDE_MOUNT:
			return "Hide mount";
		case ANTI_LAG_HIDE_CHARACTER:
			return "Hide character";
		case ANTI_LAG_HIDE_MONSTER:
			return "Hide monster";
		case ANTI_LAG_HIDE_MAP_OBJECT:
			return "Hide map object";
		case ANTI_LAG_HIDE_BACKGROUND:
			return "Hide background";
		default:
			return "";
		}
	}

	void AddNotice(const char* name, bool enabled)
	{
		char text[128];
		sprintf_s(text, "[ANTI-LAG] %s: %s", name, enabled ? "ON" : "OFF");
		CreateNotice(text, 0);
	}

	void SetFeature(ANTI_LAG_FEATURE feature, bool enabled)
	{
		if (feature < 0 || feature >= ANTI_LAG_FEATURE_COUNT)
		{
			return;
		}

		g_AntiLagHidden[feature] = enabled;
		AddNotice(GetFeatureName(feature), enabled);
	}

	void ToggleFeature(ANTI_LAG_FEATURE feature)
	{
		SetFeature(feature, !AntiLag_IsHidden(feature));
	}

	void ToggleAll()
	{
		bool enabled = false;

		for (int i = 0; i < ANTI_LAG_FEATURE_COUNT; ++i)
		{
			if (!g_AntiLagHidden[i])
			{
				enabled = true;
				break;
			}
		}

		for (int i = 0; i < ANTI_LAG_FEATURE_COUNT; ++i)
		{
			g_AntiLagHidden[i] = enabled;
		}

		AddNotice("Hide all", enabled);
	}

	bool IsShiftDown()
	{
		return (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	}
}

bool AntiLag_IsHidden(ANTI_LAG_FEATURE feature)
{
	if (feature < 0 || feature >= ANTI_LAG_FEATURE_COUNT)
	{
		return false;
	}

	return g_AntiLagHidden[feature];
}

void AntiLag_UpdateHotkeys()
{
	if (!IsShiftDown())
	{
		return;
	}

	if (SEASON3B::IsPress('1'))
	{
		ToggleFeature(ANTI_LAG_HIDE_WINGS);
	}
	else if (SEASON3B::IsPress('2'))
	{
		ToggleFeature(ANTI_LAG_HIDE_EFFECTS);
	}
	else if (SEASON3B::IsPress('3'))
	{
		ToggleFeature(ANTI_LAG_HIDE_ITEMS);
	}
	else if (SEASON3B::IsPress('4'))
	{
		ToggleFeature(ANTI_LAG_HIDE_PETS);
	}
	else if (SEASON3B::IsPress('5'))
	{
		ToggleFeature(ANTI_LAG_HIDE_MOUNT);
	}
	else if (SEASON3B::IsPress('6'))
	{
		ToggleFeature(ANTI_LAG_HIDE_CHARACTER);
	}
	else if (SEASON3B::IsPress('7'))
	{
		ToggleFeature(ANTI_LAG_HIDE_MONSTER);
	}
	else if (SEASON3B::IsPress('8'))
	{
		ToggleFeature(ANTI_LAG_HIDE_MAP_OBJECT);
	}
	else if (SEASON3B::IsPress('9'))
	{
		ToggleFeature(ANTI_LAG_HIDE_BACKGROUND);
	}
	else if (SEASON3B::IsPress('0'))
	{
		ToggleAll();
	}
}