// CustomHealthBar.h: interface for the CCustomHealthBar class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Protocol.h"

struct MESSAGE_INFO_VONGQUAY {
  int Index;
  char Message[256];
};

struct DATA_VONGQUAYITEM {
  float SizeBMD;
  float PosX;
  float PosY;
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
  int Rate;
  int Star;
  int Quantity;
};

struct DATA_VONGQUAY {
  int IndexVongQuay;
  int IndexItemYC;
  int WC;
  int WP;
  int GP;
  int Count;
  char NameVongQuay[90];
  std::vector<DATA_VONGQUAYITEM> ListItemNhan;
};

struct DATA_VONGQUAY_TICHLUY {
  int RequiredSpin;
  DATA_VONGQUAYITEM Item;
};

//===List VONG QUAY
struct PMSG_VONGQUAY_SEND {
  PSWMSG_HEAD header; // C2:F3:E2
  BYTE count;
};

struct ListVongQuaySend {
  int IndexVongQuay;
  char Name[30];
};

//===List THuowng
struct PMSG_YCVONGQUAY_SEND {
  PSWMSG_HEAD header; // C2:F3:E2
  BYTE count;
  int IndexYC;
  int CountItem;
  int WCYC;
  int WPYC;
  int GPYC;
};

struct LISTITEMVONGQUAY_SENDINFO {
  float SizeBMD;
  float PosX;
  float PosY;
  short Index;
  BYTE Dur;
  BYTE Item[12];
  int PeriodTime;
  int Star;
  int Quantity;
};

struct PMSG_VONGQUAY_TICHLUY_SEND {
  PSWMSG_HEAD header; // C2:D3:8D
  BYTE count;
  DWORD DiemTichLuy;
  DWORD NhanThuongMask;
  DWORD ResetTichLuyTime;
};

struct LISTVONGQUAY_TICHLUY_SENDINFO {
  int RequiredSpin;
  float SizeBMD;
  float PosX;
  float PosY;
  short Index;
  BYTE Dur;
  BYTE Item[12];
  int PeriodTime;
};

struct XULY_CGPACKET_VONGQUAY {
  PSBMSG_HEAD header; // C3:F3:03
  DWORD StartRoll;
  DWORD IndexWin;
};

struct XULY_CGPACKET_SOLAN {
  PSBMSG_HEAD header; // C3:F3:03
  DWORD ThaoTac;
  DWORD SoLan;
};

struct XULY_CGPACKET_NHANTICHLUY {
  PSBMSG_HEAD header; // C1:D3:8D
  DWORD Index;
};

class CCustomVongQuay {
public:
  CCustomVongQuay();
  virtual ~CCustomVongQuay();
  void Init();
  void LoadFileXML(char *FilePath);
  void UserSendClientInfo(int aIndex);
  void SendListNhanThuong(int aIndex, int VongQuaySo);
  void SendTichLuyInfo(int aIndex);
  void ActionVongQuay(int aIndex, int MocNap, int solan);
  void MakeItem(int aIndex, int type);
  void ClaimTichLuyReward(int aIndex, int rewardIndex);
  void CheckTichLuyReset(int aIndex);

private:
  int Enable;
  int Firework;
  int Notice;
  int ResetTichLuyTime;
  std::map<int, MESSAGE_INFO_VONGQUAY> m_MessageInfoBP;
  std::map<int, DATA_VONGQUAY> m_DataVongQuay;
  std::vector<DATA_VONGQUAY_TICHLUY> m_DataTichLuy;
  DWORD GetCurrentTichLuyResetKey();
  char *GetMessage(int index);
};

extern CCustomVongQuay gCustomVongQuay;
