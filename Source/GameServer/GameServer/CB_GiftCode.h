#pragma once
#include "DefaultClassInfo.h"
#include "Protocol.h"
#include "SkillManager.h"
#include "User.h"

#if (CB_CUSTOMGIFTCODE)
class CB_GiftCode {
public:
  struct MESSAGE_INFO {
    int Index;
    char Message[256];
  };
  struct DATA_CBITEM {
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
  struct DATA_LIST {
    int IndexMoc;
    char GiftCode[30];
    int WC;
    int WP;
    int GP;
    // int Ruud;
    std::vector<DATA_CBITEM> ListItemNhan;
    std::vector<std::string> ListAccount;
  };
  enum TYPEACTION_RECV {
    eGetInfoCode,
    eAccepGift,
  };
  struct CGPACKET_SENDRECV {
    PSBMSG_HEAD header; // C3:F3:03
    DWORD ThaoTac;
    char GiftCode[30];
  };
  struct SEND_DS_GETSTATUS {
    PSBMSG_HEAD h;
    WORD index;
    int Status;
    char account[11];
    char name[11];
    char giftcode[30];
    int IsAccountCheck;
  };
  //===List THuowng
  struct PMSG_CBLISTTHUONG_SEND {
    PSWMSG_HEAD header; // C2:F3:E2
    BYTE count;
    int WC;
    int WP;
    int GP;
    // int Ruud;
  };
  struct LISTITEM_SENDINFO {
    float SizeBMD;
    int Count;
    short Index;
    BYTE Dur;
    BYTE Item[12];
    int PeriodTime;
  };

  CB_GiftCode();
  ~CB_GiftCode();
  void LoadFileXML(char *FilePath);
  void RecvProtocol(BYTE *aRecv, int aIndex);
  void DSRecvProtocol(BYTE *aRecv);
  void SendListNhanThuong(int aIndex, const std::string &GiftCodeText);
  int Enable;

private:
  //===Mess

  int Firework;
  int Notice;
  std::map<int, MESSAGE_INFO> m_MessageInfoBP;
  std::map<std::string, DATA_LIST> m_DataGiftCode;

  char *GetMessage(int index);
  void ItemByteConvert(BYTE *lpMsg, DATA_CBITEM *Data);
};

extern CB_GiftCode gCB_GiftCode;
#endif