
#include "stdafx.h"
#include "CustomChoTroi.h"
#include "DataServerProtocol.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "Util.h"
#if (CUSTOM_CHOTROI)
CChoTroi g_CustomChoTroi;

static void ChoTroiExecSchemaQuery(const char *query) {
  gQueryManager.ExecQuery((char *)query);
  gQueryManager.Fetch();
  gQueryManager.Close();
}

static void ChoTroiEnsureColumn(const char *columnName,
                                const char *columnDefinition) {
  char query[1024];

  sprintf_s(query, sizeof(query),
            "IF COL_LENGTH('dbo.ItemMarketData','%s') IS NULL "
            "ALTER TABLE [dbo].[ItemMarketData] ADD [%s] %s",
            columnName, columnName, columnDefinition);

  ChoTroiExecSchemaQuery(query);
}

static void ChoTroiNormalizeItemBuffer(BYTE *item, int itemLength,
                                       int itemBufferSize) {
  if (item == 0 || itemBufferSize <= 0) {
    return;
  }

  if (itemLength == 12 && itemBufferSize >= MARKET_ITEM_BUFFER) {
    BYTE clientItem[12];
    memcpy(clientItem, item, sizeof(clientItem));

    memset(item, 0xFF, itemBufferSize);
    item[0] = clientItem[0];
    item[1] = clientItem[1];
    item[2] = clientItem[2];
    item[3] = 0;
    item[4] = 0;
    item[5] = 0;
    item[6] = 0;
    item[7] = clientItem[3];
    item[8] = clientItem[4];
    item[9] = clientItem[5];
    item[10] = clientItem[6];
    memcpy(&item[11], &clientItem[7], 5);
    return;
  }

  if (itemLength < 0) {
    itemLength = 0;
  } else if (itemLength > itemBufferSize) {
    itemLength = itemBufferSize;
  }

  if (itemLength < itemBufferSize) {
    memset(&item[itemLength], 0xFF, itemBufferSize - itemLength);
  }
}

CChoTroi::CChoTroi() {}

CChoTroi::~CChoTroi() {}

void CChoTroi::Init() {}

void CChoTroi::Load() {}

bool CChoTroi::Connect() {
  this->CreateTable();

  return true;
}

void CChoTroi::CreateTable() {
  ChoTroiExecSchemaQuery(
      "IF OBJECT_ID('dbo.ItemMarketData','U') IS NULL "
      "CREATE TABLE [dbo].[ItemMarketData]( [ID] [int] IDENTITY(1,1) NOT "
      "NULL) ON [PRIMARY]");

  ChoTroiEnsureColumn("Account", "[varchar](10) NULL");

#if (MARKET_NAME_DEV)

  ChoTroiEnsureColumn("Name", "[varchar](10) NULL");

#endif

  ChoTroiEnsureColumn("PriceType", "INT not null default(0)");

  ChoTroiEnsureColumn("PriceValue", "INT not null default(0)");

  ChoTroiEnsureColumn("Status", "INT not null default(0)");

#if (MARKET_FILTER_DEV)

  ChoTroiEnsureColumn("FilterType", "INT not null default(0)");

  ChoTroiEnsureColumn("FilterLevel", "INT not null default(0)");

  ChoTroiEnsureColumn("FilterLuck", "INT not null default(0)");

  ChoTroiEnsureColumn("FilterExl", "INT not null default(0)");

  ChoTroiEnsureColumn("FilterAnc", "INT not null default(0)");

#endif

  ChoTroiEnsureColumn("Date", "[varchar](20) NULL");

  ChoTroiEnsureColumn("Item", "[varbinary](16) NULL");

  ChoTroiEnsureColumn("TypeItem", "INT not null default(0)");

  ChoTroiEnsureColumn("Time", "INT not null default(0)");

  ChoTroiEnsureColumn("Pass", "INT NULL");

  ChoTroiExecSchemaQuery(
      "DECLARE @ConstraintName sysname; "
      "SELECT @ConstraintName = dc.name FROM sys.default_constraints dc "
      "INNER JOIN sys.columns c ON c.default_object_id = dc.object_id "
      "WHERE dc.parent_object_id = OBJECT_ID('dbo.ItemMarketData') AND "
      "c.name = 'Pass'; "
      "IF @ConstraintName IS NOT NULL EXEC('ALTER TABLE [dbo].[ItemMarketData] "
      "DROP CONSTRAINT [' + @ConstraintName + ']')");

  ChoTroiExecSchemaQuery(
      "ALTER TABLE [dbo].[ItemMarketData] ALTER COLUMN [Pass] INT NULL");

}

void CChoTroi::Protocol(BYTE protoNum, BYTE *aRecv, int uIndex) {
  switch (protoNum) {
  case 0x00:
    this->GDReqItemListPage((SDHP_REQ_MARKET_ITEM *)aRecv, uIndex);
    break;
  case 0x01:
    this->GDReqItemSell((SDHP_REQ_MARKET_SELL *)aRecv, uIndex);
    break;
  case 0x02:
    this->GDReqBuyItem((SDHP_REQ_MARKET_BUY *)aRecv, uIndex);
    break;
#if (MARKET_FILTER_DEV)
  case 0x03:
    this->GDReqItemStatus((SDHP_REQ_MARKET_STATUS *)aRecv, uIndex);
    break;
#endif
  }
}

void CChoTroi::GDReqItemList(SDHP_REQ_MARKET_ITEM *lpMsg, int uIndex) {
#if (MARKET_PRICE_DEV)

  this->GDReqItemListPage(lpMsg, uIndex);

#else

  char szQuery[256];

  int iReturnCode = 0;

  int iCount = 0;

  SDHP_ANS_MARKET_ITEM pMsg;

  memset(&pMsg, 0, sizeof(pMsg));

  pMsg.h.set(0xFE, 0x00, sizeof(pMsg));

  pMsg.Result = 1;

  pMsg.aIndex = lpMsg->aIndex;

  // --

  char szPriceType[256];

  memset(szPriceType, 0, sizeof(szPriceType));

  if (lpMsg->PriceType == eMarketPriceTypeZen) {
    sprintf(szPriceType, "AND PriceType = %d ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceTypeWcoin) {
    sprintf(szPriceType, "AND PriceType = %d ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceTypeCredit) {
    sprintf(szPriceType, "AND PriceType = %d ", lpMsg->PriceType);
  }

  // --

#if (MARKET_FILTER_DEV)

  char szFilter[256];

  memset(szFilter, 0, sizeof(szFilter));

  if (lpMsg->Filter.FilterType < 16) {
    sprintf(szFilter, "AND FilterType = %d ", lpMsg->Filter.FilterType);
  }

  if (lpMsg->Filter.FilterLevel > 0 && lpMsg->Filter.FilterLevel <= 15) //=(
  {
    char szLevel[256];

    memset(szLevel, 0, sizeof(szLevel));

    sprintf(szLevel, "AND FilterLevel >= %d AND FilterLevel <= 15 ",
            lpMsg->Filter.FilterLevel);

    strcat(szFilter, szLevel);
  }

  if (lpMsg->Filter.FilterLuck) {
    strcat(szFilter, "AND FilterLuck = 1 ");
  }

  if (lpMsg->Filter.FilterExl) {
    strcat(szFilter, "AND FilterExl = 1 ");
  }

  if (lpMsg->Filter.FilterAnc) {
    strcat(szFilter, "AND FilterAnc = 1 ");
  }

  sprintf(szQuery,
          "SELECT TOP %d ID, Account, Name, PriceType, PriceValue, Item, "
          "DATALENGTH(Item) AS ItemLen FROM "
          "ItemMarketData WHERE Status = 0 AND Item IS NOT NULL AND "
          "DATALENGTH(Item) BETWEEN 12 AND 16 %s %s ORDER BY ID DESC",
          MARKET_ITEM_MAX, szFilter, szPriceType);

#else

  sprintf(szQuery,
          "SELECT TOP %d ID, Account, PriceType, PriceValue, Item, "
          "DATALENGTH(Item) AS ItemLen FROM "
          "ItemMarketData WHERE Status = 0 AND Item IS NOT NULL AND "
          "DATALENGTH(Item) BETWEEN 12 AND 16 ORDER BY ID DESC",
          MARKET_ITEM_MAX); // <<== Arg

#endif

  if (!gQueryManager.ExecQuery(szQuery)) {
    gQueryManager.Close();

    pMsg.Result = 0;
  } else {
    short sqlReturn = gQueryManager.Fetch();

    while (sqlReturn != SQL_NO_DATA && sqlReturn != SQL_NULL_DATA) {
      iReturnCode = gQueryManager.GetResult(0);

      if (iReturnCode < 0) {
        break;
      }

      char szSeller[MARKET_NAME_LEN];
      gQueryManager.GetAsString("Name", szSeller, sizeof(szSeller));

      pMsg.Data[iCount].Name[MARKET_NAME_LEN - 1] = 0;

      memcpy(pMsg.Data[iCount].Name, szSeller, MARKET_NAME_LEN - 1);

      pMsg.Data[iCount].PriceType = gQueryManager.GetAsInteger("PriceType");

      pMsg.Data[iCount].Price = gQueryManager.GetAsInteger("PriceValue");

      // --

      int id = gQueryManager.GetAsInteger("ID");

      pMsg.Data[iCount].ID = id;

      gQueryManager.GetAsBinary("Item", pMsg.Data[iCount].Item,
                                sizeof(pMsg.Data[iCount].Item));
      ChoTroiNormalizeItemBuffer(
          pMsg.Data[iCount].Item, gQueryManager.GetAsInteger("ItemLen"),
          sizeof(pMsg.Data[iCount].Item));

      // --

      iCount++;

      if (iCount >= MARKET_ITEM_MAX) {
        break;
      }

      sqlReturn = gQueryManager.Fetch();
    }

    gQueryManager.Close();
  }

  gSocketManager.DataSend(uIndex, (BYTE *)&pMsg, sizeof(pMsg));

#endif
}

void CChoTroi::GDReqItemSell(SDHP_REQ_MARKET_SELL *lpMsg, int uIndex) {
  char szQuery[512];

  char szAccountID[11] = {0};
  strncpy(szAccountID, lpMsg->Account, 10);

  char szName[11];
  memset(szName, 0, sizeof(szName));
  memcpy(szName, lpMsg->Name, sizeof(szName) - 1);

  LogAdd(LOG_BLUE,
         "[ChoTroiDebug] DS recv EE/01 aIndex=%d account=%s name=%s price=%d "
         "coin=%d typeItem=%d day=%d pass=%d",
         lpMsg->aIndex, szAccountID, szName, lpMsg->Price, lpMsg->PriceType,
         lpMsg->TypeItem, lpMsg->ItemDay, lpMsg->Pass);

  int itemDay = lpMsg->ItemDay;
  if (itemDay < 1) {
    itemDay = 1;
  } else if (itemDay > 30) {
    itemDay = 30;
  }

  int TimeHSD = (int)(time(0) + ((itemDay * 86400))); // HSD
  int itemPass = lpMsg->Pass;
  char szPassValue[16];
  if (itemPass >= 0) {
    sprintf(szPassValue, "%d", itemPass);
  } else {
    strcpy(szPassValue, "NULL");
  }

  // LogAdd(LOG_RED, "GDReqItemSell Day %d Time %d Pass %d", lpMsg->ItemDay,
  // TimeHSD,lpMsg->Pass);
  sprintf(szQuery,
          "INSERT INTO ItemMarketData (Account, PriceType, PriceValue, Date, "
          "TypeItem, Name, Time,Pass) VALUES ('%s', %d, %d, '%s', %d, '%s', "
          "%d, %s)",
          szAccountID, lpMsg->PriceType, lpMsg->Price, "19.12.2020",
          lpMsg->TypeItem, szName, TimeHSD, szPassValue);

  if (!gQueryManager.ExecQuery(szQuery)) {
    LogAdd(LOG_RED, "[ChoTroiDebug] DS stop: insert failed account=%s name=%s",
           szAccountID, szName);
    LogAdd(LOG_RED, "[ChoTroi] Loi Sell Item len cho !!");
    gQueryManager.Close();
    return;
  }

  gQueryManager.Close();

  sprintf(szQuery,
          "SELECT TOP 1 ID FROM ItemMarketData WHERE Account = '%s' AND "
          "PriceType = %d AND PriceValue = %d AND TypeItem = %d AND Name = "
          "'%s' AND Time = %d AND Item IS NULL ORDER BY ID DESC",
          szAccountID, lpMsg->PriceType, lpMsg->Price, lpMsg->TypeItem, szName,
          TimeHSD);

  if (!gQueryManager.ExecQuery(szQuery) ||
      gQueryManager.Fetch() == SQL_NO_DATA) {
    LogAdd(LOG_RED,
           "[ChoTroiDebug] DS stop: cannot find new row account=%s name=%s "
           "time=%d",
           szAccountID, szName, TimeHSD);
    LogAdd(LOG_RED, "[ChoTroi] Khong tim thay row moi de cap nhat item [%s][%s]",
           szAccountID, szName);
    gQueryManager.Close();
    return;
  }

  int id = gQueryManager.GetAsInteger("ID");

  gQueryManager.Close();

  // --

  sprintf(szQuery,
          "UPDATE ItemMarketData SET Item = ? WHERE ID = %d AND Account = '%s'",
          id, szAccountID);

  gQueryManager.BindParameterAsBinary(1, lpMsg->ItemData,
                                      sizeof(lpMsg->ItemData));

  if (!gQueryManager.ExecQuery(szQuery, id)) {
    LogAdd(LOG_RED, "[ChoTroiDebug] DS stop: blob update failed ID=%d", id);
    LogAdd(LOG_RED, "[ChoTroi] Loi cap nhat blob item [%s][%s] ID=%d",
           szAccountID, szName, id);
  } else {
    LogAdd(LOG_BLUE, "[ChoTroiDebug] DS save complete ID=%d account=%s name=%s",
           id, szAccountID, szName);
#if (MARKET_DEBUG)
    LogAdd(LOG_BLUE, "[ChoTroi] SaveItem OK ID=%d Account=%s Name=%s Len=%d",
           id, szAccountID, szName, (int)sizeof(lpMsg->ItemData));
#endif
  }

  gQueryManager.Close();

  // --

  // gSocketManager.DataSend(uIndex, (BYTE*)&pMsg, sizeof(pMsg));
}

void CChoTroi::GDReqBuyItem(SDHP_REQ_MARKET_BUY *lpMsg, int uIndex) {
  char szQuery[256];

  int iResult = 0;

  sprintf(szQuery,
          "Select Status from ItemMarketData WHERE ID = %d And Status = 0",
          lpMsg->ID);

  if (!gQueryManager.ExecQuery(szQuery) ||
      gQueryManager.Fetch() == SQL_NO_DATA) {
    iResult = 0;
    gQueryManager.Close();
    goto Exit;
  }

  gQueryManager.Close();

  if (lpMsg->Result == 1) {
    sprintf(szQuery, "UPDATE ItemMarketData SET Status = 1 WHERE ID = %d",
            lpMsg->ID);

    iResult = gQueryManager.ExecQuery(szQuery);

    gQueryManager.Close();
  }
  if (lpMsg->Result == 2) {
    sprintf(szQuery, "Delete FROM ItemMarketData WHERE ID = %d", lpMsg->ID);

    iResult = gQueryManager.ExecQuery(szQuery);

    gQueryManager.Close();
  }
Exit:
  // LogAdd(LOG_RED, "GDReqBuyItem %d", iResult);
  SDHP_ANS_MARKET_BUY pMsg;

  memset(&pMsg, 0, sizeof(pMsg));

  pMsg.h.set(0xFE, 0x02, sizeof(pMsg));

  pMsg.Result = iResult == 0 ? 3 : lpMsg->Result;

  pMsg.aIndex = lpMsg->aIndex;

  pMsg.ID = lpMsg->ID;

  gSocketManager.DataSend(uIndex, (BYTE *)&pMsg, sizeof(pMsg));
}

void DeleteItemCoin(int ID) {
  gQueryManager.ExecQuery("Delete FROM ItemMarketData WHERE ID = %d", ID);
  gQueryManager.Fetch();
  gQueryManager.Close();
}
void CChoTroi::GDReqItemStatus(SDHP_REQ_MARKET_STATUS *lpMsg,
                               int uIndex) // Get va Update List Item Da Ban
{

  BYTE send[4096];

  CBCUSTOM_LOAD_COUNT pMsg;

  pMsg.header.set(0xFE, 0x03, 0);

  int size = sizeof(pMsg);

  pMsg.count = 0;

  pMsg.aIndex = lpMsg->aIndex;

  char szAccountID[11] = {0};
  strncpy(szAccountID, lpMsg->Account, 10);

  SDHP_ANS_MARKET_STATUS info;

  if (gQueryManager.ExecQuery(
          "SELECT *, DATALENGTH(Item) AS ItemLen FROM ItemMarketData WHERE "
          "Account = '%s' AND Status = 1 AND Item IS NOT NULL AND "
          "DATALENGTH(Item) BETWEEN 12 AND 16",
          szAccountID) != 0) {
    while (gQueryManager.Fetch() != SQL_NO_DATA) {

      info.TypeItem = gQueryManager.GetAsInteger("TypeItem");

      info.PriceType = gQueryManager.GetAsInteger("PriceType");

      info.PriceValue = gQueryManager.GetAsInteger("PriceValue");

      gQueryManager.GetAsString("Account", info.Account, sizeof(info.Account));

      memset(info.ItemData, 0xFF, sizeof(info.ItemData));
      gQueryManager.GetAsBinary("Item", info.ItemData, sizeof(info.ItemData));
      ChoTroiNormalizeItemBuffer(info.ItemData,
                                 gQueryManager.GetAsInteger("ItemLen"),
                                 sizeof(info.ItemData));

      memcpy(&send[size], &info, sizeof(info));
      size += sizeof(info);

      pMsg.count++;
    }
    gQueryManager.Close();
    gQueryManager.ExecQuery(
        "Delete FROM ItemMarketData WHERE Account = '%s' AND Status = 1",
        szAccountID);
    gQueryManager.Close();
  } else {
    gQueryManager.Close();
    return;
  }

  pMsg.header.size[0] = SET_NUMBERHB(size);
  pMsg.header.size[1] = SET_NUMBERLB(size);

  memcpy(send, &pMsg, sizeof(pMsg));

  gSocketManager.DataSend(uIndex, send, size);
}

void CChoTroi::GDReqItemListPage(SDHP_REQ_MARKET_ITEM *lpMsg, int uIndex) {
  char szQuery[256];

  int iPageNumber = lpMsg->PageNumber;
  // --

  char szPriceType[256];
  int DayNow = (int)time(0);
  // LogAdd(LOG_RED, "GDReqItemListPage TypeHSD %d ", lpMsg->TypeHSD);
  if (lpMsg->TypeHSD == 1) // Delete item het hajn
  {

    gQueryManager.ExecQuery(
        "Delete ItemMarketData WHERE Status = 0 And Time < %d", DayNow);
    gQueryManager.Close();
  } else if (lpMsg->TypeHSD == 0) {
    DayNow = 0;
  }

  gQueryManager.ExecQuery(
      "Delete ItemMarketData WHERE Status = 0 AND (Item IS NULL OR "
      "DATALENGTH(Item) < 12 OR DATALENGTH(Item) > 16)");
  gQueryManager.Close();

  memset(szPriceType, 0, sizeof(szPriceType));

  if (lpMsg->PriceType == eMarketPriceWC) {
    sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceWP) {
    sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceGP) {
    sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceB) {
    sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceS) {
    sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
  }

  if (lpMsg->PriceType == eMarketPriceC) {
    sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
  }

  // --

  char szFilter[256];
  char szAccountID[11] = {0};
  strncpy(szAccountID, lpMsg->Account, 10);
  memset(szFilter, 0, sizeof(szFilter));

  // TypeItem = 1 Vũ Khí, Khiên
  // 	TypeItem = 2 Mũ
  // 	TypeItem = 3 Áo
  // 	TypeItem = 4 Quần
  // 	TypeItem = 5 Tay
  // 	TypeItem = 6 Chân
  // 	TypeItem = 7 Cánh
  // 	TypeItem = 8 Pet, sói, thú cưỡi
  // 	TypeItem = 9 Dây Chuyền
  // 	TypeItem = 10 Nhẫn
  if (lpMsg->GetTypeItem > 0) {
    if (lpMsg->GetTypeItem != 12) {
      int TypeItem = lpMsg->GetTypeItem;
      if (lpMsg->GetTypeItem == 11)
        TypeItem = 255;
      sprintf(szFilter, "AND TypeItem = '%d' ", TypeItem);
    } else {
      sprintf(szFilter, "AND Account = '%s' ", szAccountID);
    }
  }

  if (lpMsg->GetTypeItem == 12) {
    sprintf(szQuery,
            "SELECT TOP 100 ID, Account, Name, PriceType, PriceValue, "
            "Item,TypeItem,Time,ISNULL(Pass,-1) AS Pass,DATALENGTH(Item) AS ItemLen FROM "
            "ItemMarketData WHERE Status = 0 AND Item IS NOT NULL AND "
            "DATALENGTH(Item) BETWEEN 12 AND 16 %s %s ORDER BY ID DESC",
            szFilter, szPriceType);
  } else {
    sprintf(szQuery,
            "SELECT TOP 100 ID, Account, Name, PriceType, PriceValue, "
            "Item,TypeItem,Time,ISNULL(Pass,-1) AS Pass,DATALENGTH(Item) AS ItemLen FROM "
            "ItemMarketData WHERE Status = 0 And Item IS NOT NULL AND "
            "DATALENGTH(Item) BETWEEN 12 AND 16 AND Time > %d  %s %s ORDER BY "
            "ID DESC",
            DayNow, szFilter, szPriceType);
  }

  BYTE send[8192];

  CBCUSTOM_LOAD_COUNT pMsg;

  pMsg.header.set(0xFE, 0x00, 0);

  int size = sizeof(pMsg);

  pMsg.count = 0;

  pMsg.aIndex = lpMsg->aIndex;

  if (gQueryManager.ExecQuery(szQuery) != 0) {
    short i = gQueryManager.Fetch();

    while (i != SQL_NO_DATA && i != SQL_NULL_DATA) {
      int iReturnCode = gQueryManager.GetResult(0);

      if (iReturnCode < 0) {
        break;
      }
      MARKET_DATA info = {0};
      info.PriceType = gQueryManager.GetAsInteger("PriceType");
      info.Price = gQueryManager.GetAsInteger("PriceValue");
      info.ID = gQueryManager.GetAsInteger("ID");
      info.TypeItem = gQueryManager.GetAsInteger("TypeItem");
      info.TimeItemRaoBan = gQueryManager.GetAsInteger("Time") - (int)time(0);
      info.Pass = gQueryManager.GetAsInteger("Pass");

      memset(info.Item, 0xFF, sizeof(info.Item));
      gQueryManager.GetAsBinary("Item", info.Item, sizeof(info.Item));
      ChoTroiNormalizeItemBuffer(info.Item,
                                 gQueryManager.GetAsInteger("ItemLen"),
                                 sizeof(info.Item));
      gQueryManager.GetAsString("Name", info.Name, sizeof(info.Name));

      //			LogAdd(LOG_RED, "GDReqItemListPage %d  %s (%d)
      //%d", info.ID, info.Name, info.TimeItemRaoBan,info.Pass);

      if ((size + sizeof(info)) >= sizeof(send)) {
        break;
      }
      // --
      memcpy(&send[size], &info, sizeof(info));
      size += sizeof(info);
      pMsg.count++;

      i = gQueryManager.Fetch();
    }
    gQueryManager.Close();
  } else {
    gQueryManager.Close();
    return;
  }

  pMsg.header.size[0] = SET_NUMBERHB(size);
  pMsg.header.size[1] = SET_NUMBERLB(size);

  memcpy(send, &pMsg, sizeof(pMsg));

#if (MARKET_DEBUG)
  LogAdd(LOG_BLUE, "[ChoTroi] LoadList Account=%s Type=%d Price=%d Count=%d",
         szAccountID, lpMsg->GetTypeItem, lpMsg->PriceType, pMsg.count);
#endif

  gSocketManager.DataSend(uIndex, send, size);
  // LogAdd(LOG_RED, "GDReqItemListPage %d", pMsg.count);
}
#endif
