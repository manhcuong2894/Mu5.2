#pragma once
#include "WSclient.h"
#define SET_NUMBERHB(x) ((BYTE)((DWORD)(x)>>(DWORD)8))
#define SET_NUMBERLB(x) ((BYTE)((DWORD)(x)&0xFF))
#define SET_NUMBERHW(x) ((WORD)((DWORD)(x)>>(DWORD)16))
#define SET_NUMBERLW(x) ((WORD)((DWORD)(x)&0xFFFF))
#define SET_NUMBERHDW(x) ((DWORD)((QWORD)(x)>>(QWORD)32))
#define SET_NUMBERLDW(x) ((DWORD)((QWORD)(x)&0xFFFFFFFF))

#define MAKE_NUMBERW(x,y) ((WORD)(((BYTE)((y)&0xFF))|((BYTE)((x)&0xFF)<<8)))
#define MAKE_NUMBERDW(x,y) ((DWORD)(((WORD)((y)&0xFFFF))|((WORD)((x)&0xFFFF)<<16)))
#define MAKE_NUMBERQW(x,y) ((QWORD)(((DWORD)((y)&0xFFFFFFFF))|((DWORD)((x)&0xFFFFFFFF)<<32)))
//===Move Item
struct PMSG_ITEM_MOVE_RECV
{
	PSBMSG_HEAD h;
	BYTE sFlag;
	BYTE tFlag;
	BYTE Source;
	BYTE Target;
};
struct XULY_CGPACKET
{
	PSBMSG_HEAD header; // C3:F3:03
	DWORD ThaoTac;
};
#if(CUSTOM_CHANGEITEM)
struct INFO_CHANGEITEM_CLIENT
{
	PSWMSG_HEAD header;
	BYTE ActiveMix;
	BYTE ItemChinh[16];
	BYTE ItemPhu[3][16];
	BYTE ItemKetQua[12][16];
	int WC;
	int WP;
	int GP;
	int RequiredLevel;
	int RequiredSkill;
	int RequiredLuck;
	int RequiredOption;
	int RequiredExc;
};
#endif
struct SEND_COUNTLIST
{
	PSWMSG_HEAD header;
	int Count;
	BYTE Type;
};


struct RANK_INFO_SEND
{
	char NameRank[128];
};

struct PMSG_HEALTH_BAR_BOSS_RECV
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
};

extern std::vector <std::string> m_DataSelectNameTop;
BOOL ProtocolCoreEx(BYTE head, BYTE* lpMsg, int size, int key);


void CGReqInfoCharTop(const char* Name);
void GCRecvInfoCharTop(DATA_VIEWTOPRANKING_TO_CLIENT* lpMsg);

