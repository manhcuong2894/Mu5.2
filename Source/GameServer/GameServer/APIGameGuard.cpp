
#include "StdAfx.h"
#include "APIGameGuard.h"
#include "Util.h"
#include "Protocol.h"
#include "User.h"
#include "Notice.h"
#include "Log.h"
#include "ServerInfo.h"
#include "SkillManager.h"

#if (ANTIHACK_GGNEW)
APIGameGuard gAPIGameGuard;

void LogAddHackCallBack(int Type, const char* message)
{
	if (Type == 15)
	{
		gLog.Output(LOG_HACK, "%s", message);
	}
	else
	{
		LogHackAdd((eLogColor)Type, "%s", message);
	}
}
void CBDataSendCallback(int aIndex, BYTE* lpMsg, DWORD size)
{
	DataSend(aIndex, lpMsg, size);
}
void CBGCCloseClientSendCallback(int aIndex, BYTE result)
{
	gObj[aIndex].CloseType = 2;
	gObj[aIndex].CloseCount = 5;
	//GCCloseClientSend(aIndex, result);
}
void CBGCNoticeSendCallback(int aIndex, BYTE type, BYTE count, BYTE opacity, WORD delay, DWORD color, BYTE speed, const char* message)
{
	gNotice.GCNoticeSend(aIndex, type, count, opacity, delay, color, speed, "%s",message);
}
void CBTeleportCallback(int aIndex, int map, int x, int y)
{
	gObjTeleport(aIndex, map, x, y);
}
void CBGObjInventoryRollback(int aIndex)
{
	gObjInventoryRollback(aIndex);
}

APIGameGuard::APIGameGuard()
{
	SetLogCallback(LogAddHackCallBack);
	SetDataSendCallback(CBDataSendCallback);
	SetGCCloseClientSendCallback(CBGCCloseClientSendCallback);
	SetGCNoticeSendCallback(CBGCNoticeSendCallback);
	SetgObjInventoryRollbackCallback(CBGObjInventoryRollback);
	SetgObjTeleportCallback(CBTeleportCallback);
	objects_.clear();
	nMAX_OBJECT = MAX_OBJECT;
	BMAX_SKILL = MAX_SKILL;

	
	//=====
	nKeyEncDec = ""; //Key quan trọng không lộ ra ngoài, liên hệ CuongBeo để lấy Key này
}


APIGameGuard::~APIGameGuard()
{
	objects_.clear();
}

void APIGameGuard::ClearCache(int aIndex,bool Clear)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (Clear == 0 && gObj[aIndex].StatusNotCheck != 0)
	{
		return;
	}
	//Antihack
	/*if (gObj[aIndex].StatusNotCheck != 0)
	{
		return;
	}*/
	LPOBJ lpObj = &gObj[aIndex];
	CBAntihackSetUserZeroCache(InputData(lpObj), Clear);
}
void APIGameGuard::UpdateObjectData(CB_LPOBJ obj, LPOBJ lpObj)
{
	obj->Index = lpObj->Index;
	memcpy(&obj->Account, lpObj->Account, sizeof(obj->Account));
	memcpy(&obj->Name, lpObj->Name, sizeof(obj->Name));
	memcpy(&obj->IpAddr, lpObj->IpAddr, sizeof(obj->IpAddr));
	obj->Class = lpObj->Class;
	obj->Level = lpObj->Level;
	obj->LevelUpPoint = lpObj->LevelUpPoint;
	obj->Strength = lpObj->Strength;
	obj->Dexterity = lpObj->Dexterity;
	obj->Vitality = lpObj->Vitality;
	obj->Energy = lpObj->Energy;
	obj->Leadership = lpObj->Leadership;
	obj->AddStrength = lpObj->AddStrength;
	obj->AddDexterity = lpObj->AddDexterity;
	obj->AddVitality = lpObj->AddVitality;
	obj->AddEnergy = lpObj->AddEnergy;
	obj->AddLeadership = lpObj->AddLeadership;
	obj->PhysiSpeed = lpObj->PhysiSpeed;
	obj->MagicSpeed = lpObj->MagicSpeed;
	obj->Live = lpObj->Live;
	obj->State = lpObj->State;
	obj->Teleport = lpObj->Teleport;
	obj->Transaction = lpObj->Transaction;
	obj->InterfaceType = lpObj->Interface.type;
	obj->X = lpObj->X;
	obj->Y = lpObj->Y;
	obj->Dir = lpObj->Dir;
	obj->Map = lpObj->Map;
	obj->RegenOk = lpObj->RegenOk;

	obj->Type = lpObj->Type;
	obj->Connected = lpObj->Connected;
	obj->CloseType = lpObj->CloseType;
	obj->StatusNotCheck = lpObj->StatusNotCheck;

	for (int n = 0; n < 12; n++)
	{
		obj->Inventory[n].m_Index = -1;
		obj->Inventory[n].IsItem = 0;
		if (lpObj->Inventory[n].IsItem())
		{
			obj->Inventory[n].m_Index = lpObj->Inventory[n].m_Index;
			obj->Inventory[n].IsItem = lpObj->Inventory[n].IsItem();
		}
	}
}
CB_LPOBJ APIGameGuard::InputData(LPOBJ lpObj)
{
	std::map<int, CB_LPOBJ>::iterator it = this->objects_.find(lpObj->Index);

	if (it == this->objects_.end())
	{
		CB_OBJECTSTRUCT* info = new CB_OBJECTSTRUCT;
		UpdateObjectData(info, lpObj);
		this->objects_.insert(std::make_pair(lpObj->Index, info));
		return info;
	}
	else
	{
		UpdateObjectData(it->second, lpObj);
		return it->second;
	}
	
}
bool APIGameGuard::Tracking(int aIndex, BYTE header, BYTE head, BYTE subcode)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}
	if (gObj[aIndex].Type != OBJECT_USER || gObj[aIndex].Connected != OBJECT_ONLINE)
	{
		return 0;
	}

	if (gObj[aIndex].StatusNotCheck != 0)
	{
		return 0;
	}
	LPOBJ lpObj = &gObj[aIndex];
	//CB_LPOBJ BlpObj = InputData(lpObj);
	if (header == 0xC3 || header == 0xC4)
	{
		return 0;
	}
	if (head == 0x11 || head == 0x19 || head == 0x1E || head == 0x4A || head == 0x4B || (head == 0xD3 && subcode == 0xAC))
	{
		return 0;
	}
	if (CBAntihackTracking(InputData(lpObj)) != 0)
	{
		LogAdd(LOG_RED, "[ANTIHACK] Tracking disconnect [%d][%s][%s] Header[%02X] Head[%02X] Sub[%02X]", aIndex, lpObj->Account, lpObj->Name, header, head, subcode);
		return 1;
	}
	return 0;
}

bool APIGameGuard::PacketRecv(BYTE head, BYTE* recvlpMsg, int size, int aIndex, int encrypt, int serial)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 1;
	}
	if (gObj[aIndex].Type != OBJECT_USER || gObj[aIndex].Connected != OBJECT_ONLINE)
	{
		return 1;
	}

	if (gObj[aIndex].StatusNotCheck != 0)
	{
		return 1;
	}
	LPOBJ lpObj = &gObj[aIndex];
	//CB_LPOBJ BlpObj = InputData(lpObj);
	CBAntihackPackerRecv(head, recvlpMsg, size, InputData(lpObj), encrypt, serial);
	return 1;
}

void APIGameGuard::ViewportProc(int aIndex)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	if (gObj[aIndex].Type != OBJECT_USER || (gObj[aIndex].Connected != OBJECT_ONLINE))
	{
		return;
	}

	if (gObj[aIndex].StatusNotCheck != 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];
	//CB_LPOBJ BlpObj = InputData(lpObj);
	CBAntihackViewportProc(InputData(lpObj));
	nServerNameGS = gServerInfo.m_ServerName;
}

bool APIGameGuard::BlockAttack(int aIndex)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 1;
	}

	if (gObj[aIndex].Type != OBJECT_USER || (gObj[aIndex].Connected != OBJECT_ONLINE))
	{
		return 1;
	}

	if (gObj[aIndex].StatusNotCheck != 0)
	{
		return 1;
	}
	LPOBJ lpObj = &gObj[aIndex];
	//CB_LPOBJ BlpObj = InputData(lpObj);
	return CBAntihackBlockAttack(InputData(lpObj));
}
#endif

