#include "StdAfx.h"
#include "H_ViewCharInfo.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "TextClient.h"
#include "ZzzInterface.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInventory.h"
#include "NewUIItemMng.h"
#include "NewUIMyInventory.h"
#include "CGMProtect.h"

using namespace SEASON3B;

#if(H_VIEWCHARINFO)
H_ViewCharInfo* gH_ViewCharInfo;

namespace
{
	constexpr float VIEWCHAR_PREVIEW_X_OFFSET = 112.0f;
	constexpr float VIEWCHAR_PREVIEW_Y_OFFSET = 0.0f;
	constexpr float VIEWCHAR_PREVIEW_WIDTH = 155.0f;
	constexpr float VIEWCHAR_PREVIEW_HEIGHT = 205.0f;
	constexpr float VIEWCHAR_PREVIEW_ANGLE = 100.0f;
	constexpr float VIEWCHAR_PREVIEW_ZOOM = 0.40f;
	constexpr float VIEWCHAR_PREVIEW_ROTATE_SPEED = 1.0f;

	void RestoreViewChar2DState()
	{
		glViewport2(0, 0, WindowWidth, WindowHeight);
		DisableAlphaBlend();
		DisableDepthTest();
		DisableCullFace();
		DisableDepthMask();
		EnableAlphaTest(false);
		glColor4f(1.f, 1.f, 1.f, 1.f);
	}
}

H_ViewCharInfo::H_ViewCharInfo()
{
#if(H_EXPANDSLOTITEM)
	memset(&m_ViewCharItemSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_NEW_EQUIPMENT);
#else
	memset(&m_ViewCharItemSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_EQUIPMENT_INDEX);
#endif
	memset(this->ViewPreviewEquipment, 0, sizeof(this->ViewPreviewEquipment));
	this->ViewClass = 0;
	this->m_HasPreviewData = false;
	this->m_PreviewViewerInitialized = false;
	this->m_PreviewDirty = false;
	this->m_PreviewAngle = VIEWCHAR_PREVIEW_ANGLE;
	this->m_LastPreviewMouseX = 0.0f;
	this->m_IsPreviewRotating = false;
	this->m_BlockEscapeKey = false;
	this->ClearData();
}


H_ViewCharInfo::~H_ViewCharInfo()
{
}
void H_ViewCharInfo::SetImgSlot(float x, float y)
{
#if(H_EXPANDSLOTITEM)
	memset(&m_ViewCharItemSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_NEW_EQUIPMENT);
#else
	memset(&m_ViewCharItemSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_EQUIPMENT_INDEX);
#endif

//	m_ViewCharItemSlots[EQUIPMENT_HELPER].x = x + 15;
//	m_ViewCharItemSlots[EQUIPMENT_HELPER].y = y + 44;
//	m_ViewCharItemSlots[EQUIPMENT_HELPER].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_HELPER].height = 46;
//	m_ViewCharItemSlots[EQUIPMENT_HELPER].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_FAIRY;
//
//	m_ViewCharItemSlots[EQUIPMENT_HELM].x = x + 75;
//	m_ViewCharItemSlots[EQUIPMENT_HELM].y = y + 44;
//	m_ViewCharItemSlots[EQUIPMENT_HELM].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_HELM].height = 46;
//	m_ViewCharItemSlots[EQUIPMENT_HELM].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_HELM;
//
//	m_ViewCharItemSlots[EQUIPMENT_WING].x = x + 120;
//	m_ViewCharItemSlots[EQUIPMENT_WING].y = y + 44;
//	//m_ViewCharItemSlots[EQUIPMENT_WING].width = 61;
//	m_ViewCharItemSlots[EQUIPMENT_WING].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_WING].height = 46;
//	m_ViewCharItemSlots[EQUIPMENT_WING].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_WING;
//
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].x = x + 135;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].y = y + 87;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_LEFT;
//
//	m_ViewCharItemSlots[EQUIPMENT_ARMOR].x = x + 75;
//	m_ViewCharItemSlots[EQUIPMENT_ARMOR].y = y + 87;
//	m_ViewCharItemSlots[EQUIPMENT_ARMOR].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_ARMOR].height = 66;
//	m_ViewCharItemSlots[EQUIPMENT_ARMOR].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_ARMOR;
//
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].x = x + 15;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].y = y + 87;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
//	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RIGHT;
//
//	m_ViewCharItemSlots[EQUIPMENT_GLOVES].x = x + 15;
//	m_ViewCharItemSlots[EQUIPMENT_GLOVES].y = y + 150;
//	m_ViewCharItemSlots[EQUIPMENT_GLOVES].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_GLOVES].height = 46;
//	m_ViewCharItemSlots[EQUIPMENT_GLOVES].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_GLOVES;
//
//	m_ViewCharItemSlots[EQUIPMENT_PANTS].x = x + 75;
//	m_ViewCharItemSlots[EQUIPMENT_PANTS].y = y + 150;
//	m_ViewCharItemSlots[EQUIPMENT_PANTS].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_PANTS].height = 46;
//	m_ViewCharItemSlots[EQUIPMENT_PANTS].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_PANTS;
//
//	m_ViewCharItemSlots[EQUIPMENT_BOOTS].x = x + 135;
//	m_ViewCharItemSlots[EQUIPMENT_BOOTS].y = y + 150;
//	m_ViewCharItemSlots[EQUIPMENT_BOOTS].width = 46;
//	m_ViewCharItemSlots[EQUIPMENT_BOOTS].height = 46;
//	m_ViewCharItemSlots[EQUIPMENT_BOOTS].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_BOOT;
//
//	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].x = x + 118;
//	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].y = y + 150;
//	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].width = 20;
//	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].height = 20;
//	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING;
//
//	m_ViewCharItemSlots[EQUIPMENT_AMULET].x = x + 58;
//	m_ViewCharItemSlots[EQUIPMENT_AMULET].y = y + 87;
//	m_ViewCharItemSlots[EQUIPMENT_AMULET].width = 20;
//	m_ViewCharItemSlots[EQUIPMENT_AMULET].height = 20;
//	m_ViewCharItemSlots[EQUIPMENT_AMULET].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_NECKLACE;
//
//	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].x = x + 58;
//	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].y = y + 150;
//	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].width = 20;
//	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].height = 20;
//	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING;
//
//#if(H_EXPANDSLOTITEM)
//		m_ViewCharItemSlots[EQUIPMENT_MUUN].x = x + 58;
//		m_ViewCharItemSlots[EQUIPMENT_MUUN].y = y + 69;
//		m_ViewCharItemSlots[EQUIPMENT_MUUN].width = 20;
//		m_ViewCharItemSlots[EQUIPMENT_MUUN].height = 20;
//		m_ViewCharItemSlots[EQUIPMENT_MUUN].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_MUUN;
//
//		m_ViewCharItemSlots[EQUIPMENT_ERING_R].x = x + 118;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_R].y = y + 115;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_R].width = 20;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_R].height = 20;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_R].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_EARRING;
//
//		m_ViewCharItemSlots[EQUIPMENT_ERING_L].x = x + 58;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_L].y = y + 115;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_L].width = 20;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_L].height = 20;
//		m_ViewCharItemSlots[EQUIPMENT_ERING_L].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_EARRING;
//
//		m_ViewCharItemSlots[EQUIPMENT_EAGLE].x = x + 118;
//		m_ViewCharItemSlots[EQUIPMENT_EAGLE].y = y + 170;
//		m_ViewCharItemSlots[EQUIPMENT_EAGLE].width = 20;
//		m_ViewCharItemSlots[EQUIPMENT_EAGLE].height = 20;
//		m_ViewCharItemSlots[EQUIPMENT_EAGLE].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_MUUN;
//#endif

	// ===== START SetImgSlot FIX =====

	m_ViewCharItemSlots[EQUIPMENT_HELM].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_HELM].y = y + 44;
	m_ViewCharItemSlots[EQUIPMENT_HELM].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELM].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELM].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_HELM;
	
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].y = y + 90;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_ARMOR;
	
	m_ViewCharItemSlots[EQUIPMENT_PANTS].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].y = y + 136;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_PANTS;
	
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].y = y + 182;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_GLOVES;
	
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].y = y + 228;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_BOOT;


	m_ViewCharItemSlots[EQUIPMENT_WING].x = x + 61;
	m_ViewCharItemSlots[EQUIPMENT_WING].y = y + 44;
	m_ViewCharItemSlots[EQUIPMENT_WING].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_WING].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_WING].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_WING;

	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].x = x + 61;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].y = y + 90;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RIGHT;

	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].x = x + 61;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].y = y + 162;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_LEFT;

	m_ViewCharItemSlots[EQUIPMENT_HELPER].x = x + 61;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].y = y + 228;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_FAIRY;

	/*m_ViewCharItemSlots[EQUIPMENT_PENTAGRAM].x = x + 43;
	m_ViewCharItemSlots[EQUIPMENT_PENTAGRAM].y = y + 227;
	m_ViewCharItemSlots[EQUIPMENT_PENTAGRAM].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_PENTAGRAM].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_PENTAGRAM].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_PENTAGRAM;*/

	m_ViewCharItemSlots[EQUIPMENT_AMULET].x = x + 107;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].y = y + 253;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_NECKLACE;

	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].x = x + 128;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].y = y + 253;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING;

	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].x = x + 149;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].y = y + 253;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING;

#if(H_EXPANDSLOTITEM)
	m_ViewCharItemSlots[EQUIPMENT_MUUN].x = x + 107;
	m_ViewCharItemSlots[EQUIPMENT_MUUN].y = y + 229;
	m_ViewCharItemSlots[EQUIPMENT_MUUN].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_MUUN].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_MUUN].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_MUUN;

	m_ViewCharItemSlots[EQUIPMENT_ERING_R].x = x + 128;
	m_ViewCharItemSlots[EQUIPMENT_ERING_R].y = y + 229;
	m_ViewCharItemSlots[EQUIPMENT_ERING_R].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_ERING_R].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_ERING_R].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_EARRING;

	m_ViewCharItemSlots[EQUIPMENT_ERING_L].x = x + 149;
	m_ViewCharItemSlots[EQUIPMENT_ERING_L].y = y + 229;
	m_ViewCharItemSlots[EQUIPMENT_ERING_L].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_ERING_L].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_ERING_L].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_EARRING;

	m_ViewCharItemSlots[EQUIPMENT_EAGLE].x = x + 170;
	m_ViewCharItemSlots[EQUIPMENT_EAGLE].y = y + 229;
	m_ViewCharItemSlots[EQUIPMENT_EAGLE].width = 20;
	m_ViewCharItemSlots[EQUIPMENT_EAGLE].height = 20;
	m_ViewCharItemSlots[EQUIPMENT_EAGLE].dwBgImage = CNewUIMyInventory::IMAGE_ITEM_MUUN;
#endif
}

void H_ViewCharInfo::ClearData()
{
#if(H_EXPANDSLOTITEM)
	for (int i = 0; i < MAX_NEW_EQUIPMENT; ++i)
#else
	for (int i = 0; i < MAX_EQUIPMENT; ++i)
#endif
	{
		this->CharItemInfo[i].Type = -1;
		this->CharItemInfo[i].Level = 0;
		this->CharItemInfo[i].Option1 = 0;
	}
	this->CharViewName.clear();
	this->ViewReset = 0;
	this->ViewLevel = 0;
	this->ViewMaster = 0;
	this->ViewPoint = 0;
	this->ViewGuild = "--Chưa có--";
	this->ViewVip = "--Chưa có--";

	this->ViewClass = 0;
	memset(this->ViewPreviewEquipment, 0, sizeof(this->ViewPreviewEquipment));
	this->m_HasPreviewData = false;
	this->m_PreviewDirty = false;
	this->m_PreviewAngle = VIEWCHAR_PREVIEW_ANGLE;
	this->m_LastPreviewMouseX = 0.0f;
	this->m_IsPreviewRotating = false;
	this->m_CharItemInfo.clear();
}

void H_ViewCharInfo::EnsurePreviewViewer()
{
	if (this->m_PreviewViewerInitialized || Hero == nullptr)
	{
		return;
	}

	this->m_PhotoViewer.Init(0);
	this->m_PhotoViewer.SetAnimation(AT_STAND1);
	this->m_PhotoViewer.SetPreviewAngle(this->m_PreviewAngle);
	this->m_PhotoViewer.SetZoom(VIEWCHAR_PREVIEW_ZOOM);
	this->m_PhotoViewer.SetHeight(0.0f);
	this->m_PreviewViewerInitialized = true;
	this->m_PreviewDirty = true;
}

void H_ViewCharInfo::UpdatePreviewViewer()
{
	if (!this->m_PreviewViewerInitialized || !this->m_HasPreviewData || !this->m_PreviewDirty)
	{
		return;
	}

	BYTE clientClass = gCharacterManager.ChangeServerClassTypeToClientClassType(this->ViewClass);

	this->m_PhotoViewer.SetClass(clientClass);
	this->m_PhotoViewer.ResetPreviewHelperState();
	this->m_PhotoViewer.SetEquipmentPacket(this->ViewPreviewEquipment);
	this->m_PhotoViewer.SetID(this->CharViewName.c_str());
	this->m_PhotoViewer.SetAnimation(AT_STAND1);
	this->m_PhotoViewer.SetPreviewAngle(this->m_PreviewAngle);
	this->m_PreviewDirty = false;
}

bool H_ViewCharInfo::GetPreviewRect(float& x, float& y, float& width, float& height) const
{
	if (!gInterface->Data[eWindowViewItemChar].OnShow)
	{
		return false;
	}

	float startX = 110.0f;
	float startY = 50.0f;

	if (gInterface->Data[eWindowViewItemChar].FirstLoad)
	{
		startX = gInterface->Data[eWindowViewItemChar].X;
		startY = gInterface->Data[eWindowViewItemChar].Y;
	}

	x = startX + VIEWCHAR_PREVIEW_X_OFFSET;
	y = startY + VIEWCHAR_PREVIEW_Y_OFFSET;
	width = VIEWCHAR_PREVIEW_WIDTH;
	height = VIEWCHAR_PREVIEW_HEIGHT;
	return true;
}

void H_ViewCharInfo::HandlePreviewMouse(float x, float y, float width, float height)
{
	if (!this->m_HasPreviewData)
	{
		this->m_IsPreviewRotating = false;
		return;
	}

	const bool mouseInPreview = (SEASON3B::CheckMouseIn(x, y, width, height) == TRUE);
	const bool rightButtonDown = ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);

	if (this->m_IsPreviewRotating)
	{
		MouseOnWindow = true;
		gInterface->SetBlockCur(TRUE);
		MouseRButtonPush = false;
		MouseRButton = false;
		MouseRButtonPress = 0;

		if (MouseRButtonPop)
		{
			MouseRButtonPop = false;
		}

		if (!rightButtonDown)
		{
			this->m_IsPreviewRotating = false;
			return;
		}

		const float deltaX = MouseX - this->m_LastPreviewMouseX;
		if (deltaX != 0.0f)
		{
			this->m_PreviewAngle += (deltaX * VIEWCHAR_PREVIEW_ROTATE_SPEED);

			if (this->m_PreviewViewerInitialized)
			{
				this->m_PhotoViewer.SetPreviewAngle(this->m_PreviewAngle);
			}
		}

		this->m_LastPreviewMouseX = MouseX;
		return;
	}

	if (!mouseInPreview)
	{
		return;
	}

	MouseOnWindow = true;
	gInterface->SetBlockCur(TRUE);
	if (MouseRButtonPush && rightButtonDown)
	{
		this->m_IsPreviewRotating = true;
		this->m_LastPreviewMouseX = MouseX;
		MouseRButtonPush = false;
		MouseRButton = false;
		MouseRButtonPop = false;
		MouseRButtonPress = 0;
	}
}

void H_ViewCharInfo::UpdateMouseInput()
{
	if (this->m_BlockEscapeKey && ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0))
	{
		this->m_BlockEscapeKey = false;
	}

	if (!this->IsOpen())
	{
		this->m_IsPreviewRotating = false;
		return;
	}

	if (SEASON3B::IsPress(VK_ESCAPE))
	{
		this->m_BlockEscapeKey = true;
		this->Close();
		SEASON3B::CNewKeyInput::GetInstance()->SetKeyState(VK_ESCAPE, SEASON3B::CNewKeyInput::KEY_NONE);
		return;
	}

	float previewX = 0.0f;
	float previewY = 0.0f;
	float previewW = 0.0f;
	float previewH = 0.0f;

	if (!this->GetPreviewRect(previewX, previewY, previewW, previewH))
	{
		this->m_IsPreviewRotating = false;
		return;
	}

	this->HandlePreviewMouse(previewX, previewY, previewW, previewH);
}

bool H_ViewCharInfo::IsOpen() const
{
	return (gInterface->Data[eWindowViewItemChar].OnShow != 0);
}

bool H_ViewCharInfo::ShouldBlockEscapeKey() const
{
	return (this->m_BlockEscapeKey || this->IsOpen());
}

void H_ViewCharInfo::Close()
{
	this->m_IsPreviewRotating = false;
	gInterface->Data[eWindowViewItemChar].OnShow = 0;
	gInterface->Data[eWindowViewItemChar].EventTick = GetTickCount();
}

void H_ViewCharInfo::DrawCharacterPreview(float x, float y, float width, float height)
{
	this->EnsurePreviewViewer();
	this->UpdatePreviewViewer();

	if (!this->m_PreviewViewerInitialized || !this->m_HasPreviewData)
	{
		return;
	}

	EndBitmap();
	this->m_PhotoViewer.SetPosition((int)x, (int)y);
	this->m_PhotoViewer.SetSize((int)width, (int)height);
	this->m_PhotoViewer.Render();
	BeginBitmap();
	RestoreViewChar2DState();
}

void H_ViewCharInfo::RecvProtocol(BYTE* Recv)
{
	if (!Recv) return;
	PMSG_COUNTLIST_VIEWCHAR* mRecv = (PMSG_COUNTLIST_VIEWCHAR*)Recv;
	this->ClearData();
	this->CharViewName = mRecv->Name;
	this->ViewReset = mRecv->Reset;
	this->ViewLevel = mRecv->Level;
	this->ViewMaster = mRecv->MasterLevel;
	this->ViewPoint = mRecv->TotalPoints;
	char szGuildName[sizeof(mRecv->GuildName) + 1] = { 0 };
	memcpy(szGuildName, mRecv->GuildName, sizeof(mRecv->GuildName));
	if (szGuildName[0] != 0)
	{
		this->ViewGuild = szGuildName;
	}
	this->ViewClass = mRecv->Class;
	memcpy(this->ViewPreviewEquipment, mRecv->Equipment, sizeof(this->ViewPreviewEquipment));
	this->m_HasPreviewData = true;
	this->m_PreviewDirty = true;
	this->m_PreviewAngle = VIEWCHAR_PREVIEW_ANGLE;
	this->m_LastPreviewMouseX = 0.0f;
	this->m_IsPreviewRotating = false;

	switch (mRecv->AccountLevel)
	{
	case 0: this->ViewVip = "Thường"; break;
	case 1: this->ViewVip = "VIP 1"; break;
	case 2: this->ViewVip = "VIP 2"; break;
	case 3: this->ViewVip = "VIP 3"; break;
	default:
		char szVip[32];
		sprintf_s(szVip, sizeof(szVip), "VIP %d", mRecv->AccountLevel);
		this->ViewVip = szVip;
		break;
	}

	for (int n = 0; n < mRecv->Count; n++)
	{
		GETINFOCHAR_DATA lpInfo = *(GETINFOCHAR_DATA*)(((BYTE*)Recv) + sizeof(PMSG_COUNTLIST_VIEWCHAR) + (sizeof(GETINFOCHAR_DATA) * n));
#if(H_EXPANDSLOTITEM)
		if (n >= MAX_NEW_EQUIPMENT) break;
#else
		if (n > MAX_EQUIPMENT) break;
#endif
		
		int parsedType = ConvertItemType(lpInfo.Item);
		if (parsedType != 0x1FFF && parsedType != -1)
		{
			ITEM* pNewItem = g_pNewItemMng->CreateItem(lpInfo.Item);
			if (pNewItem)
			{
				memcpy(&this->CharItemInfo[n], pNewItem, sizeof(ITEM));
				this->CharItemInfo[n].Durability = lpInfo.Dur;
				if (lpInfo.PeriodTime)
				{
					this->CharItemInfo[n].bPeriodItem = 1;
					this->CharItemInfo[n].lExpireTime = lpInfo.PeriodTime;
				}
				g_pNewItemMng->DeleteItem(pNewItem);
			}
		}
		else
		{
			this->CharItemInfo[n].Type = -1;
		}
	}
	gInterface->Data[eWindowViewItemChar].OnShow = 1;
	//gInterface->DrawMessage(1, "Recv Info Char %s", this->CharViewName.c_str());
}
void H_ViewCharInfo::SendRequestViewItem(int aIndex)
{
	if (aIndex != -1)
	{
		//gInterface->DrawMessage(1, "SendRequestViewItem %d", aIndex);

		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x02, sizeof(pMsg));
		pMsg.ThaoTac = aIndex;
		DataSend((LPBYTE)& pMsg, pMsg.header.size);

		this->ClearData();
		gInterface->Data[eWindowViewItemChar].OnShow = 1;
	}
}
void H_ViewCharInfo::OpenClose()
{
	if (GetTickCount() - gInterface->Data[eWindowViewItemChar].EventTick > 300)
	{
		gInterface->Data[eWindowViewItemChar].EventTick = GetTickCount();
		if (gInterface->Data[eWindowViewItemChar].OnShow)
		{
			this->Close();
			return;
		}
		//==Show hoac send packet open
		gInterface->Data[eWindowViewItemChar].OnShow = 1;
	}
}

void H_ViewCharInfo::DrawWindow()
{
	//=== Test Open Window
	//if (GetKeyState(VK_F3) & 0x4000 && GetTickCount() > gInterface->Data[eWindowViewItemChar].EventTick + 300)
	//{
	//	this->OpenClose(); // << Ham Goi Window
	//	gInterface->Data[eWindowViewItemChar].EventTick = GetTickCount();
	//}

	if (gInterface->CheckWindow(CB_Interface::ObjWindow::MoveList) || gInterface->CheckWindow(CB_Interface::ObjWindow::CashShop) || gInterface->CheckWindow(CB_Interface::ObjWindow::SkillTree) || gInterface->CheckWindow(CB_Interface::ObjWindow::FullMap)
		|| (gInterface->CheckWindow(CB_Interface::ObjWindow::Inventory)
			&& gInterface->CheckWindow(CB_Interface::ObjWindow::ExpandInventory)
			&& gInterface->CheckWindow(CB_Interface::ObjWindow::Store))
		|| (gInterface->CheckWindow(CB_Interface::ObjWindow::Inventory)
			&& gInterface->CheckWindow(CB_Interface::ObjWindow::Warehouse)
			&& gInterface->CheckWindow(CB_Interface::ObjWindow::ExpandWarehouse)))
	{
		this->m_IsPreviewRotating = false;
		gInterface->Data[eWindowViewItemChar].OnShow = false;
		return;
	}

	if (!gInterface->Data[eWindowViewItemChar].OnShow)
	{
		this->m_IsPreviewRotating = false;
		return;
	}
	float CuaSoW = 450;
	float CuaSoH = 300;
	float StartX = 110;
	float StartY = 50;
	float previewX = StartX + VIEWCHAR_PREVIEW_X_OFFSET;
	float previewY = StartY + VIEWCHAR_PREVIEW_Y_OFFSET;
	float previewW = VIEWCHAR_PREVIEW_WIDTH;
	float previewH = VIEWCHAR_PREVIEW_HEIGHT;

	gInterface->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowViewItemChar, gTextClient.txtClient_ViewCharInfo[0]);
	this->SetImgSlot(StartX, StartY);
	previewX = StartX + VIEWCHAR_PREVIEW_X_OFFSET;
	previewY = StartY + VIEWCHAR_PREVIEW_Y_OFFSET;
	int NumberInfo = -1;
#if(H_EXPANDSLOTITEM)
	int loopsLimit = MAX_NEW_EQUIPMENT;
#else
	int loopsLimit = MAX_EQUIPMENT_INDEX;
#endif

	// --- Vòng 1: Vẽ Background 2D ---
	for (int i = 0; i < loopsLimit; i++)
	{
		if (m_ViewCharItemSlots[i].width <= 0 || m_ViewCharItemSlots[i].height <= 0 || m_ViewCharItemSlots[i].dwBgImage == 0)
		{
			continue;
		}

		glColor4f(1.f, 1.f, 1.f, 1.f);
		EnableAlphaTest();

		RenderImage(m_ViewCharItemSlots[i].dwBgImage, m_ViewCharItemSlots[i].x, m_ViewCharItemSlots[i].y,
			m_ViewCharItemSlots[i].width, m_ViewCharItemSlots[i].height);


		DisableAlphaBlend();

		if (SEASON3B::CheckMouseIn(m_ViewCharItemSlots[i].x, m_ViewCharItemSlots[i].y,
			m_ViewCharItemSlots[i].width, m_ViewCharItemSlots[i].height))
		{
			NumberInfo = i;
		}
	}

	// --- Vẽ cột Text bên phải ---
	this->DrawCharacterPreview(previewX, previewY, previewW, previewH);
	EnableAlphaTest(true);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0);

	float textX = StartX + 270.f;  // Vị trí X của các chữ số
	float textY = StartY + 50.f;
	float textGap = 20.f;

	// Labels
	g_pRenderText->SetTextColor(255, 191, 0, 255);
	g_pRenderText->RenderText(textX, textY, gTextClient.txtClient_ViewCharInfo[1]);
	g_pRenderText->RenderText(textX, textY + textGap, gTextClient.txtClient_ViewCharInfo[2]);
	g_pRenderText->RenderText(textX, textY + textGap * 2, gTextClient.txtClient_ViewCharInfo[3]);
	g_pRenderText->RenderText(textX, textY + textGap * 3, gTextClient.txtClient_ViewCharInfo[4]);
	g_pRenderText->RenderText(textX, textY + textGap * 4, gTextClient.txtClient_ViewCharInfo[5]);
	
	float textY_Guild = textY + textGap * 5 + 10.f;
	g_pRenderText->RenderText(textX, textY_Guild, gTextClient.txtClient_ViewCharInfo[6]);
	g_pRenderText->RenderText(textX, textY_Guild + textGap, gTextClient.txtClient_ViewCharInfo[7]);

	// Values (Căn phải dùng RT3_SORT_RIGHT)
	g_pRenderText->SetTextColor(50, 200, 255, 255);
	float valueWidth = 145.f;
	char szTemp[256];

	g_pRenderText->RenderText(textX, textY, this->CharViewName.c_str(), valueWidth, 0, RT3_SORT_RIGHT);
	
	sprintf_s(szTemp, sizeof(szTemp), "%d", this->ViewReset);
	g_pRenderText->RenderText(textX, textY + textGap, szTemp, valueWidth, 0, RT3_SORT_RIGHT);

	sprintf_s(szTemp, sizeof(szTemp), "%d", this->ViewLevel);
	g_pRenderText->RenderText(textX, textY + textGap * 2, szTemp, valueWidth, 0, RT3_SORT_RIGHT);

	sprintf_s(szTemp, sizeof(szTemp), "%d", this->ViewMaster);
	g_pRenderText->RenderText(textX, textY + textGap * 3, szTemp, valueWidth, 0, RT3_SORT_RIGHT);

	sprintf_s(szTemp, sizeof(szTemp), "%d", this->ViewPoint);
	g_pRenderText->RenderText(textX, textY + textGap * 4, szTemp, valueWidth, 0, RT3_SORT_RIGHT);

	g_pRenderText->RenderText(textX, textY_Guild, this->ViewGuild.c_str(), valueWidth, 0, RT3_SORT_RIGHT);
	g_pRenderText->RenderText(textX, textY_Guild + textGap, this->ViewVip.c_str(), valueWidth, 0, RT3_SORT_RIGHT);

	// --- Vòng 2: Thiết lập Ma Trận Khung Nhìn 3D và Vẽ Items ---
	SEASON3B::begin3D();

	for (int i = 0; i < loopsLimit; i++)
	{
		if (m_ViewCharItemSlots[i].width <= 0 || m_ViewCharItemSlots[i].height <= 0 || m_ViewCharItemSlots[i].dwBgImage == 0)
		{
			continue;
		}

		if (CharItemInfo[i].Type != -1 && CharItemInfo[i].Type != 0x1FFF)
		{
			float y = 0.f;
			if (i == EQUIPMENT_ARMOR)
			{
				y = m_ViewCharItemSlots[i].y - 10.f;
			}
			else
			{
				y = m_ViewCharItemSlots[i].y;
			}

			glColor4f(1.f, 1.f, 1.f, 1.f);
			RenderItem3D(
				(float)(m_ViewCharItemSlots[i].x + 1),
				y,
				(float)(m_ViewCharItemSlots[i].width - 4),
				(float)(m_ViewCharItemSlots[i].height - 4),
				CharItemInfo[i].Type,
				CharItemInfo[i].Level,
				CharItemInfo[i].Option1,
				CharItemInfo[i].ExtOption,
				false);
		}
	}

	SEASON3B::endrender3D();
	RestoreViewChar2DState();

	// Vẽ Tooltip (Phải luôn nằm cuối cùng để không đè State)
	if (NumberInfo != -1)
	{
		g_pNewItemTooltip->RenderItemTooltip(MouseX + 75, MouseY, &CharItemInfo[NumberInfo], false);
	}

	RestoreViewChar2DState();
}
#endif
