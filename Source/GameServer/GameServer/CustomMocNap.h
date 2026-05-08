#pragma once
#include "DefaultClassInfo.h"
#include "User.h"
#include "Protocol.h"
#include "SkillManager.h"
#include <map>
#include <vector>
#define eMessageBox				255
#if(CUSTOM_MOCNAP)
struct SDHP_MOCNAP_AUTO_REWARD_RECV;
struct SDHP_MOCNAP_PAYMENT_CREATE_RECV;

struct PMSG_MOCNAP_PAYMENT_CREATE_RECV
{
	PSBMSG_HEAD header; // C1:D3:9D
	int Amount;
};

struct PMSG_MOCNAP_PAYMENT_CREATE_SEND
{
	PSWMSG_HEAD header; // C2:D3:9D
	int Result;
	int Amount;
	QWORD OrderCode;
	char Message[128];
	char CheckoutUrl[512];
	char QrCode[2048];
	char BankBin[16];
	char AccountNumber[32];
	char AccountName[64];
	char Description[32];
};

struct MESSAGE_INFO_MocNap
{
	int Index;
	char Message[256];
};
struct DATA_CBITEMMOCNAP
{
	float SizeBMD;
	int Count;
	int IndexItem;
	int LvItem;
	int Dur;
	int Skill;
	int Luck;
	int Opt;
	int Exc;
	int Anc;
	int SK[MAX_SOCKET_OPTION];
	int SKBonus;
	int HSD;
	int Class[MAX_CLASS];
};
struct DATA_CBMOCNAP
{
	int IndexMocNap;
	int GiaTriNap;
	int WC;
	int WP;
	int GP;
	int Ruud;
	std::vector<DATA_CBITEMMOCNAP> ListItemNhan;
};
//===List Moc Nap
struct PMSG_CBMOCNAP_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
	int NhanMocNap;
	int TongNap;
	int RateNapThe;
	int RateApplyTo;
};

struct ListMocNapSend
{
	int IndexMocNap;
	int GiaTriNap;
};
//===List THuowng
struct PMSG_CBLISTTHUONG_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
	int WC;
	int WP;
	int GP;
	int Ruud;
};
struct LISTITEMMOCNAP_SENDINFO
{
	float SizeBMD;
	int Count;
	short Index;
	BYTE Dur;
	BYTE Item[12];
	int  PeriodTime;
};
class gBMocNap
{
public:
	gBMocNap();
	virtual ~gBMocNap();
	void LoadFileXML(char* FilePath);
	void MainProc();
	void UserSendClientInfo(int aIndex);
	void SendListNhanThuong(int aIndex, int MocNap);
	void NhanThuongMocNap(int aIndex, int MocNap);
	void DGMocNapAutoRewardRecv(SDHP_MOCNAP_AUTO_REWARD_RECV* lpMsg);
	void CreatePaymentLink(int aIndex, int amount);
	void DGMocNapPaymentCreateRecv(SDHP_MOCNAP_PAYMENT_CREATE_RECV* lpMsg);
private:
	//===Mess
	int Enable;
	int Firework;
	int Notice;
	int RateNapThe;
	int RateApplyTo;
	DWORD AutoRewardTick;
	std::map<int, MESSAGE_INFO_MocNap> m_MessageInfoBP;
	std::map<int, DATA_CBMOCNAP> m_DataMocNap;
	char* GetMessage(int index);
	void CalcAutoRewardCoin(int amount, int* wc, int* wp, int* gp);
};

extern gBMocNap gMocNap;

#endif
