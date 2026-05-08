#pragma once
// ---
#include "Protocol.h"
// ---
#define MAX_RANK  10
// ---
struct CUSTOM_RANKING
{
	int index;
	char Name[50];
	char Col1[50];
	char Col2[50];
};

struct CUSTOM_RANKING_DATA
{
	char szName[20];
	char szDate[20];
	BYTE Class;
	BYTE Vip;
	int Score;
	BYTE IsOnline;
	int Map;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//
struct SDHP_CUSTOM_RANKING_SEND
{
    PBMSG_HEAD header;
	WORD index;
	WORD type;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//
struct DATA_VIEWTOPRANKING
{
	PSWMSG_HEAD header;
	char NameChar[11];
	char GuildName[11];
	BYTE GuildMark[32];
	DWORD TongPoint;
	DWORD Reset;
	DWORD Level;
	DWORD MasterLevel;
	BYTE Item[12][16];
	int aIndex;
	BYTE Class;
};

struct REQUESTINFO_CHARTOP
{
	PSBMSG_HEAD header;
	char NameChar[11];
	int aIndex;
};

struct SDHP_CUSTOM_RANKING_RECV
{
	PWMSG_HEAD header; 
	int index;
	int type;
	int count;
};

//**********************************************//
//********** GameServer -> Cliente    **********//
//**********************************************//

struct PMSG_CUSTOM_RANKING_SEND
{
	PSWMSG_HEAD header; 
	int RankIndex;
	char rankname[50];
	char col1[50];
	char col2[50];
	int count;
};

struct PMSG_CUSTOM_RANKING_COUNT_SEND
{
	PSWMSG_HEAD header; // C2:C1
	int count;
	char RankNames[MAX_RANK][30];
};

//**********************************************//
//********** Cliente -> GameServer    **********//
//**********************************************//

struct PMSG_CUSTOM_RANKING_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
};

struct PMSG_CUSTOM_RANKING_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
	BYTE type;
};
// ---
class CCustomRanking
{
public:
	void Load(char* path);
	void GCReqRanking(int Index,PMSG_CUSTOM_RANKING_RECV* pMsg);
	void GCReqRankingCount(int Index,PMSG_CUSTOM_RANKING_COUNT_RECV* lpMsg);
	// ---
	int GetRankIndex(int aIndex);
	void CheckUpdate(LPOBJ lpObj);
	void GDCustomRankingRecv(BYTE* ReceiveBuffer);
	void CGetInfoCharTop(REQUESTINFO_CHARTOP* lpMsg, int aIndex);
	void RecvInfoCharTop(DATA_VIEWTOPRANKING* lpMsg);

private:
	int m_count;
	// ---
	CUSTOM_RANKING r_Data[MAX_RANK];
};
extern CCustomRanking gCustomRanking;
// ---
















