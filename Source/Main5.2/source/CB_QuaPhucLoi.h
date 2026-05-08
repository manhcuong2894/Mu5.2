#pragma once
#include "WSclient.h"

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
	struct CGPACKET_SENDRECV_B
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD ThaoTac;
		time_t server_time;
	};
	struct CGPACKET_SENDRECV_A
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE Type;
		BYTE Index;
	};
	struct CGPACKET_SENDRECV
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD ThaoTac;
		char GiftCode[30];

	};
	enum TYPEACTION_RECV
	{
		eGetInfoCode,
		eAccepGift,
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

	CB_QuaPhucLoi();
	~CB_QuaPhucLoi();
	void CB_QuaPhucLoi::DrawWindow();
	bool CB_QuaPhucLoi::OpenWindow();
	void CB_QuaPhucLoi::DrawPageDef(float X, float Y, float W, float H);
	void CB_QuaPhucLoi::DrawPageGift(float X, float Y, float W, float H);
	void CB_QuaPhucLoi::DrawPageListItem(float X, float Y, float W, float H);
	std::vector<std::string> m_ButtoNText;
	int StatePage;
	void CB_QuaPhucLoi::RecvClientButton(BYTE* aRecv);
	void CB_QuaPhucLoi::RecvDataInfo(BYTE* lpMsg);

	std::vector< DATALIST_QUAPL> m_DataListPhucLoi;
	int m_DataListCoin;
	int m_DataListMocDaNhan;
private:

};

extern CB_QuaPhucLoi* gCB_QuaPhucLoi;
