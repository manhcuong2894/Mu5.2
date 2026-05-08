// Log.h: interface for the CLog class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_LOG 99

enum eLogType
{
	LOG_GENERAL = 0,
	LOG_CHAT,
	LOG_COMMAND,
	LOG_TRADE,
	LOG_CONNECT,
	LOG_HACK,
	LOG_CASH_SHOP,
	LOG_CHAOS_MIX,
	LOG_ANTIFLOOD,
	LOG_BANK_JEWEL,
	LOG_THUMUA,
	LOG_ITEMBAGDROP,
	LOG_CONG_HUONG,
	LOG_ITEM_CHARACTER,
	LOG_SHOP,
	LOG_WC,
	LOG_MocNap,
	LOG_DEBUG,
	LOG_ITEM_HomDo,
	LOG_TRADEBOT,
	LOG_QUAPHUCLOI,
};

struct LOG_INFO
{
	BOOL Active;
	char Directory[256];
	int Day;
	int Month;
	int Year;
	char Filename[256];
	HANDLE File;
};

class CLog
{
public:
	CLog();
	virtual ~CLog();
	void AddLog(BOOL active,char* directory);
	void Output(eLogType type,char* text,...);
private:
	LOG_INFO m_LogInfo[MAX_LOG];
	int m_count;
};

extern CLog gLog;
