
#pragma once

#include "User.h"
#include <fstream>

#if FAKE_ONLINE == TRUE

struct OFFEXP_DATA
{
	char Account[11];
	char Password[11];
	char Name[11];
	WORD SkillID;
	int GateNumber;
	int MapX;
	int MapY;
	int PhamViTrain;
	int MoveRange;
	int TimeReturn;
	int TuNhatItem;
	int TuDongReset;
	int PVPMode;
	int PartyMode;
	int ReturnAfterDie;
	int ReturnAfterDieTimeMin;
	int ReturnAfterDieTimeMax;
};

class CFakeOnline
{
public:
	CFakeOnline();
	virtual ~CFakeOnline();
	void LoadFakeData(char* path);
	void RestoreFakeOnline();
	OFFEXP_DATA* GetOffExpInfo(LPOBJ lpObj);
	OFFEXP_DATA* GetOffExpInfoByAccount(LPOBJ lpObj);
	void FakeAttackProc(LPOBJ lpObj);
	void Attack(int UserIndex);
	void OnAttackAlreadyConnected(LPOBJ lpObj);
	int  PickupItem(int aIndex);
	void ReturnOriginalPosition(int aIndex);
	bool GetReturnAfterDiePosition(OFFEXP_DATA* info, int map, int* x, int* y);
	bool IsFakeTargetActive(LPOBJ lpObj, int targetType);
	void UsePotion(int aIndex);
	void UseAutoSkill(int aIndex);
	bool GetTargetPlayer(LPOBJ lpObj, int SkillNumber, int* MonsterIndex); // OK
	bool GetTargetMonster(LPOBJ lpObj, int SkillNumber, int* MonsterIndex); // OK
	void SendSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void SendMultiSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void SendDurationSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void SendRFSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
public:
	std::map<std::string, OFFEXP_DATA> m_Data;
	int AccountsRestored;
	DWORD TimeFakeLogIn;
	int DelayRange;
};

extern CFakeOnline s_FakeOnline;
#endif
