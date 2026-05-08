#pragma once
#include "User.h"
#include <map>
struct CB_CItem
{
	short m_Index;
	bool IsItem;

};
struct CB_OBJECTSTRUCT
{
	int Index;
	char Account[11];
	char Name[11];
	WORD Class;
	int Level;
	int LevelUpPoint;
	int Strength;
	int Dexterity;
	int Vitality;
	int Energy;
	int Leadership;
	int AddStrength;
	int AddDexterity;
	int AddVitality;
	int AddEnergy;
	int AddLeadership;
	int PhysiSpeed;
	int MagicSpeed;
	BYTE Live;
	DWORD State;
	BYTE Teleport;
	BYTE Transaction;
	DWORD InterfaceType;
	short X;
	short Y;
	BYTE Dir;
	BYTE Map;
	int RegenOk;
	CB_CItem Inventory[12];
	int Type;
	int Connected;
	int CloseType;
	int StatusNotCheck;
	char IpAddr[16];
};
typedef CB_OBJECTSTRUCT* CB_LPOBJ;
#pragma comment(lib, "APIGameGuard.lib")
#define APIGAMEGUARD_API __declspec(dllimport)


//===Notice
typedef void (*LogCallback)(int Type, const char* format);
APIGAMEGUARD_API void SetLogCallback(LogCallback callback);
//==Protocol Send
typedef void (*DataSendCallback)(int aIndex, BYTE* lpMsg, DWORD size);
APIGAMEGUARD_API void SetDataSendCallback(DataSendCallback callback);
//==Close Client
typedef void (*GCCloseClientSendCallback)(int aIndex, BYTE result);
APIGAMEGUARD_API void SetGCCloseClientSendCallback(GCCloseClientSendCallback callback);
//===Notice
typedef void (*GCNoticeSendCallback)(int aIndex, BYTE type, BYTE count, BYTE opacity, WORD delay, DWORD color, BYTE speed, const char* message);
APIGAMEGUARD_API void SetGCNoticeSendCallback(GCNoticeSendCallback callback);
//==Call RollbackItem
typedef void (*gObjInventoryRollbackCallback)(int aIndex);
APIGAMEGUARD_API void SetgObjInventoryRollbackCallback(gObjInventoryRollbackCallback callback);
//==Call Teleport
typedef void (*gObjTeleportCallback)(int aIndex, int map, int x, int y);
APIGAMEGUARD_API void SetgObjTeleportCallback(gObjTeleportCallback callback);


APIGAMEGUARD_API void CBAntihackLoadXML(void);
APIGAMEGUARD_API void CBAntihackSetUserZeroCache(CB_LPOBJ BlpObj, bool Clear);
APIGAMEGUARD_API bool CBAntihackTracking(CB_LPOBJ BlpObj);
APIGAMEGUARD_API bool CBAntihackPackerRecv(BYTE head, BYTE* recvlpMsg, int size, CB_LPOBJ lpObj, int encrypt, int serial);
APIGAMEGUARD_API void CBAntihackViewportProc(CB_LPOBJ BlpObj);
APIGAMEGUARD_API void CBAntihackResetCache(int aIndex);
APIGAMEGUARD_API bool CBAntihackBlockAttack(CB_LPOBJ BlpObj);

APIGAMEGUARD_API void CBAntihackAddIpAddress(char* IpAddress);
APIGAMEGUARD_API bool  CBAntihackCheckFlood(char* IpAddress);

APIGAMEGUARD_API int BMAX_SKILL;
APIGAMEGUARD_API int nMAX_OBJECT;
APIGAMEGUARD_API std::string nKeyEncDec;
APIGAMEGUARD_API std::string nServerNameGS;
//==XShield
APIGAMEGUARD_API void XShieldServerInit(HWND hwnd);
APIGAMEGUARD_API bool XShieldServerConnect(DWORD wMsg);
APIGAMEGUARD_API void XShieldServerMsgProc(WPARAM wParam, LPARAM lParam);
APIGAMEGUARD_API bool XShieldServerReconnect(HWND hwnd, DWORD wMsg);
APIGAMEGUARD_API std::string CBAntihackGetWinTitle();
class APIGameGuard
{


public:
	APIGameGuard();
	virtual ~APIGameGuard();
	CB_LPOBJ InputData(LPOBJ lpObj);
	bool APIGameGuard::Tracking(int aIndex, BYTE header, BYTE head, BYTE subcode);
	void APIGameGuard::ViewportProc(int aIndex);
	bool APIGameGuard::PacketRecv(BYTE head, BYTE* recvlpMsg, int size, int aIndex, int encrypt, int serial);
	bool APIGameGuard::BlockAttack(int aIndex);
	void APIGameGuard::ClearCache(int aIndex, bool Clear =0);
	void APIGameGuard::gObjInventoryRollback(int aIndex);
private:
	void APIGameGuard::UpdateObjectData(CB_LPOBJ obj, LPOBJ lpObj);
	std::map<int, CB_LPOBJ> objects_;
};

extern APIGameGuard gAPIGameGuard;
