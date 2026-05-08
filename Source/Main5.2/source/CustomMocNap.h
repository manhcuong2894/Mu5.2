#pragma once

#include "Protocol.h"
#include <vector>
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

struct PMSG_MOCNAP_PAYMENT_CREATE_REQ
{
	PSBMSG_HEAD header; // C1:D3:9D
	int Amount;
};

struct PMSG_MOCNAP_PAYMENT_CREATE_ANS
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

struct INFO_LOCAL_ITEM
{
	float SizeBMD;
	int Count;
	short Index;
	BYTE Item[12];
	int PeriodTime;
};
struct CBLISTPHANTHUONGMOCNAP_CLIENT
{
	int WC;
	int WP;
	int GP;
	int Ruud;
	std::vector<INFO_LOCAL_ITEM> ListItemMocNap;

	void Clear()
	{
		WC = 0;
		WP = 0;
		GP = 0;
		Ruud = 0;
		ListItemMocNap.clear();
	}
};
struct CBINFOMOCNAP_CLIENT
{
	int NhanMocNap;
	int TongNap;
	int RateNapThe;
	int RateApplyTo;
	std::vector<ListMocNapSend> DanhSachMocNap;
	void Clear()
	{
		NhanMocNap = 0;
		TongNap = 0;
		RateNapThe = 0;
		RateApplyTo = 0;
		DanhSachMocNap.clear();
	}
};


class MocNap
{
	CUITextInputBox* InputVND = NULL;
	char GetInputVND[14];

public:
	
	MocNap();
	virtual ~MocNap();
	void DrawWindow();
	void DrawXemMocNap();
	void OpenWindowMocNap();
	void GetListDonate(BYTE* Recv);
	void RecvListPhanThuong(BYTE* Recv);
	void RecvPaymentLink(BYTE* Recv);

	CBINFOMOCNAP_CLIENT mDataMocNapClient;
	CBLISTPHANTHUONGMOCNAP_CLIENT mDataListItemMocNapClient;
};

extern MocNap gMocNap;
