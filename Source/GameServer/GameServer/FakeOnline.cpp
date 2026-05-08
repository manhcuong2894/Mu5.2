#include "stdafx.h"
#include "FakeOnline.h"
#include "ItemManager.h"
#include "Map.h"
#include "MasterSkillTree.h"
#include "Notice.h"
#include "SkillManager.h"
#include "SocketManager.h"
#include "Viewport.h"
#include "Util.h"
#include "MemScript.h"
#include "Path.h"
#include "Party.h"
#include "EffectManager.h"
#include "MapManager.h"
#include "Message.h"
#include "Monster.h"
#include "DSProtocol.h"
#include "Quest.h"
#include "QuestObjective.h"
#include <list>
#include <string>
#include "JSProtocol.h"
#include "ObjectManager.h"
#include "Move.h"
#include "CommandManager.h"
#include "Gate.h"
#include "ItemLevel.h"
#include "ServerInfo.h"
#include "MapServerManager.h"
#include "Attack.h"
#include "Protocol.h"

#if FAKE_ONLINE == TRUE
//========================================================================================================================
CFakeOnline s_FakeOnline;

static DWORD FakeOnlineGetAttackDelay(LPOBJ lpObj, CSkill* lpSkill)
{
	int MultiPlicador = (lpObj->Class == CLASS_RF) ? 1 : 5;
	DWORD AttackDelay = ((((DWORD)lpObj->PhysiSpeed) * MultiPlicador) > 1500) ? 0 : (1500 - (((DWORD)lpObj->PhysiSpeed) * MultiPlicador));
	DWORD MinDelay = (lpObj->Class == CLASS_RF) ? 1000 : 850;
	DWORD MaxDelay = (lpObj->Class == CLASS_RF) ? 1400 : 1300;

	if (lpSkill->m_skill == SKILL_DARK_SIDE)
	{
		MinDelay = 1050;
		MaxDelay = 1400;
	}

	if (AttackDelay < MinDelay)
	{
		AttackDelay = MinDelay;
	}

	if (AttackDelay > MaxDelay)
	{
		AttackDelay = MaxDelay;
	}

	SKILL_INFO SkillInfo;

	if (gSkillManager.GetInfo(lpSkill->m_index, &SkillInfo) != 0 && SkillInfo.Delay > 0 && AttackDelay < ((DWORD)SkillInfo.Delay))
	{
		AttackDelay = (DWORD)SkillInfo.Delay;
	}

	return AttackDelay;
}

static bool FakeOnlineNeedSkillVisual(LPOBJ lpObj, CSkill* lpSkill)
{
	DWORD VisualDelay = 180;

	if (lpSkill->m_skill == SKILL_DARK_SIDE)
	{
		VisualDelay = 320;
	}

	if (GetTickCount() >= (lpObj->IsFakeSkillVisualTick + VisualDelay))
	{
		lpObj->IsFakeSkillVisualTick = GetTickCount();
		return true;
	}

	return false;
}

static void FakeOnlineSendSkillVisual(LPOBJ lpObj, int aIndex, CSkill* lpSkill)
{
	if (FakeOnlineNeedSkillVisual(lpObj, lpSkill) == 0)
	{
		return;
	}

	lpObj->ActionState.Attack = 1;
	lpObj->ActionState.Emotion = 0;
	lpObj->ActionState.EmotionCount = 0;

	if (lpSkill->m_skill == SKILL_DARK_SIDE)
	{
		gSkillManager.GCRageFighterSkillAttackSend(lpObj, lpSkill->m_index, aIndex, 1);
		return;
	}

	BYTE dir = (gSkillManager.GetSkillAngle(gObj[aIndex].X, gObj[aIndex].Y, lpObj->X, lpObj->Y) * 255) / 360;
	gSkillManager.GCDurationSkillAttackSend(lpObj, lpSkill->m_index, (BYTE)gObj[aIndex].X, (BYTE)gObj[aIndex].Y, dir);
}

CFakeOnline::CFakeOnline() {
	this->m_Data.clear();
	this->DelayRange = 0;
}

CFakeOnline::~CFakeOnline() {
}
//========================================================================================================================

void CFakeOnline::LoadFakeData(char* path)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(path);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", path, res.description());
		return;
	}

	this->m_Data.clear();
	this->DelayRange = 0;


	pugi::xml_node Recipe1 = file.child("Config");
	this->DelayRange = Recipe1.attribute("DelayRange").as_int();
	pugi::xml_node oFakeOnlineData = file.child("FakeOnlineData");
	for (pugi::xml_node rInfoData = oFakeOnlineData.child("Info"); rInfoData; rInfoData = rInfoData.next_sibling())
	{
		OFFEXP_DATA info;

		memset(&info, 0, sizeof(info));

		strcpy_s(info.Account, rInfoData.attribute("Account").as_string());
		strcpy_s(info.Password, rInfoData.attribute("Password").as_string());
		strcpy_s(info.Name, rInfoData.attribute("Name").as_string());

		info.SkillID = rInfoData.attribute("SkillID").as_int();
		info.GateNumber = rInfoData.attribute("GateNumber").as_int();
		info.MapX = rInfoData.attribute("MapX").as_int();
		info.MapY = rInfoData.attribute("MapY").as_int();
		info.PhamViTrain = rInfoData.attribute("PhamViTrain").as_int();
		info.MoveRange = rInfoData.attribute("MoveRange").as_int();
		info.TimeReturn = rInfoData.attribute("TimeReturn").as_int();
		info.TuNhatItem = rInfoData.attribute("TuNhatItem").as_int();
		info.TuDongReset = rInfoData.attribute("TuDongReset").as_int();
		info.PVPMode = rInfoData.attribute("PVPMode").as_int();
		info.PartyMode = rInfoData.attribute("PartyMode").as_int();
		info.ReturnAfterDie = rInfoData.attribute("ReturnAfterDie").as_int();

		const char* returnAfterDieTime = rInfoData.attribute("ReturnAfterDieTime").as_string();

		if (returnAfterDieTime[0] != 0)
		{
			if (sscanf_s(returnAfterDieTime, "%d,%d", &info.ReturnAfterDieTimeMin, &info.ReturnAfterDieTimeMax) == 1)
			{
				info.ReturnAfterDieTimeMax = info.ReturnAfterDieTimeMin;
			}
		}

		if (info.PVPMode < 0 || info.PVPMode > 2)
		{
			info.PVPMode = 0;
		}

		if (info.PartyMode < 0 || info.PartyMode > 3)
		{
			info.PartyMode = 0;
		}

		if (info.ReturnAfterDie < 0 || info.ReturnAfterDie > 1)
		{
			info.ReturnAfterDie = 0;
		}

		if (info.ReturnAfterDieTimeMin < 0)
		{
			info.ReturnAfterDieTimeMin = 0;
		}

		if (info.ReturnAfterDieTimeMax < info.ReturnAfterDieTimeMin)
		{
			info.ReturnAfterDieTimeMax = info.ReturnAfterDieTimeMin;
		}

		this->m_Data.insert(std::pair<std::string, OFFEXP_DATA>(info.Account, info));
	}
	LogAdd(LOG_BLUE, "[FakeOnline] Load Data OK");
}

void CFakeOnline::RestoreFakeOnline()
{
	for (std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.begin(); it != this->m_Data.end(); it++)
	{
		if (gObjFindByAcc(it->second.Account) != 0) {
			continue;
		}

		int aIndex = gObjAddSearch(0, "127.0.0.1");

		if (aIndex >= 0)
		{
			char account[11] = { 0 };
			memcpy(account, it->second.Account, (sizeof(account) - 1));

			char password[11] = { 0 };
			memcpy(password, it->second.Password, (sizeof(password) - 1));


			gObjAdd(0, "127.0.0.1", aIndex);
			gObj[aIndex].LoginMessageSend++;
			gObj[aIndex].LoginMessageSend++;
			gObj[aIndex].LoginMessageCount++;
			gObj[aIndex].ConnectTickCount = GetTickCount();
			gObj[aIndex].ClientTickCount = GetTickCount();
			gObj[aIndex].ServerTickCount = GetTickCount();

			gObj[aIndex].MapServerMoveRequest = 0;
			gObj[aIndex].LastServerCode = -1;
			gObj[aIndex].DestMap = -1;
			gObj[aIndex].DestX = 0;
			gObj[aIndex].DestY = 0;

			GJConnectAccountSend(aIndex, account, password, "127.0.0.1");

			gObj[aIndex].Socket = INVALID_SOCKET;

			LogAdd(LOG_RED, "[FakeOnline] [%d]  [TK: %s NV: %s] Online", aIndex, it->second.Account, it->second.Name);
		}
	}
}

OFFEXP_DATA* CFakeOnline::GetOffExpInfo(LPOBJ lpObj)
{
	std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.find(lpObj->Account);

	if (it != this->m_Data.end())
	{
		if (strcmp(lpObj->Name, it->second.Name) == 0)
		{
			return &it->second;
		}
	}
	return 0;
}


OFFEXP_DATA* CFakeOnline::GetOffExpInfoByAccount(LPOBJ lpObj)
{
	std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.find(lpObj->Account);
	if (it != this->m_Data.end())
	{
		return &it->second;
	}
	return 0;
}

bool CFakeOnline::GetReturnAfterDiePosition(OFFEXP_DATA* info, int map, int* x, int* y)
{
	if (info == 0 || x == 0 || y == 0 || MAP_RANGE(map) == 0)
	{
		return 0;
	}

	int baseX = info->MapX;
	int baseY = info->MapY;

	if (gMap[map].CheckAttr(baseX, baseY, 2) == 0 && gMap[map].CheckAttr(baseX, baseY, 4) == 0 && gMap[map].CheckAttr(baseX, baseY, 8) == 0)
	{
		*x = baseX;
		*y = baseY;
		return 1;
	}

	int searchRange = ((info->MoveRange > 0) ? (info->MoveRange + 3) : 3);

	if (searchRange < 3)
	{
		searchRange = 3;
	}

	for (int range = 1; range <= searchRange; range++)
	{
		for (int tx = (baseX - range); tx <= (baseX + range); tx++)
		{
			for (int ty = (baseY - range); ty <= (baseY + range); ty++)
			{
				if (tx != (baseX - range) && tx != (baseX + range) && ty != (baseY - range) && ty != (baseY + range))
				{
					continue;
				}

				if (gMap[map].CheckAttr(tx, ty, 2) == 0 && gMap[map].CheckAttr(tx, ty, 4) == 0 && gMap[map].CheckAttr(tx, ty, 8) == 0)
				{
					*x = tx;
					*y = ty;
					return 1;
				}
			}
		}
	}

	if (gMap[map].CheckAttr(baseX, baseY, 4) == 0 && gMap[map].CheckAttr(baseX, baseY, 8) == 0)
	{
		*x = baseX;
		*y = baseY;
		return 1;
	}

	return 0;
}

bool CFakeOnline::IsFakeTargetActive(LPOBJ lpObj, int targetType)
{
	if (lpObj == 0 || OBJECT_RANGE(lpObj->IsFakeTargetIndex) == 0)
	{
		return 0;
	}

	LPOBJ lpTarget = &gObj[lpObj->IsFakeTargetIndex];

	if (lpTarget->Live == 0 || lpTarget->State == OBJECT_EMPTY || lpTarget->DieRegen != 0 || lpTarget->RegenType != 0)
	{
		lpObj->IsFakeTargetIndex = -1;
		return 0;
	}

	if (lpTarget->Map != lpObj->Map)
	{
		lpObj->IsFakeTargetIndex = -1;
		return 0;
	}

	return ((lpTarget->Type == targetType) ? 1 : 0);
}

void FakeAnimationMove(int aIndex, int x, int y, bool dixa)
{
	LPOBJ lpObj = &gObj[aIndex];

	int map_num = gObj[aIndex].Map;

	BYTE path[8];

	if (lpObj->RegenOk > 0) {
		return;
	}

	if (lpObj->Teleport != 0) {
		return;
	}

	if (gObjCheckMapTile(lpObj, 1) != 0) {
		return;
	}

	if (gEffectManager.CheckStunEffect(lpObj) != 0 || gEffectManager.CheckImmobilizeEffect(lpObj) != 0) {
		return;
	}

	if (lpObj->SkillSummonPartyTime != 0) {
		lpObj->SkillSummonPartyTime = 0;
	}

	lpObj->Dir = path[0] >> 4;
	lpObj->Rest = 0;
	lpObj->PathCur = 0;
	lpObj->PathCount = path[0] & 0x0F;
	lpObj->LastMoveTime = GetTickCount();

	memset(lpObj->PathX, 0, sizeof(lpObj->PathX));

	memset(lpObj->PathY, 0, sizeof(lpObj->PathY));

	memset(lpObj->PathOri, 0, sizeof(lpObj->PathOri));

	lpObj->TX = x;
	lpObj->TY = y;
	lpObj->PathCur = ((lpObj->PathCount > 0) ? 1 : 0);
	lpObj->PathCount = ((lpObj->PathCount > 0) ? (lpObj->PathCount + 1) : lpObj->PathCount);
	lpObj->PathStartEnd = 1;
	lpObj->PathX[0] = x;
	lpObj->PathY[0] = y;
	lpObj->PathDir[0] = lpObj->Dir;

	for (int n = 1; n < lpObj->PathCount; n++)
	{
		if ((n % 2) == 0)
		{
			lpObj->TX = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 0];
			lpObj->TY = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 1];
			lpObj->PathX[n] = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 0];
			lpObj->PathY[n] = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 1];
			lpObj->PathOri[n - 1] = path[((n + 1) / 2)] & 0x0F;
			lpObj->PathDir[n + 0] = path[((n + 1) / 2)] & 0x0F;
		}
		else
		{
			lpObj->TX = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 0];
			lpObj->TY = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 1];
			lpObj->PathX[n] = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 0];
			lpObj->PathY[n] = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 1];
			lpObj->PathOri[n - 1] = path[((n + 1) / 2)] / 0x10;
			lpObj->PathDir[n + 0] = path[((n + 1) / 2)] / 0x10;
		}
	}

	gMap[lpObj->Map].DelStandAttr(lpObj->OldX, lpObj->OldY);

	if (dixa == true) {
		int RandX = rand() % 3 + 1;
		int RandY = rand() % 3 + 1;
		BYTE wall = 0;
		if (x > lpObj->X) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X + RandX, lpObj->Y);
			if (wall == 1) lpObj->X += RandX;
		}
		else if (x < lpObj->X) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X - RandX, lpObj->Y);
			if (wall == 1)  lpObj->X -= RandX;
		}
		if (y > lpObj->Y) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y + RandY);
			if (wall == 1) lpObj->Y += RandY;
		}
		else if (y < lpObj->Y) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y - RandY);
			if (wall == 1) lpObj->Y -= RandY;
		}

	}
	else {
		lpObj->X = x;
		lpObj->Y = y;
	}

	lpObj->TX = lpObj->TX;
	lpObj->TY = lpObj->TY;

	lpObj->OldX = lpObj->TX;
	lpObj->OldY = lpObj->TY;

	lpObj->ViewState = 0;

	gMap[lpObj->Map].SetStandAttr(lpObj->TX, lpObj->TY);

	PMSG_MOVE_SEND pMsg;

	pMsg.header.set(PROTOCOL_CODE1, sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(lpObj->Index);

	pMsg.index[1] = SET_NUMBERLB(lpObj->Index);

	pMsg.x = (BYTE)lpObj->TX;

	pMsg.y = (BYTE)lpObj->TY;

	pMsg.dir = lpObj->Dir << 4;

	lpObj->PathCur = 0;
	lpObj->PathCount = 0;

	lpObj->TX = lpObj->X;
	lpObj->TY = lpObj->Y;

	pMsg.x = (BYTE)lpObj->X;
	pMsg.y = (BYTE)lpObj->Y;


	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].type == OBJECT_USER)
		{
			if (lpObj->VpPlayer2[n].state != OBJECT_EMPTY && lpObj->VpPlayer2[n].state != OBJECT_DIECMD && lpObj->VpPlayer2[n].state != OBJECT_DIED)
			{
				DataSend(lpObj->VpPlayer2[n].index, (BYTE*)&pMsg, pMsg.header.size);
			}
		}
	}
}
void FakeAutoRepair(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	for (int n = 0; n < INVENTORY_WEAR_SIZE; ++n)
	{
		if (lpObj->Inventory[n].IsItem() != 0)
		{
			int money = gItemManager.RepairItem(lpObj, &lpObj->Inventory[n], n, 1);

			if (money != 0)
			{
				gObjectManager.CharacterCalcAttribute(aIndex);
			}
		}
	}
}

void CFakeOnline::FakeAttackProc(LPOBJ lpObj) // OK
{
	if (lpObj->IsFakeOnline != 0)
	{
		lpObj->CheckSumTime = GetTickCount();
		lpObj->ConnectTickCount = GetTickCount();
	}
}
void CFakeOnline::OnAttackAlreadyConnected(LPOBJ lpObj) // OK
{
	if (lpObj->IsFakeOnline != 0)
	{
		lpObj->IsFakeOnline = 0;
		gObjDel(lpObj->Index);
#if FAKE_ONLINE == TRUE
		s_FakeOnline.FakeAttackProc(lpObj);
#endif
	}
}

void CFakeOnline::Attack(int aIndex)
{
	if (OBJMAX_RANGE(aIndex) == FALSE) {
		return;
	}

	if (!gObjIsConnectedGP(aIndex)) {
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->IsFakeOnline == 0 || !lpObj->IsFakeRegen) {
		return;
	}

	if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0) {
		return;
	}

	if (gServerInfo.InSafeZone(aIndex) == true) {
		return;
	}

	this->UsePotion(aIndex);

	this->UseAutoSkill(aIndex);

	FakeAutoRepair(aIndex);
}

bool FakeisJewels(int index)
{
	if (index == GET_ITEM(12, 15) ||
		index == GET_ITEM(14, 13) ||
		index == GET_ITEM(14, 14) ||
		index == GET_ITEM(14, 16) ||
		index == GET_ITEM(14, 22) ||
		index == GET_ITEM(14, 31) ||
		index == GET_ITEM(14, 42))
	{
		return true;
	}

	return false;
}

static bool FakeOnlineCanPickItem(LPOBJ lpObj, CMapItem* lpMapItem)
{
	if (lpObj->ObtainPickSelected != 1)
	{
		return false;
	}

	if (lpObj->ObtainPickMoney == 1 && lpMapItem->m_Index == GET_ITEM(14, 15))
	{
		return true;
	}

	if (lpObj->ObtainPickJewels == 1 && FakeisJewels(lpMapItem->m_Index) == true)
	{
		return true;
	}

	return false;
}

static int FakeOnlineGetPickupOwner(int map, int itemIndex, CMapItem* lpMapItem)
{
	int bestDistance = 0x7FFFFFFF;
	int ownerCount = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (OBJMAX_RANGE(n) == FALSE || gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		LPOBJ lpOwner = &gObj[n];

		if (lpOwner->IsFakeOnline == 0 || lpOwner->Map != map)
		{
			continue;
		}

		if (lpOwner->State == OBJECT_DELCMD || lpOwner->DieRegen != 0 || lpOwner->Teleport != 0)
		{
			continue;
		}

		if (gServerInfo.InSafeZone(n) == true)
		{
			continue;
		}

		if (FakeOnlineCanPickItem(lpOwner, lpMapItem) == false)
		{
			continue;
		}

		int distance = (int)sqrt((float)((lpOwner->X - lpMapItem->m_X) * (lpOwner->X - lpMapItem->m_X) + (lpOwner->Y - lpMapItem->m_Y) * (lpOwner->Y - lpMapItem->m_Y)));

		if (distance > (int)lpOwner->ObtainRange)
		{
			continue;
		}

		if (distance < bestDistance)
		{
			bestDistance = distance;
		}
	}

	if (bestDistance == 0x7FFFFFFF)
	{
		return -1;
	}

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (OBJMAX_RANGE(n) == FALSE || gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		LPOBJ lpOwner = &gObj[n];

		if (lpOwner->IsFakeOnline == 0 || lpOwner->Map != map)
		{
			continue;
		}

		if (lpOwner->State == OBJECT_DELCMD || lpOwner->DieRegen != 0 || lpOwner->Teleport != 0)
		{
			continue;
		}

		if (gServerInfo.InSafeZone(n) == true)
		{
			continue;
		}

		if (FakeOnlineCanPickItem(lpOwner, lpMapItem) == false)
		{
			continue;
		}

		int distance = (int)sqrt((float)((lpOwner->X - lpMapItem->m_X) * (lpOwner->X - lpMapItem->m_X) + (lpOwner->Y - lpMapItem->m_Y) * (lpOwner->Y - lpMapItem->m_Y)));

		if (distance == bestDistance)
		{
			ownerCount++;
		}
	}

	int ownerOffset = ((ownerCount <= 1) ? 0 : (itemIndex % ownerCount));
	int ownerPosition = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (OBJMAX_RANGE(n) == FALSE || gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		LPOBJ lpOwner = &gObj[n];

		if (lpOwner->IsFakeOnline == 0 || lpOwner->Map != map)
		{
			continue;
		}

		if (lpOwner->State == OBJECT_DELCMD || lpOwner->DieRegen != 0 || lpOwner->Teleport != 0)
		{
			continue;
		}

		if (gServerInfo.InSafeZone(n) == true)
		{
			continue;
		}

		if (FakeOnlineCanPickItem(lpOwner, lpMapItem) == false)
		{
			continue;
		}

		int distance = (int)sqrt((float)((lpOwner->X - lpMapItem->m_X) * (lpOwner->X - lpMapItem->m_X) + (lpOwner->Y - lpMapItem->m_Y) * (lpOwner->Y - lpMapItem->m_Y)));

		if (distance != bestDistance)
		{
			continue;
		}

		if (ownerPosition == ownerOffset)
		{
			return n;
		}

		ownerPosition++;
	}

	return -1;
}

int CFakeOnline::PickupItem(int aIndex)
{
	if (OBJMAX_RANGE(aIndex) == FALSE) {
		return 0;
	}

	if (!gObjIsConnectedGP(aIndex)) {
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->IsFakeOnline == 0) {
		return 0;
	}

	if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0) {
		return 0;
	}

	if (gServerInfo.InSafeZone(aIndex) == true) {
		return 0;
	}

	CMapItem* lpMapItem;

	int distance = lpObj->ObtainRange;

	int map_num = gObj[aIndex].Map;

	if (gObj[aIndex].DieRegen != 0)
	{
		return 0;
	}

	if (MAP_RANGE(map_num) == FALSE)
	{
		return 0;
	}

	int OldX;
	int OldY;
	bool PickItem;

	for (int n = 0; n < MAX_MAP_ITEM; n++)
	{
		PickItem = false;

		lpMapItem = &gMap[map_num].m_Item[n];

		if (lpMapItem->IsItem() == TRUE && lpMapItem->m_Give == false && lpMapItem->m_Live == true)
		{
			int dis = (int)sqrt((float)((lpObj->X - lpMapItem->m_X) * (lpObj->X - lpMapItem->m_X) + (lpObj->Y - lpMapItem->m_Y) * (lpObj->Y - lpMapItem->m_Y)));

			if (dis > distance)
			{
				continue;
			}

			OldX = lpObj->X;

			OldY = lpObj->Y;

			if (FakeOnlineGetPickupOwner(map_num, n, lpMapItem) != aIndex)
			{
				continue;
			}

			if (lpObj->ObtainPickSelected == 1)
			{
				if ((lpObj->ObtainPickMoney == 1 && lpMapItem->m_Index == GET_ITEM(14, 15))
					|| (lpObj->ObtainPickJewels == 1 && FakeisJewels(lpMapItem->m_Index) == true))
				{
					PickItem = true;
					FakeAnimationMove(lpObj->Index, lpMapItem->m_X, lpMapItem->m_Y, false);
				}
				else
				{
					PickItem = false;
					continue;
				}
			}
			else
			{
				PickItem = false;
				continue;
			}

			if (PickItem == false)
			{
				continue;
			}

			if (lpObj->X == lpMapItem->m_X && lpObj->Y == lpMapItem->m_Y)
			{
				//zen item
				if (lpMapItem->m_Index == GET_ITEM(14, 15))
				{
					if (lpObj->ObtainPickMoney == 1)
					{
						gMap[map_num].ItemGive(aIndex, n);

						if (!gObjCheckMaxMoney(aIndex, lpMapItem->m_BuyMoney))
						{
							if (lpObj->Money < MAX_MONEY)
							{
								lpObj->Money = MAX_MONEY;
								return 1;
							}
						}
						else
						{
							lpObj->Money += lpMapItem->m_BuyMoney;
						}
						return 1;
					}
				}
				//all items
				if (lpMapItem->m_QuestItem != false)
				{
					if (!gQuestObjective.CheckQuestObjectiveItemCount(lpObj, lpMapItem->m_Index, lpMapItem->m_Level))
					{
						continue;
					}
				}

				CItem item = (*lpMapItem);

				BYTE result = gItemManager.InventoryInsertItemStack(&gObj[aIndex], lpMapItem);

				if (result != 0xFF)
				{
					gMap[map_num].ItemGive(aIndex, n);

					BYTE pos = gItemManager.InventoryInsertItem(aIndex, item);

					if (pos != 0xFF)
					{
						::GCPartyItemInfoSend(aIndex, lpMapItem);
					}

					return 1;
				}
			}
		}
	}

	return 1;
}

int random(int minN, int maxN) {
	srand((int)time(0));
	return minN + rand() % (maxN + 1 - minN);
}

void CFakeOnline::ReturnOriginalPosition(int aIndex)
{
	if (OBJMAX_RANGE(aIndex) == FALSE) {
		return;
	}

	if (!gObjIsConnectedGP(aIndex)) {
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->IsFakeOnline == 0) {
		return;
	}

	OFFEXP_DATA* info = s_FakeOnline.GetOffExpInfo(lpObj);

	if (info != 0 && lpObj->Socket == INVALID_SOCKET)
	{
		if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0) {
			return;
		}

		if (lpObj->IsFakeReturnAfterDie != 0)
		{
			if (GetTickCount() < lpObj->IsFakeReturnAfterDieTick)
			{
				return;
			}

			int gate, map, x, y, dir, level;

			if (gGate.GetGate(info->GateNumber, &gate, &map, &x, &y, &dir, &level) != 0)
			{
				int oldMap = lpObj->Map;
				int oldX = lpObj->X;
				int oldY = lpObj->Y;

				if (MAP_RANGE(oldMap) != 0)
				{
					gMap[oldMap].DelStandAttr(oldX, oldY);
					gMap[oldMap].DelStandAttr(lpObj->OldX, lpObj->OldY);
				}

				if (this->GetReturnAfterDiePosition(info, map, &x, &y) != 0)
				{
					lpObj->IsFakeReturnAfterDie = false;
					lpObj->IsFakeRegen = true;
					lpObj->m_OfflineCoordX = info->MapX;
					lpObj->m_OfflineCoordY = info->MapY;
					lpObj->m_OfflineTimeResetMove = GetTickCount();
					lpObj->m_OfflineMoveDelay = GetTickCount();
					lpObj->AttackCustomDelay = GetTickCount();


					gObjClearViewport(lpObj);

					lpObj->X = x;
					lpObj->Y = y;
					lpObj->TX = x;
					lpObj->TY = y;
					lpObj->OldX = x;
					lpObj->OldY = y;
					lpObj->Map = map;
					lpObj->Dir = dir;
					lpObj->PathCur = 0;
					lpObj->PathCount = 0;
					lpObj->PathStartEnd = 0;
					lpObj->Teleport = 0;
					lpObj->ViewState = 0;
					lpObj->MiniMapState = 0;
					lpObj->MiniMapValue = -1;
					lpObj->RegenMapNumber = (BYTE)map;
					lpObj->RegenMapX = (BYTE)x;
					lpObj->RegenMapY = (BYTE)y;
					lpObj->RegenOk = 0;
					lpObj->State = OBJECT_CREATE;

					gMap[lpObj->Map].SetStandAttr(lpObj->X, lpObj->Y);
					gObjViewportListCreate(lpObj->Index);
					gObjViewportListProtocolCreate(lpObj);
					gObjectManager.CharacterUpdateMapEffect(lpObj);

					LogAdd(LOG_BLUE, "[FakeOnline][%s] ReturnAfterDie delayed return Map:%d X:%d Y:%d ConfigX:%d ConfigY:%d", lpObj->Name, map, x, y, info->MapX, info->MapY);
					return;
				}

				LogAdd(LOG_RED, "[FakeOnline][%s] ReturnAfterDie invalid position Gate:%d Map:%d X:%d Y:%d", lpObj->Name, info->GateNumber, map, x, y);
			}
			else
			{
				LogAdd(LOG_RED, "[FakeOnline][%s] ReturnAfterDie invalid gate %d", lpObj->Name, info->GateNumber);
			}

			lpObj->IsFakeReturnAfterDie = false;
		}

		if (this->IsFakeTargetActive(lpObj, OBJECT_MONSTER) != 0 || this->IsFakeTargetActive(lpObj, OBJECT_USER) != 0)
		{
			return;
		}

		int PhamViDiTrain = (int)sqrt(pow(((float)lpObj->X - (float)info->MapX), 2) + pow(((float)lpObj->Y - (float)info->MapY), 2));

		if ((GetTickCount() >= lpObj->IsFakeTimeLag + 30000) && (GetTickCount() >= lpObj->AttackCustomDelay + 30000) && lpObj->IsFakeRegen && (GetTickCount() >= lpObj->m_OfflineMoveDelay + 30000))
		{
			lpObj->IsFakeRegen = false;
			lpObj->IsFakeTimeLag = GetTickCount();
			lpObj->m_OfflineMoveDelay = GetTickCount();
			lpObj->AttackCustomDelay = GetTickCount();
			PhamViDiTrain = (lpObj->IsFakeMoveRange + 10);
		}

		int gateMap = gGate.GetGateMap(info->GateNumber);

		if ((MAP_RANGE(gateMap) != 0 && lpObj->Map != gateMap) || (PhamViDiTrain >= 100 && !lpObj->IsFakeRegen))
		{
			gObjMoveGate(lpObj->Index, info->GateNumber);
			LogAdd(LOG_BLUE, "[FakeOnline][%s] Move Gate", lpObj->Name);
			return;
		}

		if (GetTickCount() >= lpObj->m_OfflineTimeResetMove + 2000) // 2000
		{
			if ((PhamViDiTrain >= (lpObj->IsFakeMoveRange + 5) && !lpObj->IsFakeRegen) || gServerInfo.InSafeZone(lpObj->Index) == true)
			{
				int DiChuyenX = lpObj->X;
				int DiChuyenY = lpObj->Y;
				for (int n = 0; n < 16; n++)
				{
					if (lpObj->X > info->MapX) { DiChuyenX -= random(1, 3); }
					else if (lpObj->X < info->MapX) { DiChuyenX += random(1, 3); }
					else { DiChuyenX = info->MapX; }

					if (lpObj->Y > info->MapY) { DiChuyenY -= random(1, 3); }
					else if (lpObj->Y < info->MapY) { DiChuyenY += random(1, 3); }
					else { DiChuyenY = info->MapY; }

					if (DiChuyenX == info->MapX && DiChuyenY == info->MapY) { lpObj->IsFakeRegen = true; }

					if (gMap[lpObj->Map].CheckAttr(DiChuyenX, DiChuyenY, 2) == 0 && gMap[lpObj->Map].CheckAttr(DiChuyenX, DiChuyenY, 4) == 0 && gMap[lpObj->Map].CheckAttr(DiChuyenX, DiChuyenY, 8) == 0) // 8
					{
						lpObj->m_OfflineTimeResetMove = GetTickCount();
						FakeAnimationMove(lpObj->Index, DiChuyenX, DiChuyenY, false);					
						return;
					}
				}
				return;

			}
			else if (!lpObj->IsFakeRegen) {
				lpObj->m_OfflineTimeResetMove = GetTickCount();
				lpObj->IsFakeRegen = true;
			}
		}

		if (lpObj->IsFakeMoveRange != 0) {
			if (GetTickCount() >= lpObj->m_OfflineTimeResetMove + 2000 && lpObj->IsFakeRegen) // 2000
			{
				int MoveRange = lpObj->IsFakeMoveRange;
				//======Move Range======//
				int maxmoverange = MoveRange * 2 + 1;
				int searchc = 10; // 10
				BYTE tpx;
				BYTE tpy;
				while (searchc-- != 0)
				{
					__try
					{
						tpx = (lpObj->X - MoveRange) + (BYTE)(GetLargeRand() % maxmoverange);
						tpy = (lpObj->Y - MoveRange) + (BYTE)(GetLargeRand() % maxmoverange);
					}
					__except (maxmoverange = 1, 1)
					{

					}
					BYTE attr = gMap[lpObj->Map].GetAttr(tpx, tpy);
					if ((attr & 1) != 1 && (attr & 2) != 2 && (attr & 4) != 4 && (attr & 8) != 8 && GetTickCount() >= lpObj->m_OfflineMoveDelay + 2000) // 1000
					{
						lpObj->m_OfflineMoveDelay = GetTickCount();
						FakeAnimationMove(lpObj->Index, tpx, tpy, false);
						return;
					}
				}
			}
		}
	}

	if (lpObj->DistanceReturnOn != 0)
	{
		if (lpObj->DistanceMin > 0 && GetTickCount() >= lpObj->m_OfflineTimeResetMove + lpObj->DistanceMin)
		{
			if (lpObj->m_OfflineCoordX != lpObj->X || lpObj->m_OfflineCoordY != lpObj->Y)
			{			
				FakeAnimationMove(lpObj->Index, lpObj->m_OfflineCoordX, lpObj->m_OfflineCoordY, false);
				return;
			}
			lpObj->m_OfflineTimeResetMove = GetTickCount();
		}
	}
}

void CFakeOnline::UsePotion(int aIndex)	//-- OK
{
	if (!gObjIsConnectedGP(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->RecoveryPotionOn != 0)
	{
		if (lpObj->Life > 0 && lpObj->Life < ((lpObj->MaxLife * lpObj->RecoveryPotionPercent) / 100))
		{
			PMSG_ITEM_USE_RECV pMsg;

			pMsg.header.set(0x26, sizeof(pMsg));

			pMsg.SourceSlot = 0xFF;

			pMsg.SourceSlot = ((pMsg.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 3), -1) : pMsg.SourceSlot);

			pMsg.SourceSlot = ((pMsg.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 2), -1) : pMsg.SourceSlot);

			pMsg.SourceSlot = ((pMsg.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 1), -1) : pMsg.SourceSlot);

			pMsg.TargetSlot = 0xFF;

			pMsg.type = 0;

			if (INVENTORY_FULL_RANGE(pMsg.SourceSlot) != 0)
			{
				gItemManager.CGItemUseRecv(&pMsg, lpObj->Index);
			}
		}
	}
}

bool CFakeOnline::GetTargetMonster(LPOBJ lpObj, int SkillNumber, int* MonsterIndex) // OK
{
	int NearestDistance = 100;

	if (this->IsFakeTargetActive(lpObj, OBJECT_MONSTER) != 0)
	{
		(*MonsterIndex) = lpObj->IsFakeTargetIndex;
		return 1;
	}

	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_MONSTER)
		{
			continue;
		}

		if (gSkillManager.CheckSkillTarget(lpObj, lpObj->VpPlayer2[n].index, -1, lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}

		if (gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]) >= NearestDistance)
		{
			continue;
		}

		if (gSkillManager.CheckSkillRange(SkillNumber, lpObj->X, lpObj->Y, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			(*MonsterIndex) = lpObj->VpPlayer2[n].index;
			lpObj->IsFakeTargetIndex = (*MonsterIndex);
			NearestDistance = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
			continue;
		}

		if (gSkillManager.CheckSkillRadio(SkillNumber, lpObj->X, lpObj->Y, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			(*MonsterIndex) = lpObj->VpPlayer2[n].index;
			lpObj->IsFakeTargetIndex = (*MonsterIndex);
			NearestDistance = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
			continue;
		}
	}

	return ((NearestDistance == 100) ? 0 : 1);
}

bool CFakeOnline::GetTargetPlayer(LPOBJ lpObj, int SkillNumber, int* MonsterIndex) // OK
{
	int NearestDistance = 100;

	if (this->IsFakeTargetActive(lpObj, OBJECT_USER) != 0)
	{
		(*MonsterIndex) = lpObj->IsFakeTargetIndex;
		return 1;
	}

	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_USER)
		{
			continue;
		}

		if (gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]) >= NearestDistance)
		{
			continue;
		}
		if (lpObj->IsFakePartyMode >= 2 && gParty.IsParty(gObj[lpObj->VpPlayer2[n].index].PartyNumber) == 0 && (GetTickCount() >= lpObj->IsFakeSendParty + 5000) && !gObjIsSelfDefense(&gObj[lpObj->VpPlayer2[n].index], lpObj->Index))
		{
			if (lpObj->IsFakePartyMode == 3 && !gObj[lpObj->VpPlayer2[n].index].IsFakeOnline) { continue; }
			lpObj->IsFakeSendParty = GetTickCount();

			if (gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]) <= MAX_PARTY_DISTANCE)
			{
				PMSG_PARTY_REQUEST_RECV pMsg;

				pMsg.header.set(0x40, sizeof(pMsg));
				pMsg.index[0] = SET_NUMBERHB(lpObj->VpPlayer2[n].index);
				pMsg.index[1] = SET_NUMBERLB(lpObj->VpPlayer2[n].index);

				gParty.CGPartyRequestRecv(&pMsg, lpObj->Index);
			}
			else
			{
				FakeAnimationMove(lpObj->Index, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y, false);
			}

			return 0;
		}
		if (gObjIsSelfDefense(&gObj[lpObj->VpPlayer2[n].index], lpObj->Index))
		{
			(*MonsterIndex) = lpObj->VpPlayer2[n].index;
			lpObj->IsFakeTargetIndex = (*MonsterIndex);
			NearestDistance = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
			continue;
		}
		if (gSkillManager.CheckSkillRange(SkillNumber, lpObj->X, lpObj->Y, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			if (lpObj->IsFakePVPMode == 2) {
				(*MonsterIndex) = lpObj->VpPlayer2[n].index;
				lpObj->IsFakeTargetIndex = (*MonsterIndex);
				NearestDistance = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
			}
			continue;
		}

		if (gSkillManager.CheckSkillRadio(SkillNumber, lpObj->X, lpObj->Y, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			if (lpObj->IsFakePVPMode == 2) {
				(*MonsterIndex) = lpObj->VpPlayer2[n].index;
				lpObj->IsFakeTargetIndex = (*MonsterIndex);
				NearestDistance = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
			}
			continue;
		}

		if (lpObj->GuildNumber == gObj[lpObj->VpPlayer2[n].index].GuildNumber && lpObj->GuildNumber != 0)
		{
			return 0;
		}
	}

	return ((NearestDistance == 100) ? 0 : 1);
}

void CFakeOnline::UseAutoSkill(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];
	int caminar = 0;
	int distance = (lpObj->HuntingRange > 6) ? 6 : lpObj->HuntingRange;

	CSkill* SkillRender;

	SkillRender = gSkillManager.GetSkill(lpObj, lpObj->SkillBasicID);

	if (SkillRender == 0)
	{
		if (GetTickCount() >= lpObj->IsFakeTimeLag + 10000)
		{
			lpObj->IsFakeTimeLag = GetTickCount();
			LogAdd(LOG_RED, "[FakeOnline][%s] SkillID %d not found in character skill list", lpObj->Name, lpObj->SkillBasicID);
		}
		return;
	}

	if (lpObj->IsFakePVPMode == 2 && lpObj->PKCount >= 10)
	{
		lpObj->PKLevel = PVP_NEUTRAL;
		lpObj->PKCount = 0;
		lpObj->PKTime = 0;

		GCPKLevelSend(lpObj->Index, lpObj->PKLevel, lpObj->PKTime);
		GDCharacterInfoSaveSend(lpObj->Index);
	}

	int atacar = 0;

	int tObjNum = -1;
	int KillUser = -1;
	if (this->GetTargetPlayer(lpObj, SkillRender->m_index, &KillUser) != 0 && lpObj->IsFakePVPMode >= 1)
	{
		atacar = 0;

		if (gObj[KillUser].Live == 0 || gObj[KillUser].State == OBJECT_EMPTY || gObj[KillUser].RegenType != 0)
		{
			return;
		}

		if (OBJMAX_RANGE(KillUser) == FALSE)
		{
			return;
		}

		if (gObj[KillUser].Type != OBJECT_USER)
		{
			return;
		}

		if (gServerInfo.InSafeZone(KillUser) == true)
		{
			return;
		}

		int dis = (int)sqrt((float)((lpObj->X - gObj[KillUser].X) * (lpObj->X - gObj[KillUser].X) + (lpObj->Y - gObj[KillUser].Y) * (lpObj->Y - gObj[KillUser].Y)));

		if (dis > distance)
		{
			return;
		}
		else
		{
			if (gSkillManager.CheckSkillRange(SkillRender->m_index, lpObj->X, lpObj->Y, gObj[KillUser].X, gObj[KillUser].Y) != 0 ||
				gSkillManager.CheckSkillRadio(SkillRender->m_index, lpObj->X, lpObj->Y, gObj[KillUser].X, gObj[KillUser].Y) != 0)
			{
				caminar = 0;
			}
			else
			{
				caminar = 1;

			}

			if (caminar == 1)
			{
				return;
			}

			atacar = 1;
		}

		if (atacar != 0)
		{
			if (lpObj->Mana < gSkillManager.GetSkillMana(SkillRender->m_index))
			{
				//-- AUTO POTION MP PRIMERO VALIDA QUE NO ESTE MUERTO
				PMSG_ITEM_USE_RECV pMsgMP;

				pMsgMP.header.set(0x26, sizeof(pMsgMP));

				pMsgMP.SourceSlot = 0xFF;

				pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 6), -1) : pMsgMP.SourceSlot);

				pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 5), -1) : pMsgMP.SourceSlot);

				pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 4), -1) : pMsgMP.SourceSlot);

				pMsgMP.TargetSlot = 0xFF;

				pMsgMP.type = 0;

				if (INVENTORY_FULL_RANGE(pMsgMP.SourceSlot) != 0)
				{
					gItemManager.CGItemUseRecv(&pMsgMP, lpObj->Index);
				}
				else
				{
					return;
				}
			}

			FakeOnlineSendSkillVisual(lpObj, KillUser, SkillRender);

			if ((GetTickCount() - ((DWORD)lpObj->AttackCustomDelay)) >= FakeOnlineGetAttackDelay(lpObj, SkillRender))
			{
				lpObj->AttackCustomDelay = GetTickCount();

				if (SkillRender->m_skill != SKILL_FLAME
					&& SkillRender->m_skill != SKILL_TWISTER
					&& SkillRender->m_skill != SKILL_EVIL_SPIRIT
					&& SkillRender->m_skill != SKILL_HELL_FIRE
					&& SkillRender->m_skill != SKILL_AQUA_BEAM
					&& SkillRender->m_skill != SKILL_BLAST
					&& SkillRender->m_skill != SKILL_INFERNO
					&& SkillRender->m_skill != SKILL_TRIPLE_SHOT
					&& SkillRender->m_skill != SKILL_IMPALE
					&& SkillRender->m_skill != SKILL_MONSTER_AREA_ATTACK
					&& SkillRender->m_skill != SKILL_PENETRATION
					&& SkillRender->m_skill != SKILL_FIRE_SLASH
					&& SkillRender->m_skill != SKILL_FIRE_SCREAM)
				{
					if (SkillRender->m_skill != SKILL_DARK_SIDE)
					{

						this->SendDurationSkillAttack(lpObj, KillUser, SkillRender->m_index);
					}
					else
					{
						this->SendRFSkillAttack(lpObj, KillUser, SkillRender->m_index);
					}
				}
				else
				{
					this->SendMultiSkillAttack(lpObj, KillUser, SkillRender->m_index);
				}
			}
		}

		return;
	}
	//=============================================================
	if (this->GetTargetMonster(lpObj, SkillRender->m_index, &tObjNum) != 0)
	{

		atacar = 0;

		if (gObj[tObjNum].Live == 0 || gObj[tObjNum].State == OBJECT_EMPTY || gObj[tObjNum].RegenType != 0)
		{
			return;
		}

		if (OBJMAX_RANGE(tObjNum) == FALSE)
		{
			return;
		}

		if (gObj[tObjNum].Type != OBJECT_MONSTER)
		{
			return;
		}

		if (gServerInfo.InSafeZone(tObjNum) == true)
		{
			return;
		}

		int dis = (int)sqrt((float)((lpObj->X - gObj[tObjNum].X) * (lpObj->X - gObj[tObjNum].X) + (lpObj->Y - gObj[tObjNum].Y) * (lpObj->Y - gObj[tObjNum].Y)));

		if (dis > distance)
		{
			return;
		}
		else
		{
			if (gSkillManager.CheckSkillRange(SkillRender->m_index, lpObj->X, lpObj->Y, gObj[tObjNum].X, gObj[tObjNum].Y) != 0 ||
				gSkillManager.CheckSkillRadio(SkillRender->m_index, lpObj->X, lpObj->Y, gObj[tObjNum].X, gObj[tObjNum].Y) != 0)
			{
				caminar = 0;
			}
			else
			{
				caminar = 1;

			}

			if (caminar == 1)
			{
				return;
			}

			atacar = 1;
		}

		if (atacar != 0)
		{
			if (lpObj->Mana < gSkillManager.GetSkillMana(SkillRender->m_index))
			{
				PMSG_ITEM_USE_RECV pMsgMP;

				pMsgMP.header.set(0x26, sizeof(pMsgMP));

				pMsgMP.SourceSlot = 0xFF;

				pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 6), -1) : pMsgMP.SourceSlot);

				pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 5), -1) : pMsgMP.SourceSlot);

				pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 4), -1) : pMsgMP.SourceSlot);

				pMsgMP.TargetSlot = 0xFF;

				pMsgMP.type = 0;

				if (INVENTORY_FULL_RANGE(pMsgMP.SourceSlot) != 0)
				{
					gItemManager.CGItemUseRecv(&pMsgMP, lpObj->Index);
				}
				else
				{
					return;
				}
			}

			FakeOnlineSendSkillVisual(lpObj, tObjNum, SkillRender);

			if ((GetTickCount() - ((DWORD)lpObj->AttackCustomDelay)) >= FakeOnlineGetAttackDelay(lpObj, SkillRender))
			{
				lpObj->AttackCustomDelay = GetTickCount();

				if (SkillRender->m_skill != SKILL_FLAME
					&& SkillRender->m_skill != SKILL_TWISTER
					&& SkillRender->m_skill != SKILL_EVIL_SPIRIT
					&& SkillRender->m_skill != SKILL_HELL_FIRE
					&& SkillRender->m_skill != SKILL_AQUA_BEAM
					&& SkillRender->m_skill != SKILL_BLAST
					&& SkillRender->m_skill != SKILL_INFERNO
					&& SkillRender->m_skill != SKILL_TRIPLE_SHOT
					&& SkillRender->m_skill != SKILL_IMPALE
					&& SkillRender->m_skill != SKILL_MONSTER_AREA_ATTACK
					&& SkillRender->m_skill != SKILL_PENETRATION
					&& SkillRender->m_skill != SKILL_FIRE_SLASH
					&& SkillRender->m_skill != SKILL_FIRE_SCREAM)
				{
					if (SkillRender->m_skill != SKILL_DARK_SIDE)
					{

						this->SendDurationSkillAttack(lpObj, tObjNum, SkillRender->m_index);
					}
					else
					{
						this->SendRFSkillAttack(lpObj, tObjNum, SkillRender->m_index);
					}
				}
				else
				{
					this->SendMultiSkillAttack(lpObj, tObjNum, SkillRender->m_index);
				}
			}
		}
	}
}
//================ Attack Custom

void CFakeOnline::SendSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) // OK
{
	PMSG_SKILL_ATTACK_RECV pMsg;

	pMsg.header.set(0x19, sizeof(pMsg));

	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);

	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	pMsg.dis = 0;
	lpObj->IsFakeTimeLag = GetTickCount();
	gSkillManager.CGSkillAttackRecv(&pMsg, lpObj->Index);
}

void CFakeOnline::SendMultiSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) // OK
{
	this->SendDurationSkillAttack(lpObj, aIndex, SkillNumber);

	BYTE send[256];

	PMSG_MULTI_SKILL_ATTACK_RECV pMsg;

	pMsg.header.set(PROTOCOL_CODE4, sizeof(pMsg));

	int size = sizeof(pMsg);

	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);

	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);

	pMsg.x = (BYTE)lpObj->X;

	pMsg.y = (BYTE)lpObj->Y;

	pMsg.serial = 0;

	pMsg.count = 0;

	PMSG_MULTI_SKILL_ATTACK info;

	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 /*|| lpObj->VpPlayer2[n].type != OBJECT_MONSTER*/)
		{
			continue;
		}

		int index = lpObj->VpPlayer2[n].index;

		if (gSkillManager.CheckSkillTarget(lpObj, index, aIndex, lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}

		if (gSkillManager.CheckSkillRadio(SkillNumber, lpObj->X, lpObj->Y, gObj[index].X, gObj[index].Y) == 0)
		{
			continue;
		}

		info.index[0] = SET_NUMBERHB(index);

		info.index[1] = SET_NUMBERLB(index);

		info.MagicKey = 0;

		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);

		if (CHECK_SKILL_ATTACK_COUNT(pMsg.count) == 0)
		{
			break;
		}
	}

	pMsg.header.size = size;

	memcpy(send, &pMsg, sizeof(pMsg));
	lpObj->IsFakeTimeLag = GetTickCount();
	gSkillManager.CGMultiSkillAttackRecv((PMSG_MULTI_SKILL_ATTACK_RECV*)send, lpObj->Index, 0);
}

void CFakeOnline::SendDurationSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) // OK
{
	CSkill* lpSkill = gSkillManager.GetSkill(lpObj, SkillNumber);

	if (lpSkill == 0)
	{
		return;
	}

	BYTE x = (BYTE)gObj[aIndex].X;
	BYTE y = (BYTE)gObj[aIndex].Y;
	BYTE dir = (gSkillManager.GetSkillAngle(gObj[aIndex].X, gObj[aIndex].Y, lpObj->X, lpObj->Y) * 255) / 360;
	BYTE angle = (gSkillManager.GetSkillAngle(lpObj->X, lpObj->Y, gObj[aIndex].X, gObj[aIndex].Y) * 255) / 360;

	lpObj->IsFakeTimeLag = GetTickCount();

	if (FakeOnlineNeedSkillVisual(lpObj, lpSkill) != 0)
	{
		lpObj->ActionState.Attack = 1;
		lpObj->ActionState.Emotion = 0;
		lpObj->ActionState.EmotionCount = 0;
		gSkillManager.GCDurationSkillAttackSend(lpObj, lpSkill->m_index, x, y, dir);
	}

	if (lpObj->Type == OBJECT_USER && gSkillManager.CheckSkillDelay(lpObj, lpSkill->m_index) == 0)
	{
		return;
	}

	if (lpObj->Type == OBJECT_USER && gSkillManager.CheckSkillRequireWeapon(lpObj, lpSkill->m_skill) == 0)
	{
		return;
	}

	if (lpObj->Type != OBJECT_USER || (gSkillManager.CheckSkillMana(lpObj, lpSkill->m_index) != 0 && gSkillManager.CheckSkillBP(lpObj, lpSkill->m_index) != 0))
	{
		if (gSkillManager.RunningSkill(lpObj->Index, aIndex, lpSkill, x, y, angle, 0) != 0 && lpObj->Type == OBJECT_USER)
		{
			lpObj->Mana -= ((gSkillManager.GetSkillMana(lpSkill->m_index) * lpObj->MPConsumptionRate) / 100);
			lpObj->BP -= ((gSkillManager.GetSkillBP(lpSkill->m_index) * lpObj->BPConsumptionRate) / 100);
			GCManaSend(lpObj->Index, 0xFF, (int)lpObj->Mana, lpObj->BP);
		}
	}
}

void CFakeOnline::SendRFSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) // OK
{
	CSkill* lpSkill = gSkillManager.GetSkill(lpObj, SkillNumber);

	if (lpSkill == 0)
	{
		return;
	}

	if (lpObj->RageFighterSkillTarget != aIndex || lpObj->RageFighterSkillIndex != lpSkill->m_index)
	{
		lpObj->RageFighterSkillTarget = aIndex;
		lpObj->RageFighterSkillIndex = lpSkill->m_index;
		lpObj->RageFighterSkillCount = 0;
	}

	if (FakeOnlineNeedSkillVisual(lpObj, lpSkill) != 0)
	{
		lpObj->ActionState.Attack = 1;
		lpObj->ActionState.Emotion = 0;
		lpObj->ActionState.EmotionCount = 0;
		gSkillManager.GCRageFighterSkillAttackSend(lpObj, lpSkill->m_index, aIndex, 1);
	}

	gAttack.Attack(lpObj, &gObj[aIndex], lpSkill, 0, 0, 0, 1, 0);
	gAttack.Attack(lpObj, &gObj[aIndex], lpSkill, 0, 0, 0, 2, 0);
	lpObj->IsFakeTimeLag = GetTickCount();
}
#endif
