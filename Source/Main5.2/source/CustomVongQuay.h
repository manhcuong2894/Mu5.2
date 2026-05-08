#pragma once
#include "Protocol.h"

class CVongQuay {
public:
  struct PMSG_VONGQUAY_SEND {
    PSWMSG_HEAD header; // C2:F3:E2
    BYTE count;
  };

  struct ListVongQuaySend {
    int IndexVongQuay;
    char Name[30];
  };

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

  //==Struct Client
  struct INFO_VONGQUAY_LOCAL_ITEM {
    float SizeBMD;
    float PosX;
    float PosY;
    short Index;
    ITEM *Item;
    int Star;
    int Quantity;
  };

  struct INFO_VONGQUAY_LOCAL_TICHLUY {
    int RequiredSpin;
    float SizeBMD;
    float PosX;
    float PosY;
    short Index;
    ITEM *Item;
    int Star;
    int Quantity;
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

  CVongQuay();
  virtual ~CVongQuay();
  void Init();
  void DrawWindowVQ();
  void OpenVongQuay();
  bool IsSpinAnimating() const;
  bool ShouldBlockClose() const;
  int StartRollSau;
  int IndexItemSau;
  int IndexYC;
  int CountItem;
  int WCYC;
  int WPYC;
  int GPYC;
  DWORD DiemTichLuyVQ;
  DWORD NhanThuongTichLuyVQ;
  int ResetTichLuyTime;
  int TichLuyPage;
  std::vector<INFO_VONGQUAY_LOCAL_ITEM> ListItemVongQuay;
  std::vector<INFO_VONGQUAY_LOCAL_TICHLUY> ListTichLuyVongQuay;
  std::vector<ListVongQuaySend> DanhSachVongQuay;
  void GetListVQ(BYTE *Recv);
  void RecvListItemVQ(BYTE *Recv);
  void RecvTichLuyVQ(BYTE *Recv);
  void GetInfoVQ(BYTE *Recv);

private:
};

extern CVongQuay gVongQuay;
