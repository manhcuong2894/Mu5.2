#pragma once
#include "DefaultClassInfo.h"
#include "User.h"
#include "Protocol.h"
#include "SkillManager.h"
#if(CB_QUAPHUCLOI)
class CB_QuaPhucLoi
{
public:

	enum BUTTONENUM
	{
		eGiftCode,
		eQuaNapDau,
		eQuaNapNgay,
		eQuaNapThang,
		eQuaNapTichLuy,
		eQuaTieuPhiNgay,
		eQuaTieuPhiThang,
		eQuaTieuPhiTichLuy,
		eMaxNumButoon,
	};
	struct SDHP_SET_COIN_TIEU_PHI
	{
		PSBMSG_HEAD h; // C1:39
		WORD index;
		char account[11];
		DWORD CoinTieuPhi;
	};
	struct SDHP_SEND_GET_INFO_QPL
	{
		PSBMSG_HEAD h; // C1:39
		WORD index;
		DWORD  QuaNapDau;
		DWORD  QuaNapNgay;
		DWORD  QuaNapThang;
		DWORD  QuaNapTichLuy;
		DWORD  QuaTieuPhiNgay;
		DWORD  QuaTieuPhiThang;
		DWORD  QuaTieuPhiTichLuy;
	};
	struct MESSAGE_INFO
	{
		int Index;
		char Message[256];
	};

	struct DATA_CBITEM_LIST
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
	struct DATA_GIFT
	{
		int GiftIndex;
		std::vector<DATA_CBITEM_LIST> ListItemNhan;
	};
	struct DATA_QUAPHUCLOI_LIST
	{
		int DieuKienNhan;
		int GiftIndex;
	};
	//===List Send
	struct LISTITE_SENDINFO
	{
		float SizeBMD;
		int Count;
		short Index;
		BYTE Dur;
		BYTE Item[12];
		int  PeriodTime;
	};
	struct DATALIST_QUAPL
	{
		int DieuKienNhan;
		LISTITE_SENDINFO mListItem[6];
	};
	struct PMSG_CBLISTTHUONG_SEND
	{
		PSWMSG_HEAD header; // C2:F3:E2
		BYTE count;
		BYTE MocDaNhan;
		DWORD CoinCache;

	};

	struct CGPACKET_SENDRECV_A
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE Type;
		BYTE Index;
	};
	struct CGPACKET_SENDRECV_B
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD ThaoTac;
		time_t server_time;
	};
	CB_QuaPhucLoi();
	~CB_QuaPhucLoi();

	void RecvClient(BYTE* aRecv, int aIndex);
	void RecvClientNhanQua(BYTE* aRecv, int aIndex);
	void CB_QuaPhucLoi::SetCoinTieuPhi(int aIndex, int CoinTieuPhi);
	void CB_QuaPhucLoi::GDGetInfoQuaPhucLoi(int aIndex);
	void CB_QuaPhucLoi::DSGetInfoQuaPhucLoi(BYTE* Recv);

	void CB_QuaPhucLoi::LoadFileXML(char* FilePath);
	std::map<int, DATA_GIFT> m_DataGift;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaNapDau;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaNapNgay;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaNapThang;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaNapTichLuy;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaTieuPhiNgay;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaTieuPhiThang;
	std::vector<DATA_QUAPHUCLOI_LIST> List_QuaTieuPhiTichLuy;

	std::vector<DATA_QUAPHUCLOI_LIST> GetDataList(int Type);
	void CB_QuaPhucLoi::SaveMocDaNhan(LPOBJ lpObj, int Type, int DaNhan);
	int CB_QuaPhucLoi::GetCoinCache(LPOBJ lpObj, int Type);
	int CB_QuaPhucLoi::GetMocDaNhan(LPOBJ lpObj, int Type);
	void CB_QuaPhucLoi::ItemByteConvert(BYTE* lpMsg, DATA_CBITEM_LIST* Data);
	void CB_QuaPhucLoi::GDResetQuaPhucLoi(int aIndex, int Type);
	
	int EnableB[eMaxNumButoon];
private:
	//===Mess
	int Enable;
	std::map<int, MESSAGE_INFO> m_MessageInfoBP;
	char* GetMessage(int index);
};

extern CB_QuaPhucLoi gCB_QuaPhucLoi;
#endif