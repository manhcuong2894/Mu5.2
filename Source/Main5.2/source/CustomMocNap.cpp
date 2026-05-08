#include "StdAfx.h"
#include "CustomMocNap.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "ZzzInventory.h"
#include "ZzzOpenglUtil.h"
#include "TextClient.h"
#include "MapManager.h"
#include "ZzzInterface.h"
#include "NewUICommon.h"
#include "ZzzTexture.h"
#include <urlmon.h>

#pragma comment(lib, "urlmon.lib")

using namespace SEASON3B;

namespace
{
	const int kMocNapQrTexture = 168000;
	bool gMocNapQrTextureLoaded = false;
	DWORD gMocNapQrLastLoadTick = 0;
	DWORD gMocNapQrLastDownloadTick = 0;
	volatile LONG gMocNapQrDownloadState = 0;
	bool gMocNapQrDownloadStarted = false;
	bool gMocNapQrTextureNeedsReload = false;
	bool gMocNapQrHasPayment = false;
	char gMocNapBankShortName[64] = { 0 };
	char gMocNapQrAmount[14] = { 0 };
	char gMocNapQrData[2048] = { 0 };
	char gMocNapQrStatusText[128] = { 0 };
	char gMocNapPayOSBankBin[16] = { 0 };
	char gMocNapPayOSAccountNumber[32] = { 0 };
	char gMocNapPayOSAccountName[64] = { 0 };
	char gMocNapPayOSDescription[32] = { 0 };

	void HandleMocNapScrollWheel(CNewUIScrollBar* scrollBar, float x, float y, float width, float height)
	{
		if (!scrollBar || !SEASON3B::CheckMouseIn(x, y, width, height))
		{
			return;
		}

		if (MouseWheel < 0)
		{
			MouseWheel = 0;
			scrollBar->SetCurPos(scrollBar->GetCurPos() + 1);
		}
		else if (MouseWheel > 0)
		{
			MouseWheel = 0;
			scrollBar->SetCurPos(scrollBar->GetCurPos() - 1);
		}
	}

	bool TryLoadMocNapQrTexture()
	{
		if (gMocNapQrTextureLoaded)
		{
			return true;
		}

		DWORD tick = GetTickCount();
		if (gMocNapQrLastLoadTick != 0 && (tick - gMocNapQrLastLoadTick) < 5000)
		{
			return false;
		}

		gMocNapQrLastLoadTick = tick;
		gMocNapQrTextureLoaded = LoadBitmap("Custom\\MaQRNapThe.jpg", kMocNapQrTexture, GL_LINEAR, GL_CLAMP_TO_EDGE, false, false);
		return gMocNapQrTextureLoaded;
	}

	std::string MocNapUrlEncode(const char* text)
	{
		static const char hex[] = "0123456789ABCDEF";
		std::string out;
		if (!text)
		{
			return out;
		}

		for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p)
		{
			if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '-' || *p == '_' || *p == '.' || *p == '~')
			{
				out.push_back(static_cast<char>(*p));
			}
			else if (*p == ' ')
			{
				out.push_back('+');
			}
			else
			{
				out.push_back('%');
				out.push_back(hex[*p >> 4]);
				out.push_back(hex[*p & 0x0F]);
			}
		}

		return out;
	}

	std::string NormalizeMocNapAmount(const char* amount)
	{
		std::string normalized;
		if (!amount)
		{
			return normalized;
		}

		for (const unsigned char* p = reinterpret_cast<const unsigned char*>(amount); *p; ++p)
		{
			if (*p >= '0' && *p <= '9')
			{
				normalized.push_back(static_cast<char>(*p));
			}
		}

		while (normalized.size() > 1 && normalized[0] == '0')
		{
			normalized.erase(0, 1);
		}

		return normalized;
	}

	void SetMocNapQrStatus(const char* text)
	{
		gMocNapQrStatusText[0] = '\0';
		if (text && text[0] != '\0')
		{
			strncpy_s(gMocNapQrStatusText, sizeof(gMocNapQrStatusText), text, _TRUNCATE);
		}
	}

	bool ReadMocNapTextFile(const char* fileName, std::string& out)
	{
		FILE* file = fopen(fileName, "rb");
		if (!file)
		{
			return false;
		}

		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (size <= 0)
		{
			fclose(file);
			return false;
		}

		out.resize(size);
		bool ok = (fread(&out[0], 1, out.size(), file) == out.size());
		fclose(file);
		return ok;
	}

	std::string ExtractMocNapJsonString(const std::string& json, size_t start, const char* key)
	{
		std::string token = "\"";
		token += key;
		token += "\":\"";

		size_t keyPos = json.find(token, start);
		if (keyPos == std::string::npos)
		{
			return "";
		}

		size_t valueStart = keyPos + token.size();
		std::string value;
		for (size_t i = valueStart; i < json.size(); ++i)
		{
			if (json[i] == '"' && (i == valueStart || json[i - 1] != '\\'))
			{
				break;
			}
			value.push_back(json[i]);
		}

		return value;
	}

	std::string NormalizeMocNapBankKey(std::string text)
	{
		std::string out;
		out.reserve(text.size());
		for (char ch : text)
		{
			if (ch != ' ' && ch != '-' && ch != '_')
			{
				out.push_back(static_cast<char>(toupper(static_cast<unsigned char>(ch))));
			}
		}

		return out;
	}

	std::string FindMocNapBankShortName(const std::string& banksJson, const char* bankId)
	{
		std::string searchKey = NormalizeMocNapBankKey(bankId ? bankId : "");
		if (searchKey.empty())
		{
			return "";
		}

		size_t objectStart = banksJson.find('{');
		while (objectStart != std::string::npos)
		{
			size_t objectEnd = banksJson.find('}', objectStart);
			if (objectEnd == std::string::npos)
			{
				break;
			}

			std::string objectJson = banksJson.substr(objectStart, objectEnd - objectStart + 1);
			std::string bin = ExtractMocNapJsonString(objectJson, 0, "bin");
			std::string code = ExtractMocNapJsonString(objectJson, 0, "code");
			std::string shortName = ExtractMocNapJsonString(objectJson, 0, "shortName");
			if (shortName.empty())
			{
				shortName = ExtractMocNapJsonString(objectJson, 0, "short_name");
			}

			if (NormalizeMocNapBankKey(bin) == searchKey || NormalizeMocNapBankKey(code) == searchKey || NormalizeMocNapBankKey(shortName) == searchKey)
			{
				return shortName;
			}

			objectStart = banksJson.find('{', objectEnd + 1);
		}

		return "";
	}

	const char* GetMocNapBankLookupId()
	{
		return (gMocNapPayOSBankBin[0] != '\0') ? gMocNapPayOSBankBin : "";
	}

	void RefreshMocNapBankShortName()
	{
		gMocNapBankShortName[0] = '\0';

		const char* tempBanksJson = "Data\\Custom\\VietQrBanks.json";
		DeleteFileA(tempBanksJson);
		if (URLDownloadToFileA(NULL, "https://api.vietqr.io/v2/banks", tempBanksJson, 0, NULL) != S_OK)
		{
			return;
		}

		std::string banksJson;
		if (ReadMocNapTextFile(tempBanksJson, banksJson))
		{
			std::string shortName = FindMocNapBankShortName(banksJson, GetMocNapBankLookupId());
			if (!shortName.empty())
			{
				strncpy_s(gMocNapBankShortName, sizeof(gMocNapBankShortName), shortName.c_str(), _TRUNCATE);
			}
		}

		DeleteFileA(tempBanksJson);
	}

	const char* GetMocNapBankDisplayName()
	{
		if (gMocNapBankShortName[0] != '\0')
		{
			return gMocNapBankShortName;
		}

		return GetMocNapBankLookupId();
	}

	const char* GetMocNapAccountNumber()
	{
		return (gMocNapPayOSAccountNumber[0] != '\0') ? gMocNapPayOSAccountNumber : "";
	}

	const char* GetMocNapAccountName()
	{
		return (gMocNapPayOSAccountName[0] != '\0') ? gMocNapPayOSAccountName : "";
	}

	const char* GetMocNapDescriptionText()
	{
		return (gMocNapPayOSDescription[0] != '\0') ? gMocNapPayOSDescription : "";
	}

	const char* GetMocNapText(int index, const char* fallback)
	{
		if (index < 0 || index >= 50 || gTextClient.txtClient_Donate[index][0] == '\0' || _stricmp(gTextClient.txtClient_Donate[index], "Null") == 0)
		{
			return fallback;
		}

		return gTextClient.txtClient_Donate[index];
	}

	void DrawMocNapQrGenerating(float x, float y, float size)
	{
		bool blinkOn = ((GetTickCount() / 250) % 2) == 0;
		if (blinkOn)
		{
			gInterface->DrawBarForm(x, y, size, size, 1.0f, 1.0f, 1.0f, 0.25f);
		}

		SEASON3B::TextDraw((HFONT)g_hFontBold, x + 5.0f, y + (size / 2.0f) - 6.0f, blinkOn ? 0xFFDE26FF : 0xFFFFFFAA, 0x0, size, 0, 3, gTextClient.txtClient_Donate[16]);
	}

	bool WriteOZJFromJpeg(const char* sourceJpeg, const char* destOzj)
	{
		FILE* in = fopen(sourceJpeg, "rb");
		if (!in)
		{
			return false;
		}

		fseek(in, 0, SEEK_END);
		long size = ftell(in);
		fseek(in, 0, SEEK_SET);
		if (size <= 24)
		{
			fclose(in);
			return false;
		}

		std::vector<BYTE> data(size);
		if (fread(data.data(), 1, data.size(), in) != data.size())
		{
			fclose(in);
			return false;
		}
		fclose(in);

		if (data[0] != 0xFF || data[1] != 0xD8)
		{
			return false;
		}

		FILE* out = fopen(destOzj, "wb");
		if (!out)
		{
			return false;
		}

		fwrite(data.data(), 1, 24, out);
		fwrite(data.data(), 1, data.size(), out);
		fclose(out);
		return true;
	}

	DWORD WINAPI DownloadMocNapQrThread(LPVOID)
	{
		CreateDirectoryA("Data\\Custom", NULL);
		RefreshMocNapBankShortName();

		if (gMocNapQrData[0] == '\0')
		{
			SetMocNapQrStatus(gTextClient.txtClient_Donate[21]);
			gMocNapQrDownloadStarted = false;
			gMocNapQrHasPayment = false;
			InterlockedExchange(&gMocNapQrDownloadState, 0);
			return 0;
		}

		std::string url = "https://api.qrserver.com/v1/create-qr-code/?size=500x500&format=jpg&data=";
		url += MocNapUrlEncode(gMocNapQrData);

		const char* tempJpeg = "Data\\Custom\\MaQRNapThe.download.jpg";
		const char* targetOzj = "Data\\Custom\\MaQRNapThe.OZJ";

		bool success = false;
		DeleteFileA(tempJpeg);
		if (URLDownloadToFileA(NULL, url.c_str(), tempJpeg, 0, NULL) == S_OK)
		{
			if (WriteOZJFromJpeg(tempJpeg, targetOzj))
			{
				gMocNapQrTextureNeedsReload = true;
				success = true;
			}
		}
		DeleteFileA(tempJpeg);

		if (!success)
		{
			gMocNapQrDownloadStarted = false;
			gMocNapQrHasPayment = false;
			SetMocNapQrStatus(gTextClient.txtClient_Donate[22]);
		}
		else
		{
			SetMocNapQrStatus("");
		}
		InterlockedExchange(&gMocNapQrDownloadState, 0);
		return 0;
	}

	void StartMocNapQrDownloadThread()
	{
		gMocNapQrLastDownloadTick = GetTickCount();
		gMocNapQrDownloadStarted = true;
		HANDLE thread = CreateThread(NULL, 0, DownloadMocNapQrThread, NULL, 0, NULL);
		if (thread)
		{
			CloseHandle(thread);
		}
		else
		{
			gMocNapQrDownloadStarted = false;
			gMocNapQrHasPayment = false;
			SetMocNapQrStatus(gTextClient.txtClient_Donate[23]);
			InterlockedExchange(&gMocNapQrDownloadState, 0);
		}
	}

	void RequestMocNapQrGenerate(const char* amountText)
	{
		if (gMocNapQrDownloadState != 0)
		{
			return;
		}

		std::string amount = NormalizeMocNapAmount(amountText);
		__int64 amountValue = amount.empty() ? 0 : _atoi64(amount.c_str());
		if (amountValue <= 0 || amountValue > 2147483647)
		{
			gMocNapQrHasPayment = false;
			gMocNapQrTextureNeedsReload = true;
			SetMocNapQrStatus(gTextClient.txtClient_Donate[24]);
			DeleteFileA("Data\\Custom\\MaQRNapThe.OZJ");
			return;
		}

		if (InterlockedCompareExchange(&gMocNapQrDownloadState, 1, 0) != 0)
		{
			return;
		}

		strncpy_s(gMocNapQrAmount, sizeof(gMocNapQrAmount), amount.c_str(), _TRUNCATE);
		gMocNapQrDownloadStarted = false;
		gMocNapQrTextureNeedsReload = true;
		gMocNapQrHasPayment = false;
		gMocNapQrLastDownloadTick = 0;
		gMocNapQrLastLoadTick = 0;
		gMocNapQrData[0] = '\0';
		gMocNapPayOSBankBin[0] = '\0';
		gMocNapPayOSAccountNumber[0] = '\0';
		gMocNapPayOSAccountName[0] = '\0';
		gMocNapPayOSDescription[0] = '\0';
		SetMocNapQrStatus(gTextClient.txtClient_Donate[16]);
		DeleteFileA("Data\\Custom\\MaQRNapThe.OZJ");

		PMSG_MOCNAP_PAYMENT_CREATE_REQ pMsg;
		pMsg.header.set(0xD3, 0x9D, sizeof(pMsg));
		pMsg.Amount = (int)amountValue;
		DataSend((LPBYTE)&pMsg, pMsg.header.size);
	}
}

CNewUIScrollBar* ListMocNap = nullptr;
CNewUIScrollBar* ListItemNhan = nullptr;
int MaxListMocNapInPage = 4;
int MaxListItemInPage = 9;
int SelectTypeMocNap = -1;

MocNap gMocNap;

MocNap::MocNap()
{
	ZeroMemory(GetInputVND, sizeof(GetInputVND));
}

MocNap::~MocNap()
{
	if (InputVND)
	{
		delete InputVND;
		InputVND = NULL;
	}
}

void MocNap::OpenWindowMocNap()
{
	if (GetTickCount() < gInterface->Data[eWindowMocNap].EventTick + 300)
	{
		return;
	}

	gInterface->Data[eWindowMocNap].EventTick = GetTickCount();
	if (gInterface->Data[eWindowMocNap].OnShow)
	{
		gInterface->Data[eWindowMocNapList].Close();
	}

	if (!gInterface->Data[eWindowMocNap].OnShow)
	{
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x9C, sizeof(pMsg));
		pMsg.ThaoTac = 1;
		DataSend((LPBYTE)&pMsg, pMsg.header.size);
	}
	gInterface->Data[eWindowMocNap].OnShow ^= 1;
}

void MocNap::RecvListPhanThuong(BYTE* lpMsg)
{
	if (!lpMsg) return;

	mDataListItemMocNapClient.Clear();
	PMSG_CBLISTTHUONG_SEND* mRecv = (PMSG_CBLISTTHUONG_SEND*)lpMsg;
	mDataListItemMocNapClient.WC = mRecv->WC;
	mDataListItemMocNapClient.WP = mRecv->WP;
	mDataListItemMocNapClient.GP = mRecv->GP;
	mDataListItemMocNapClient.Ruud = mRecv->Ruud;
	for (int i = 0; i < mRecv->count; i++)
	{
		LISTITEMMOCNAP_SENDINFO lpInfo = *(LISTITEMMOCNAP_SENDINFO*)(((BYTE*)lpMsg) + sizeof(PMSG_CBLISTTHUONG_SEND) + (sizeof(LISTITEMMOCNAP_SENDINFO) * i));
		//==SetINfoItem
		INFO_LOCAL_ITEM infoItemLocal = { 0 };
		infoItemLocal.Count = lpInfo.Count;
		infoItemLocal.SizeBMD = lpInfo.SizeBMD;
		infoItemLocal.Index = lpInfo.Index;
		infoItemLocal.PeriodTime = lpInfo.PeriodTime;
		memcpy(infoItemLocal.Item, lpInfo.Item, sizeof(infoItemLocal.Item));
		mDataListItemMocNapClient.ListItemMocNap.push_back(infoItemLocal);

	}
}
void MocNap::GetListDonate(BYTE* Recv)
{
	if (!Recv) return;

	mDataMocNapClient.Clear();
	PMSG_CBMOCNAP_SEND* mRecv = (PMSG_CBMOCNAP_SEND*)Recv;
	mDataMocNapClient.NhanMocNap = mRecv->NhanMocNap;
	mDataMocNapClient.TongNap = mRecv->TongNap;
	mDataMocNapClient.RateNapThe = mRecv->RateNapThe;
	mDataMocNapClient.RateApplyTo = mRecv->RateApplyTo;
	for (int i = 0; i < mRecv->count; i++)
	{
		ListMocNapSend lpInfo = *(ListMocNapSend*)(((BYTE*)Recv) + sizeof(PMSG_CBMOCNAP_SEND) + (sizeof(ListMocNapSend) * i));
		mDataMocNapClient.DanhSachMocNap.push_back(lpInfo);
	}
}

void MocNap::RecvPaymentLink(BYTE* Recv)
{
	if (!Recv) return;

	PMSG_MOCNAP_PAYMENT_CREATE_ANS* mRecv = (PMSG_MOCNAP_PAYMENT_CREATE_ANS*)Recv;
	if (mRecv->Result == 0)
	{
		gMocNapQrDownloadStarted = false;
		gMocNapQrHasPayment = false;
		gMocNapQrTextureNeedsReload = true;
		gMocNapQrData[0] = '\0';
		SetMocNapQrStatus((mRecv->Message[0] != '\0') ? mRecv->Message : gTextClient.txtClient_Donate[25]);
		DeleteFileA("Data\\Custom\\MaQRNapThe.OZJ");
		InterlockedExchange(&gMocNapQrDownloadState, 0);
		return;
	}

	const char* qrSource = (mRecv->QrCode[0] != '\0') ? mRecv->QrCode : mRecv->CheckoutUrl;
	if (qrSource == 0 || qrSource[0] == '\0')
	{
		gMocNapQrDownloadStarted = false;
		gMocNapQrHasPayment = false;
		gMocNapQrTextureNeedsReload = true;
		SetMocNapQrStatus(gTextClient.txtClient_Donate[26]);
		DeleteFileA("Data\\Custom\\MaQRNapThe.OZJ");
		InterlockedExchange(&gMocNapQrDownloadState, 0);
		return;
	}

	strncpy_s(gMocNapQrData, sizeof(gMocNapQrData), qrSource, _TRUNCATE);
	strncpy_s(gMocNapPayOSBankBin, sizeof(gMocNapPayOSBankBin), mRecv->BankBin, _TRUNCATE);
	strncpy_s(gMocNapPayOSAccountNumber, sizeof(gMocNapPayOSAccountNumber), mRecv->AccountNumber, _TRUNCATE);
	strncpy_s(gMocNapPayOSAccountName, sizeof(gMocNapPayOSAccountName), mRecv->AccountName, _TRUNCATE);
	strncpy_s(gMocNapPayOSDescription, sizeof(gMocNapPayOSDescription), mRecv->Description, _TRUNCATE);
	gMocNapBankShortName[0] = '\0';
	gMocNapQrHasPayment = true;
	gMocNapQrTextureNeedsReload = true;
	gMocNapQrLastLoadTick = 0;
	SetMocNapQrStatus(gTextClient.txtClient_Donate[27]);
	DeleteFileA("Data\\Custom\\MaQRNapThe.OZJ");
	InterlockedExchange(&gMocNapQrDownloadState, 1);
	StartMocNapQrDownloadThread();
}

void MocNap::DrawXemMocNap()
{
	if (!gInterface->Data[eWindowMocNapList].OnShow)
	{
		return;
	}

	float WindowW = 185;
	float WindowH = 270;

	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 25.0;
	if (gInterface->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowMocNapList, gTextClient.txtClient_Donate[0]))
	{
		/// + 10
		StartX = StartX;
		StartY = StartY - 5;
		float WInfo = (WindowW - 20);
		float HInfo = WindowH - 100;

		SEASON3B::TextDraw((HFONT)g_hFont, StartX + 15, StartY + 35, 0xFF00EEDFF, 0x0, WindowW, 0, 1, gTextClient.txtClient_Donate[1]);

		//Scroll Bar
		int DataListItem = mDataListItemMocNapClient.ListItemMocNap.size();
		bool ShowItemScroll = (DataListItem > MaxListItemInPage);
		if (!ShowItemScroll)
		{
			if (ListItemNhan)
			{
				delete ListItemNhan;
				ListItemNhan = nullptr;
			}
		}
		else
		{
			if (!ListItemNhan)
			{
				ListItemNhan = new CNewUIScrollBar();
				ListItemNhan->Create((StartX + WindowW) - 25, StartY + 35 + 15, 130);
			}
			if (ListItemNhan)
			{
				int MaxItemPage = ((DataListItem + MaxListItemInPage - 1) / MaxListItemInPage) - 1;
				ListItemNhan->SetMaxPos(MaxItemPage);
				ListItemNhan->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
				if (ListItemNhan->GetCurPos() > MaxItemPage)
				{
					ListItemNhan->SetCurPos(MaxItemPage);
				}
				if (gInterface->Data[eWindowMocNapList].OnClick)
				{
					ListItemNhan->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
					ListItemNhan->SetCurPos(0);
				}
				HandleMocNapScrollWheel(ListItemNhan, StartX + 15, StartY + 35 + 15, WindowW, 130);
				ListItemNhan->Render();
				ListItemNhan->UpdateMouseEvent();
				ListItemNhan->Update();
			}
		}

		//==List Box Item 
		float PosXBoxItemGoc = StartX + 25;
		float PosXBoxItem = StartX + 25;
		float PosYBoxItem = StartY + 55;
		float WBox = 30;
		float KhoangCach = 45;
		int CountNgang = 0;
		int CountDoc = 0;
		int ItemListPage = (ShowItemScroll && ListItemNhan) ? ListItemNhan->GetCurPos() : 0;
		int BBShowInfoItem = -1;

		//
		int currentRow = 0;
		int currentCol = 0;

		for (int n = (ItemListPage * MaxListItemInPage); n < DataListItem; n++)
		{
			ITEM* pItem = NULL;
			pItem = g_pNewItemMng->CreateItem(mDataListItemMocNapClient.ListItemMocNap[n].Item);

			if (NULL != pItem)
			{
				if (mDataListItemMocNapClient.ListItemMocNap[n].PeriodTime > 0)
				{
					pItem->bPeriodItem = 1;
					pItem->lExpireTime = mDataListItemMocNapClient.ListItemMocNap[n].PeriodTime;
				}

				gInterface->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, 0x00000096, 0, 0);
				RenderLocalItem3D(PosXBoxItem, PosYBoxItem, WBox, WBox,
					pItem->Type, pItem->Level, pItem->Option1, pItem->ExtOption, false,
					mDataListItemMocNapClient.ListItemMocNap[n].SizeBMD);

				SEASON3B::TextDraw((HFONT)g_hFont, PosXBoxItem + 5, PosYBoxItem + 25, 0xE0FF14A5, 0x0, WBox, 0, 4, "x%d", mDataListItemMocNapClient.ListItemMocNap[n].Count);
				if (SEASON3B::CheckMouseIn(PosXBoxItem, PosYBoxItem, WBox, WBox))
				{
					BBShowInfoItem = n;
				}
				g_pNewItemMng->DeleteItem(pItem);
				PosXBoxItem += KhoangCach;
				CountDoc++;
				CountNgang++;
				if (CountNgang >= 3)
				{
					PosXBoxItem = PosXBoxItemGoc;
					PosYBoxItem += KhoangCach;
					CountNgang = 0;
				}
				if (CountDoc >= MaxListItemInPage) break;

				++currentCol;
				if (currentCol >= 3)
				{
					currentCol = 0;
					++currentRow;
				}
			}
		}

		//===Coin
		float PosYCoinNhan = StartY + 185;
		SEASON3B::TextDraw((HFONT)g_hFont, StartX + 15, PosYCoinNhan, 0xFF00EEDFF, 0x0, WindowW, 0, 1, gTextClient.txtClient_Donate[2]);
		PosYCoinNhan += 5;
		SEASON3B::TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 1), 0xFF8214FF, 0x0, WindowW, 0, 1, gTextClient.txtClient_Donate[3], gInterface->NumberFormat(mDataListItemMocNapClient.WC));			//Text3 = "+ WCoin : %s
		SEASON3B::TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 2), 0xFF8214FF, 0x0, WindowW, 0, 1, gTextClient.txtClient_Donate[4], gInterface->NumberFormat(mDataListItemMocNapClient.WP));		  //Text4 = "+ WCoinP : %
		SEASON3B::TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 3), 0xFF8214FF, 0x0, WindowW, 0, 1, gTextClient.txtClient_Donate[5], gInterface->NumberFormat(mDataListItemMocNapClient.GP));		  //Text5 = "+ GobinP : %
		//SEASON3B::TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 4), 0xFF8214FF, 0x0, WindowW, 0, 1, "+Ruud : %s", gInterface->NumberFormat(mDataListItemMocNapClient.Ruud));		  //Text6 = " + Ruud : % s"

		SEASON3B::TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * 5) + 5, 0x14FFC0FF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[6]);
		//===Show Info
		if (BBShowInfoItem != -1)
		{
			ITEM* CTItem = g_pNewItemMng->CreateItem(mDataListItemMocNapClient.ListItemMocNap[BBShowInfoItem].Item);
			if (CTItem && mDataListItemMocNapClient.ListItemMocNap[BBShowInfoItem].PeriodTime > 0)
			{
				CTItem->bPeriodItem = 1;
				CTItem->lExpireTime = mDataListItemMocNapClient.ListItemMocNap[BBShowInfoItem].PeriodTime;
			}
			if (CTItem)
			{
				RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);
				g_pNewItemMng->DeleteItem(CTItem);
			}
		}
	}
}

void MocNap::DrawWindow()
{
	if (gInterface->CheckWindow(CB_Interface::ObjWindow::MoveList) || gInterface->CheckWindow(CB_Interface::ObjWindow::CashShop) || gInterface->CheckWindow(CB_Interface::ObjWindow::SkillTree) || gInterface->CheckWindow(CB_Interface::ObjWindow::FullMap)
		|| (gInterface->CheckWindow(CB_Interface::Inventory)
			&& gInterface->CheckWindow(CB_Interface::ExpandInventory)
			&& gInterface->CheckWindow(CB_Interface::Store))
		|| (gInterface->CheckWindow(CB_Interface::Inventory)
			&& gInterface->CheckWindow(CB_Interface::Warehouse)
			&& gInterface->CheckWindow(CB_Interface::ExpandWarehouse)))
	{
		return;
	}

	if (!gInterface->Data[eWindowMocNap].OnShow)
	{
		if (ListMocNap)
		{
			delete ListMocNap;
			ListMocNap = nullptr;
		}
		if (ListItemNhan)
		{
			delete ListItemNhan;
			ListItemNhan = nullptr;
		}
		if (InputVND)
		{
			if (InputVND->HaveFocus())
			{
				SetFocus(gwinhandle->GethWnd());
			}
			delete InputVND;
			InputVND = NULL;
		}
		gInterface->Data[eWindowMocNapList].Close();
		if (SelectTypeMocNap != -1) SelectTypeMocNap = -1;
		return;
	}

	float WindowW = 220;
	float WindowH = 270;
#if QRCODE_MANAP
	bool ShowBankPanel = (gmProtect->EnableNapBankButton != 0);
#else
	bool ShowBankPanel = false;
#endif
	float RenderWindowW = WindowW + (ShowBankPanel ? 200.0f : 0.0f);
	gInterface->Data[eWindowMocNap].Width = RenderWindowW;
	float StartX = (MAX_WIN_WIDTH / 2) - (RenderWindowW / 2);
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);

#if QRCODE_MANAP
	if (gInterface->gDrawWindowCustom(&StartX, &StartY, RenderWindowW, WindowH, eWindowMocNap, gTextClient.txtClient_Donate[7]))
	{
		int TongSoDaNap = mDataMocNapClient.TongNap;
		int MocDaNhan = mDataMocNapClient.NhanMocNap;
		SEASON3B::TextDraw((HFONT)g_hFont, StartX, StartY + 25, 0x00FFDDFF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[8]);
		SEASON3B::TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFF9100FF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[9]);
		SEASON3B::TextDraw((HFONT)g_hFont, StartX, StartY + 50, 0x3CFF00FF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[10], gInterface->NumberFormat(TongSoDaNap));

		if (ShowBankPanel)
		{
			float BankX = StartX + WindowW + 17.0f;
			float BankY = StartY + 28.0f;
			float BankW = 140.0f;
			float QrSize = 80.0f;
			gInterface->DrawInfoBox(BankX, BankY, BankW, WindowH - 60.0f, 0x00000096, 0, 0);
			SEASON3B::TextDraw((HFONT)g_hFontBold, BankX + 4.0f, BankY + 6.0f, 0xFFDE26FF, 0x3a4b3978, BankW, 0, 3, gTextClient.txtClient_Donate[11]);
			SEASON3B::TextDraw((HFONT)g_hFont, BankX + 6.0f, BankY + 24.0f, 0xFFFFFFFF, 0x0, BankW - 12.0f, 0, 1, gTextClient.txtClient_Donate[12], GetMocNapBankDisplayName());
			SEASON3B::TextDraw((HFONT)g_hFont, BankX + 6.0f, BankY + 36.0f, 0xFFFFFFFF, 0x0, BankW - 12.0f, 0, 1, gTextClient.txtClient_Donate[13], GetMocNapAccountNumber());
			SEASON3B::TextDraw((HFONT)g_hFont, BankX + 6.0f, BankY + 48.0f, 0xFFFFFFFF, 0x0, BankW - 12.0f, 0, 1, gTextClient.txtClient_Donate[14], GetMocNapAccountName());
			SEASON3B::TextDraw((HFONT)g_hFont, BankX + 6.0f, BankY + 60.0f, 0xFFFFFFFF, 0x0, BankW - 12.0f, 0, 1, gTextClient.txtClient_Donate[15], GetMocNapDescriptionText());

			float QrX = BankX + 30.0f;
			float QrY = BankY + 78.0f;
			gInterface->DrawInfoBox(QrX, QrY, QrSize, QrSize, 0xFFFFFFFF, 0, 0);
			bool qrGenerating = (gMocNapQrDownloadState != 0);
			if (gMocNapQrTextureNeedsReload)
			{
				DeleteBitmap(kMocNapQrTexture, true);
				gMocNapQrTextureLoaded = false;
				gMocNapQrTextureNeedsReload = false;
				gMocNapQrLastLoadTick = 0;
			}
			bool qrLoaded = (!qrGenerating && gMocNapQrHasPayment && TryLoadMocNapQrTexture());
			if (qrLoaded)
			{
				RenderBitmap(kMocNapQrTexture, QrX, QrY, QrSize + 10.0f, QrSize + 10.0f, 0.0, 0.0, 1.0, 1.0, 1, 1, 0.0);
			}
			else if (qrGenerating)
			{
				DrawMocNapQrGenerating(QrX, QrY, QrSize);
			}
			else
			{
				if (gMocNapQrDownloadState == 0) {
					SEASON3B::TextDraw((HFONT)g_hFont, BankX + 33.0f, BankY + 110.0f, 0x808080FF, 0x0, QrSize, 0, 3, gTextClient.txtClient_Donate[19]);
					SEASON3B::TextDraw((HFONT)g_hFont, BankX + 33.0f, BankY + 120.0f, 0x808080FF, 0x0, QrSize, 0, 3, gTextClient.txtClient_Donate[20]);
				}
				/*const char* qrStatusText = (gMocNapQrStatusText[0] != '\0') ? gMocNapQrStatusText : ((gMocNapQrDownloadState != 0) ? gTextClient.txtClient_Donate[16] : gTextClient.txtClient_Donate[18]);
				SEASON3B::TextDraw((HFONT)g_hFont, BankX + 35.0f, BankY + 124.0f, 0x808080FF, 0x0, QrSize, 0, 3, qrStatusText);*/
			}

			SEASON3B::TextDraw((HFONT)g_hFont, BankX + 6.0f, BankY + 175.0f, 0xFF8214FF, 0x0, 130, 0, 1, "Nhập số tiền:");
			float InputVndX = BankX + 60.0f;
			float InputVndY = BankY + 175.0f;
			if (InputVND)
			{
				InputVND->SetPosition(InputVndX, InputVndY);
			}
			if (gInterface->RenderInputBox(InputVndX, InputVndY, 75, 14, GetInputVND, InputVND, UIOPTION_NUMBERONLY, sizeof(GetInputVND) - 1, false) && InputVND)
			{
				InputVND->GetText(GetInputVND, sizeof(GetInputVND));
			}

			float ButtonX = BankX + 45.0f;
			float ButtonY = BankY + 188.0f;
			float ButtonW = 60.0f;
			float ButtonH = 20.0f;
			if (qrGenerating)
			{
				gInterface->DrawBarForm(ButtonX, ButtonY, ButtonW, ButtonH, 0.0f, 0.0f, 0.0f, 0.65f);
				SEASON3B::TextDraw((HFONT)g_hFontBold, ButtonX - 5.0f, ButtonY + 6.0f, 0xAAAAAAFF, 0x0, ButtonW + 20.0f, 0, 3, "Đang tạo mã QR");
			}
			else if (gInterface->DrawButton(ButtonX, ButtonY, 100, 11, "Tạo mã QR", ButtonW))
			{
				if (InputVND)
				{
					InputVND->GetText(GetInputVND, sizeof(GetInputVND));
				}
				RequestMocNapQrGenerate(GetInputVND);
			}
		}
#else
	if (gInterface->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowMocNap, gTextClient.txtClient_Donate[7]))
	{
		int TongSoDaNap = mDataMocNapClient.TongNap;
		int MocDaNhan = mDataMocNapClient.NhanMocNap;
		SEASON3B::TextDraw((HFONT)g_hFont, StartX, StartY + 25, 0x00FFDDFF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[8]);
		SEASON3B::TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFF9100FF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[9]);
		SEASON3B::TextDraw((HFONT)g_hFont, StartX, StartY + 50, 0x3CFF00FF, 0x0, WindowW, 0, 3, gTextClient.txtClient_Donate[10], gInterface->NumberFormat(TongSoDaNap));
#endif
		//===Info Yeu Cau Moc Nap
		float InfoMocNapX = (StartX + 10) + 3;
		float InfoMocNapY = (StartY + 65);
		float TyleInfoYeuCau = 7.5f;
		float WInfo = (WindowW - 20) / 10;
		float WProcess = (WInfo * (TyleInfoYeuCau - 2.7));
		float WButton = 38;
		float HInfo = WindowH - 135;
		SEASON3B::TextDraw((HFONT)g_hFont, InfoMocNapX, InfoMocNapY, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoYeuCau + 40, 0, 3, gTextClient.txtClient_Donate[17]);
		//Scroll Bar
		int DataListMocNap = mDataMocNapClient.DanhSachMocNap.size();
		bool ShowMocNapScroll = (DataListMocNap > MaxListMocNapInPage);
		if (!ShowMocNapScroll)
		{
			if (ListMocNap)
			{
				delete ListMocNap;
				ListMocNap = nullptr;
			}
		}
		else
		{
			if (!ListMocNap)
			{
				ListMocNap = new CNewUIScrollBar();
				ListMocNap->Create((StartX + WindowW) - 15, InfoMocNapY + 10, HInfo);
			}
			if (ListMocNap)
			{
				int MaxMocNapPage = ((DataListMocNap + MaxListMocNapInPage - 1) / MaxListMocNapInPage) - 1;
				ListMocNap->SetMaxPos(MaxMocNapPage);
				ListMocNap->SetPos((StartX + WindowW) - 15, InfoMocNapY + 10);
				if (ListMocNap->GetCurPos() > MaxMocNapPage)
				{
					ListMocNap->SetCurPos(MaxMocNapPage);
				}
				if (gInterface->Data[eWindowMocNap].OnClick)
				{
					ListMocNap->SetPos((StartX + WindowW) - 15, InfoMocNapY + 10);
					ListMocNap->SetCurPos(0);
				}
				HandleMocNapScrollWheel(ListMocNap, InfoMocNapX, InfoMocNapY, WInfo * TyleInfoYeuCau, HInfo + 10);
				ListMocNap->Render();
				ListMocNap->UpdateMouseEvent();
				ListMocNap->Update();
			}
		}

		float KhoangCachYMocNap = 34;
		float InfoMocNapListStartY = InfoMocNapY;
		int MixItemListPage = (ShowMocNapScroll && ListMocNap) ? ListMocNap->GetCurPos() : 0;
		int MaxList = 0;
		for (int n = (MixItemListPage * MaxListMocNapInPage); n < DataListMocNap; n++)
		{
			//==List MocNap
			gInterface->DrawBarForm(InfoMocNapX, InfoMocNapY + 15, WInfo * TyleInfoYeuCau + 40, 30, 0.0, 0.0, 0.0, 0.5);
			SEASON3B::TextDraw((HFONT)g_hFont, InfoMocNapX + 5, InfoMocNapY + 20, 0xFFDE26FF, 0x0, WProcess, 0, 3, gTextClient.txtClient_Donate[18], mDataMocNapClient.DanhSachMocNap[n].IndexMocNap, gInterface->NumberFormat(mDataMocNapClient.DanhSachMocNap[n].GiaTriNap)); //"Mốc(%d) %s"
			//==Process
			gInterface->DrawBarForm(InfoMocNapX + 5, InfoMocNapY + 20 + 11, WProcess, 6, 0.29, 0.2767, 0.2581, 0.6); //Process BG
			//=Calc Tyle 
			float PhanTramMocNap = 0.0f;
			if (mDataMocNapClient.DanhSachMocNap[n].GiaTriNap > 0)
			{
				PhanTramMocNap = ((float)TongSoDaNap * 100.0f) / (float)mDataMocNapClient.DanhSachMocNap[n].GiaTriNap; // Tinh Ty le da nap
			}
			float TyLeTGA = (WProcess * PhanTramMocNap) / 100;
			if (TyLeTGA > WProcess) { TyLeTGA = WProcess; }
			gInterface->DrawBarForm(InfoMocNapX + 5, InfoMocNapY + 20 + 11, TyLeTGA, 6, 0.7833, 0.0, 1.0, 1.0); //Process
			//==Xem
			if (gInterface->DrawButton(InfoMocNapX + 5 + WProcess + 3, InfoMocNapY + 20, 110, 11, "Xem", WButton) && (GetTickCount() - gInterface->Data[eWindowMocNapList].EventTick) > 300) //"Xem"
			{

				SelectTypeMocNap = mDataMocNapClient.DanhSachMocNap[n].IndexMocNap;
				gInterface->Data[eWindowMocNapList].OnShow = true;
				gInterface->Data[eWindowMocNapList].EventTick = GetTickCount();
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xD3, 0x9B, sizeof(pMsg));
				pMsg.ThaoTac = SelectTypeMocNap; //
				DataSend((LPBYTE)&pMsg, pMsg.header.size);
			}
			if (MocDaNhan < mDataMocNapClient.DanhSachMocNap[n].IndexMocNap) //Kiem tra neu da nhan roi thi khong cho nhan nua
			{
				//==Nhận
				if( gInterface->DrawButton(InfoMocNapX + 5 + WProcess + 3 + WButton + 3, InfoMocNapY + 20, 110, 11, "Nhận", WButton) && (GetTickCount() - gInterface->Data[eWindowMocNapList].EventTick) > 300) //"Nhận"
				{
					XULY_CGPACKET pMsg;
					pMsg.header.set(0xD3, 0x9A, sizeof(pMsg));
					pMsg.ThaoTac = mDataMocNapClient.DanhSachMocNap[n].IndexMocNap; //
					DataSend((LPBYTE)&pMsg, pMsg.header.size);

				}

			}
			InfoMocNapY = InfoMocNapY + (KhoangCachYMocNap);
			MaxList++;
			if (MaxList >= MaxListMocNapInPage) break;
		}

		int cRateNapThe = mDataMocNapClient.RateNapThe;
		int cRateApplyTo = mDataMocNapClient.RateApplyTo;
		const char* txtRateApplyTo = "";
		if (cRateApplyTo == 1)
		{
			txtRateApplyTo = "WC";
		}
		else if (cRateApplyTo == 2) {
			txtRateApplyTo = "WP";
		}
		else if (cRateApplyTo == 3) {
			txtRateApplyTo = "WC+WP";
		}
		else if (cRateApplyTo == 4) {
			txtRateApplyTo = "GP";
		}
		else if (cRateApplyTo == 5) {
			txtRateApplyTo = "WC+GP";
		}
		else if (cRateApplyTo == 6) {
			txtRateApplyTo = "WP+GP";
		}
		else {
			txtRateApplyTo = "WC+WP+GP";
		}

		float RateInfoY = InfoMocNapListStartY + (KhoangCachYMocNap * MaxListMocNapInPage);
		SEASON3B::TextDraw((HFONT)g_hFont, InfoMocNapX + 5, RateInfoY + 15, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoYeuCau + 40, 0, 3, "Tỉ lệ donate của server là x%d.", cRateNapThe);
		SEASON3B::TextDraw((HFONT)g_hFont, InfoMocNapX + 5, RateInfoY + 25, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoYeuCau + 40, 0, 3, "Khi donate sẽ nhận được x%d %s.", cRateNapThe, txtRateApplyTo);
		SEASON3B::TextDraw((HFONT)g_hFont, InfoMocNapX + 5, RateInfoY + 35, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoYeuCau + 40, 0, 3, "Ví dụ: Donate 100k = %s %s.", gInterface->NumberFormat(cRateNapThe * 100000), txtRateApplyTo);
		SEASON3B::TextDraw((HFONT)g_hFont, InfoMocNapX + 5, RateInfoY + 47, 0xFF5C00FF, 0xEDE8D0FF, RenderWindowW - 32.0f, 0, 3, "*LƯU Ý: NẾU 1 MÃ QR MỚI ĐƯỢC TẠO, QUÉT MÃ QR CŨ SẼ KHÔNG ĐƯỢC AUTO XỬ LÝ");
		//0x3a4b3978
	}
	
}
