#pragma once

#include "Defined_Global.h"
#include "Protocol.h"

#if H_RANKINGDMGBOSS

#define H_RANKINGDMGBOSS_MAX 5

#pragma pack(push, 1)
struct H_DMGBOSS_DATA
{
	char Name[11];
	QWORD Damage;
};

struct PMSG_RANKING_DMG_BOSS_RECV
{
	PSWMSG_HEAD header; // C2:D3:2D
	BYTE count;
	WORD ClassMonster;
	QWORD BossCurLife;
	QWORD BossMaxLife;
};
#pragma pack(pop)

class H_DMGBOSS_CLASS
{
public:
	H_DMGBOSS_CLASS();
	void DmgGetInfo(PMSG_RANKING_DMG_BOSS_RECV* lpMsg);
	void DmgDraw();
	void DmgClear();
	float GetRenderBossLifeRate();

private:
	WORD m_ClassMonster;
	int m_Count;
	DWORD m_LastUpdateTick;
	QWORD m_BossCurLife;
	QWORD m_BossMaxLife;
	float m_RenderBossLifeRate;
	DWORD m_RenderBossLifeTick;
	H_DMGBOSS_DATA m_Data[H_RANKINGDMGBOSS_MAX];
};

extern H_DMGBOSS_CLASS gDmgBoss;

#endif
