// NewUIMiniMap.cpp - Flat 2D MiniMap with zoom & pan
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIMiniMap.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "NewUIGuildInfoWindow.h"
#include "NewUIButton.h"
#include "NewUIMyInventory.h"
#include "CSitemOption.h"
#include "MapManager.h"
#include "ZzzAI.h"

extern BYTE m_OccupationState;
extern int MouseWheel;
extern bool MouseRButton;
extern bool MouseRButtonPush;
extern bool MouseRButtonPop;

using namespace SEASON3B;


SEASON3B::CNewUIMiniMap::CNewUIMiniMap()
{
	m_pNewUIMng = NULL;
	m_fZoom = 1.0f;
	m_fPanX = 0.0f;
	m_fPanY = 0.0f;
	m_bDragging = false;
	m_iDragStartX = 0;
	m_iDragStartY = 0;
	m_fDragStartPanX = 0.0f;
	m_fDragStartPanY = 0.0f;
	m_bRenderTopMostPass = false;
}

SEASON3B::CNewUIMiniMap::~CNewUIMiniMap()
{
	Release();
}

bool SEASON3B::CNewUIMiniMap::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MINI_MAP, this);

	LoadBitmap("Interface\\mini_map_ui_corner.tga", IMAGE_MINIMAP_INTERFACE + 1, GL_LINEAR);
	LoadBitmap("Interface\\mini_map_ui_line.jpg", IMAGE_MINIMAP_INTERFACE + 2, GL_LINEAR);
	LoadBitmap("Interface\\mini_map_ui_cha.tga", IMAGE_MINIMAP_INTERFACE + 3, GL_LINEAR);
	LoadBitmap("Interface\\mini_map_ui_portal.tga", IMAGE_MINIMAP_INTERFACE + 4, GL_LINEAR);
	LoadBitmap("Interface\\mini_map_ui_npc.tga", IMAGE_MINIMAP_INTERFACE + 5, GL_LINEAR);
	LoadBitmap("Interface\\mini_map_ui_cancel.tga", IMAGE_MINIMAP_INTERFACE + 6, GL_LINEAR);

	m_BtnExit.ChangeButtonImgState(true, IMAGE_MINIMAP_INTERFACE + 6, false);
	m_BtnExit.ChangeToolTipText(GlobalText[1002], true);

	m_Size.cx = 255;
	m_Size.cy = 255;

	SetPos(x, y);

	m_Lenth[0].x = 800;  m_Lenth[0].y = 800;
	m_Lenth[1].x = 1000; m_Lenth[1].y = 1000;
	m_Lenth[2].x = 1200; m_Lenth[2].y = 1200;
	m_Lenth[3].x = 1400; m_Lenth[3].y = 1400;
	m_Lenth[4].x = 1600; m_Lenth[4].y = 1600;
	m_Lenth[5].x = 1800; m_Lenth[5].y = 1800;
	m_MiniPos = 0;
	m_bSuccess = false;
	m_fZoom = 1.0f;
	m_fPanX = 0.0f;
	m_fPanY = 0.0f;
	m_bDragging = false;
	return true;
}

void SEASON3B::CNewUIMiniMap::ClosingProcess()
{
	m_bDragging = false;
}

float SEASON3B::CNewUIMiniMap::GetLayerDepth()
{
	return 20.0f;
}

void SEASON3B::CNewUIMiniMap::OpenningProcess()
{
	int x = ((int)GetWindowsX - m_Size.cx) / 2;
	int y = ((int)GetWindowsY - m_Size.cy) / 2;
	SetPos(x, y);
	m_fZoom = 1.0f;
	m_fPanX = 0.0f;
	m_fPanY = 0.0f;
	m_bDragging = false;
}

void SEASON3B::CNewUIMiniMap::Release()
{
	UnloadImages();

	for (int i = 1; i < 7; i++)
	{
		DeleteBitmap(IMAGE_MINIMAP_INTERFACE + i);
	}

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIMiniMap::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_BtnExit.ChangeButtonInfo(m_Pos.x + m_Size.cx - 27, m_Pos.y, 30, 25);
}

void SEASON3B::CNewUIMiniMap::SetBtnPos(int Num, float x, float y, float nx, float ny)
{
	m_Btn_Loc[Num][0] = x;
	m_Btn_Loc[Num][1] = y;
	m_Btn_Loc[Num][2] = nx;
	m_Btn_Loc[Num][3] = ny;
}

// Convert map coordinate (0..255) to screen pixel
void SEASON3B::CNewUIMiniMap::MapToScreen(float mapX, float mapY, float& screenX, float& screenY)
{
	screenX = m_Pos.x + (mapY - m_fPanX) * m_fZoom;
	screenY = m_Pos.y + (mapX - m_fPanY) * m_fZoom;
}

// Convert screen pixel to map coordinate (0..255)
void SEASON3B::CNewUIMiniMap::ScreenToMap(int screenX, int screenY, float& mapX, float& mapY)
{
	mapY = (screenX - m_Pos.x) / m_fZoom + m_fPanX;
	mapX = (screenY - m_Pos.y) / m_fZoom + m_fPanY;
}

void SEASON3B::CNewUIMiniMap::ClampPan()
{
	float viewSize = 255.0f / m_fZoom;
	float maxPan = 255.0f - viewSize;
	if (maxPan < 0) maxPan = 0;
	if (m_fPanX < 0) m_fPanX = 0;
	if (m_fPanY < 0) m_fPanY = 0;
	if (m_fPanX > maxPan) m_fPanX = maxPan;
	if (m_fPanY > maxPan) m_fPanY = maxPan;
}

bool SEASON3B::CNewUIMiniMap::UpdateKeyEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP))
	{
		if (IsPress(VK_ESCAPE) == true || IsPress(VK_TAB) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

bool SEASON3B::CNewUIMiniMap::Render()
{
	if (m_bSuccess == false)
		return m_bSuccess;

	if (m_bRenderTopMostPass == false)
		return true;

	EnableAlphaTest();
	glColor4f(0.f, 0.f, 0.f, 0.85f);
	RenderColor((float)m_Pos.x, (float)m_Pos.y, (float)m_Size.cx, (float)m_Size.cy);
	EndRenderColor();
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	// Calculate UV coordinates based on zoom and pan
	float uvScale = 1.0f / m_fZoom;  // portion of texture visible
	float uvX = m_fPanX / 255.0f;    // UV offset
	float uvY = m_fPanY / 255.0f;

	// Render map texture with zoom/pan UV
	RenderBitmap(IMAGE_MINIMAP_INTERFACE, m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy, uvX, uvY, uvScale, uvScale);

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	int NpcWidth = 15 / 4;
	int NpcWidthP = 30 / 3;

	// Draw waypoints on map
	if (IsMoving())
	{
		for (std::deque<PAIR>::iterator it = (&m_WayPoint)->begin(); it != (&m_WayPoint)->end(); it++)
		{
			if (std::next(it) == m_WayPoint.end()) {
				NpcWidth = 15 / 3;
			}
			float sx, sy;
			MapToScreen((float)it->first, (float)it->second, sx, sy);
			if (sx >= m_Pos.x && sx <= m_Pos.x + m_Size.cx && sy >= m_Pos.y && sy <= m_Pos.y + m_Size.cy) {
				glColor4f(1.f, 0.1f, 0.1f, 1.f);
				RenderImage(IMAGE_MINIMAP_INTERFACE + 5, sx - NpcWidth / 1.5, sy - NpcWidth, NpcWidth, NpcWidth, 0.f, 0.f, 17.5f / 32.f, 17.5f / 32.f);
				glColor4f(1.f, 1.f, 1.f, 1.f);
			}
		}
	}
	NpcWidth = 15 / 3;

	float uvxy = (41.7f / 64.f);
	float uvxy_Line = 8.f / 8.f;
	float Ui_wid = 35.f;
	float Ui_Hig = 6.f;
	int i = 0;

	// Border lines (top/bottom)
	for (i = 0; i < 7; i++)
	{
		RenderImage(IMAGE_MINIMAP_INTERFACE + 2, m_Pos.x + i * Ui_wid, m_Pos.y, Ui_wid, Ui_Hig, 0.f, 1.f, uvxy, -uvxy_Line);
		RenderImage(IMAGE_MINIMAP_INTERFACE + 2, m_Pos.x + i * Ui_wid, m_Pos.y + m_Size.cy - Ui_Hig, Ui_wid, Ui_Hig, 0.f, 0.f, uvxy, uvxy_Line);
	}
	// Border lines (left/right)
	for (i = 1; i < 8; i++)
	{
		RenderBitmapRotate(IMAGE_MINIMAP_INTERFACE + 2, m_Pos.x + (Ui_Hig / 2.f), m_Pos.y + i * (Ui_wid - 3.f), Ui_wid, Ui_Hig, -90.f, 0.f, 0.f, uvxy, uvxy_Line);
		RenderBitmapRotate(IMAGE_MINIMAP_INTERFACE + 2, m_Pos.x + m_Size.cx - (Ui_Hig / 2.f), m_Pos.y + i * (Ui_wid - 3.f), Ui_wid, Ui_Hig, 90.f, 0.f, 0.f, uvxy, uvxy_Line);
	}

	// Corners
	RenderImage(IMAGE_MINIMAP_INTERFACE + 1, m_Pos.x, m_Pos.y, Ui_wid, Ui_wid, 0.f, 0.f, uvxy, uvxy);
	RenderImage(IMAGE_MINIMAP_INTERFACE + 1, m_Pos.x + m_Size.cx - Ui_wid, m_Pos.y, Ui_wid, Ui_wid, uvxy, 0.f, -uvxy, uvxy);
	RenderImage(IMAGE_MINIMAP_INTERFACE + 1, m_Pos.x, m_Pos.y + m_Size.cy - Ui_wid, Ui_wid, Ui_wid, 0.f, uvxy, uvxy, -uvxy);
	RenderImage(IMAGE_MINIMAP_INTERFACE + 1, m_Pos.x + m_Size.cx - Ui_wid, m_Pos.y + m_Size.cy - Ui_wid, Ui_wid, Ui_wid, uvxy, uvxy, -uvxy, -uvxy);


	unicode::t_char txtGuide[128];
	unicode::_sprintf(txtGuide, GlobalText[3828]);

	g_pRenderText->RenderFont(m_Pos.x + 20, m_Pos.y + 8 * (Ui_wid - 3.f), txtGuide, 0, 15.0, RT3_SORT_CENTER);

	// Render NPC/Gate icons on map (with zoom/pan transform)
	int Hoving = -1;
	int HvgPt = -1;
	POINT HverPos = { 0, 0 };

	for (i = 0; i < MAX_MINI_MAP_DATA; i++)
	{
		if (m_Mini_Map_Data[i].Kind > 0 && m_Mini_Map_Data[i].Kind <= 10)
		{
			float Tx1, Ty1;
			MapToScreen((float)m_Mini_Map_Data[i].Location[0], (float)m_Mini_Map_Data[i].Location[1], Tx1, Ty1);
			// Swap: MapToScreen returns (screenX based on mapY, screenY based on mapX)
			// Tx1 = screenX, Ty1 = screenY

			// Skip if outside visible area
			if (Tx1 < m_Pos.x - 10 || Tx1 > m_Pos.x + m_Size.cx + 10 || Ty1 < m_Pos.y - 10 || Ty1 > m_Pos.y + m_Size.cy + 10)
				continue;

			if (m_Mini_Map_Data[i].Kind == 1) // Gate
			{
				if (!(World == WD_34CRYWOLF_1ST && m_OccupationState > 0) || (m_Mini_Map_Data[i].Location[0] == 228 && m_Mini_Map_Data[i].Location[1] == 48 && World == WD_34CRYWOLF_1ST)) {
					if (CheckMouseIn(Tx1 - NpcWidth / 1.5, Ty1 - NpcWidth, NpcWidth, NpcWidth)) {
						Hoving = i;
					}
					if (Hoving == i) {
						HverPos.x = Tx1 - NpcWidth / 1.5;
						HverPos.y = Ty1 - NpcWidth;
					}
					if (Hoving == i) glColor4f(1.f, 1.f, 0.15f, 1.f); else glColor4f(1.f, 1.f, 1.f, 1.f);
					RenderImage(IMAGE_MINIMAP_INTERFACE + 5, Tx1 - NpcWidth / 1.5, Ty1 - NpcWidth, NpcWidth, NpcWidth, 0.f, 0.f, 17.5f / 32.f, 17.5f / 32.f);
					glColor4f(1.f, 1.f, 1.f, 1.f);
				}
			}
			else if (m_Mini_Map_Data[i].Kind == 2) // NPC
			{
				if (CheckMouseIn(Tx1 - NpcWidthP / 2, Ty1 - NpcWidthP / 1.5, NpcWidthP, NpcWidthP)) {
					Hoving = i;
				}
				if (Hoving == i) {
					HverPos.x = Tx1 - NpcWidthP / 2;
					HverPos.y = Ty1 - NpcWidthP / 1.5;
				}
				if (Hoving == i) glColor4f(1.f, 1.f, 0.15f, 1.f); else glColor4f(1.f, 1.f, 1.f, 1.f);
				RenderImage(IMAGE_MINIMAP_INTERFACE + 4, Tx1 - NpcWidthP / 2, Ty1 - NpcWidthP / 1.5, NpcWidthP, NpcWidthP, 0.f, 0.f, 17.5f / 32.f, 17.5f / 32.f);
				glColor4f(1.f, 1.f, 1.f, 1.f);
			}
		}
		else
			break;
	}

	// Player position (with zoom/pan)
	float Ch_wid = 12;
	float playerSX, playerSY;
	MapToScreen((float)Hero->PositionX, (float)Hero->PositionY, playerSX, playerSY);
	if (playerSX >= m_Pos.x && playerSX <= m_Pos.x + m_Size.cx && playerSY >= m_Pos.y && playerSY <= m_Pos.y + m_Size.cy) {
		RenderImage(IMAGE_MINIMAP_INTERFACE + 3, playerSX - Ch_wid / 2, playerSY - Ch_wid, Ch_wid, Ch_wid, 0.f, 0.f, 17.5f / 32.f, 17.5f / 32.f);
	}

	// Party members (with zoom/pan)
	for (int i = 0; i < PartyNumber; i++)
	{
		PARTY_t* pMember = &Party[i];
		if (pMember->Map == World && strcmp(Party[i].Name, Hero->ID)) {
			float psx, psy;
			MapToScreen((float)pMember->x, (float)pMember->y, psx, psy);
			if (psx >= m_Pos.x && psx <= m_Pos.x + m_Size.cx && psy >= m_Pos.y && psy <= m_Pos.y + m_Size.cy) {
				if (CheckMouseIn(psx - Ch_wid / 2, psy - Ch_wid, Ch_wid, Ch_wid)) {
					HvgPt = i;
				}
				glColor4f(1.f, 0.23f, 1.f, 1.f);
				RenderImage(IMAGE_MINIMAP_INTERFACE + 3, psx - Ch_wid / 2, psy - Ch_wid, Ch_wid, Ch_wid, 0.f, 0.f, 17.5f / 32.f, 17.5f / 32.f);
				glColor4f(1.f, 1.f, 1.f, 1.f);
			}
		}
	}

	// Tooltip / coordinate display
	float fMapX, fMapY;
	ScreenToMap(MouseX, MouseY, fMapX, fMapY);
	int toX = max(min((int)fMapX, 255), 0);
	int toY = max(min((int)fMapY, 255), 0);

	if (Hoving != -1) {
		g_pRenderText->RenderText(HverPos.x - (strlen(m_Mini_Map_Data[Hoving].Name) * 1), HverPos.y - 8, m_Mini_Map_Data[Hoving].Name);
	}
	else if (HvgPt != -1) {
		float ptx, pty;
		MapToScreen((float)Party[HvgPt].x, (float)Party[HvgPt].y, ptx, pty);
		g_pRenderText->RenderText(ptx - strlen(Party[HvgPt].Name), pty - Ch_wid - 8, Party[HvgPt].Name);
	}
	else if (CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy)) {
		char zChar[255];
		sprintf(zChar, "%d:%d", toX, toY);
		g_pRenderText->RenderText(MouseX - 15, MouseY - 15, zChar, 30, 0, 3);
	}

	// Click to move on map (left click)
	if (IsPress(VK_LBUTTON) && !m_bDragging) {
		if (Hoving != -1) {
			toX = m_Mini_Map_Data[Hoving].Location[0];
			toY = m_Mini_Map_Data[Hoving].Location[1];
		}
		if (CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy)) {
			if (ValidateMove(toX, toY)) {
				if (this->FindPath(Hero->PositionX, Hero->PositionY, toX, toY)) {
					this->m_MoveX = toX;
					this->m_MoveY = toY;
					this->m_Map = World;
				}
			}
		}
	}

	// Show zoom level if zoomed in
	if (m_fZoom > 1.01f) {
		char zoomText[32];
		sprintf(zoomText, "x%.1f", m_fZoom);
		g_pRenderText->SetTextColor(255, 255, 100, 255);
		g_pRenderText->RenderText(m_Pos.x + 5, m_Pos.y + m_Size.cy - 18, zoomText);
		g_pRenderText->SetTextColor(255, 255, 255, 255);
	}

	DisableAlphaBlend();

	return true;
}

bool SEASON3B::CNewUIMiniMap::RenderTopMost()
{
	if (IsVisible() == false)
	{
		return true;
	}

	m_bRenderTopMostPass = true;
	bool result = Render();
	m_bRenderTopMostPass = false;

	return result;
}

bool SEASON3B::CNewUIMiniMap::ValidateMove(int ToX, int ToY)
{
	if (ToX == Hero->PositionX && Hero->PositionY == ToY) {
		return false;
	}
	return true;
}

bool SEASON3B::CNewUIMiniMap::Update()
{
	if (PartyNumber && GetTickCount() > 500) {
		SendRequestPartyList();
	}
	return true;
}

void SEASON3B::CNewUIMiniMap::LoadImages(const char* Filename)
{
	char Fname[300];
	int i = 0;
	sprintf(Fname, "Data\\%s\\mini_map.ozt", Filename);
	FILE* pFile = fopen(Fname, "rb");

	DeleteBitmap(IMAGE_MINIMAP_INTERFACE);

	if (pFile == NULL)
	{
		m_bSuccess = false;
		return;
	}
	else
	{
		m_bSuccess = true;
		fclose(pFile);
		sprintf(Fname, "%s\\mini_map.tga", Filename);
		LoadBitmap(Fname, IMAGE_MINIMAP_INTERFACE, GL_LINEAR);
	}

	sprintf(Fname, "Data\\Local\\%s\\Minimap\\Minimap_%s_%s.bmd", g_strSelectedML.c_str(), Filename, g_strSelectedML.c_str());

	for (i = 0; i < MAX_MINI_MAP_DATA; i++)
	{
		m_Mini_Map_Data[i].Kind = 0;
	}

	FILE* fp = fopen(Fname, "rb");

	if (fp != NULL)
	{
		int Size = sizeof(MINI_MAP);
		BYTE* Buffer = new BYTE[Size * MAX_MINI_MAP_DATA + 45];
		fread(Buffer, (Size * MAX_MINI_MAP_DATA) + 45, 1, fp);

		DWORD dwCheckSum;
		fread(&dwCheckSum, sizeof(DWORD), 1, fp);
		fclose(fp);

		if (dwCheckSum != GenerateCheckSum2(Buffer, (Size * MAX_MINI_MAP_DATA) + 45, 0x2BC1))
		{
			char Text[256];
			sprintf(Text, "%s - File corrupted.", Fname);
			g_ErrorReport.Write(Text);
			MessageBox(g_hWnd, Text, NULL, MB_OK);
			SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		}
		else
		{
			BYTE* pSeek = Buffer;

			for (i = 0; i < MAX_MINI_MAP_DATA; i++)
			{
				BuxConvert(pSeek, Size);
				memcpy(&(m_Mini_Map_Data[i]), pSeek, Size);
				pSeek += Size;
			}
		}

		delete[] Buffer;
	}
}

void SEASON3B::CNewUIMiniMap::UnloadImages()
{
	DeleteBitmap(IMAGE_MINIMAP_INTERFACE);
}

bool SEASON3B::CNewUIMiniMap::IsSuccess()
{
	return m_bSuccess;
}

bool SEASON3B::CNewUIMiniMap::UpdateMouseEvent()
{
	// Handle mouse wheel zoom when mouse is over the map
	if (CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy))
	{
		if (MouseWheel != 0)
		{
			// Get map coordinate under mouse BEFORE zoom
			float mapXBefore, mapYBefore;
			ScreenToMap(MouseX, MouseY, mapXBefore, mapYBefore);

			// Apply zoom
			if (MouseWheel > 0)
				m_fZoom *= 1.25f;  // Zoom in
			else
				m_fZoom /= 1.25f;  // Zoom out

			// Clamp zoom: 1x (full map) to 4x
			if (m_fZoom < 1.0f) m_fZoom = 1.0f;
			if (m_fZoom > 4.0f) m_fZoom = 4.0f;

			// Adjust pan so the map point under cursor stays fixed
			// After zoom: screenX = m_Pos.x + (mapY - panX) * newZoom
			// We want the same screen pos, so:
			// panX = mapY - (MouseX - m_Pos.x) / newZoom
			m_fPanX = mapYBefore - (float)(MouseX - m_Pos.x) / m_fZoom;
			m_fPanY = mapXBefore - (float)(MouseY - m_Pos.y) / m_fZoom;
			ClampPan();

			MouseWheel = 0;
		}

		// Handle right-click drag for panning
		if (MouseRButtonPush && !m_bDragging)
		{
			m_bDragging = true;
			m_iDragStartX = MouseX;
			m_iDragStartY = MouseY;
			m_fDragStartPanX = m_fPanX;
			m_fDragStartPanY = m_fPanY;
			MouseRButtonPush = false;
		}
	}

	// Continue drag while right button held
	if (m_bDragging)
	{
		if (MouseRButton)
		{
			int dx = MouseX - m_iDragStartX;
			int dy = MouseY - m_iDragStartY;
			m_fPanX = m_fDragStartPanX - (float)dx / m_fZoom;
			m_fPanY = m_fDragStartPanY - (float)dy / m_fZoom;
			ClampPan();
			// Consume right button to prevent game action
			MouseRButtonPush = false;
		}
		else
		{
			m_bDragging = false;
		}
	}

	if (IsPress(VK_LBUTTON))
	{
		Check_Mouse(MouseX, MouseY);
	}

	if (CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy))
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIMiniMap::Check_Mouse(int mx, int my)
{
	return true;
}

bool SEASON3B::CNewUIMiniMap::Check_Btn(int mx, int my)
{
	int i = 0;
	for (i = 0; i < MAX_MINI_MAP_DATA; i++)
	{
		if (m_Mini_Map_Data[i].Kind > 0)
		{
			if (mx > m_Btn_Loc[i][0] && mx < (m_Btn_Loc[i][0] + m_Btn_Loc[i][2]) && my > m_Btn_Loc[i][1] && my < (m_Btn_Loc[i][1] + m_Btn_Loc[i][3]))
			{
				SIZE Fontsize;
				m_TooltipText = (unicode::t_string)m_Mini_Map_Data[i].Name;
				g_pRenderText->SetFont(g_hFont);
				g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_TooltipText.c_str(), m_TooltipText.size(), &Fontsize);

				Fontsize.cx = Fontsize.cx / ((float)WindowWidth / 640);
				Fontsize.cy = Fontsize.cy / ((float)WindowHeight / 480);

				int x = m_Btn_Loc[i][0] + ((m_Btn_Loc[i][2] / 2) - (Fontsize.cx / 2));
				int y = m_Btn_Loc[i][1] - (Fontsize.cy + 2);

				DWORD backuptextcolor = g_pRenderText->GetTextColor();
				DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

				g_pRenderText->SetTextColor(RGBA(255, 255, 255, 255));
				g_pRenderText->SetBgColor(RGBA(0, 0, 0, 180));
				g_pRenderText->RenderText(x, y, m_TooltipText.c_str(), Fontsize.cx + 6, 0, RT3_SORT_CENTER);

				g_pRenderText->SetTextColor(backuptextcolor);
				g_pRenderText->SetBgColor(backuptextbackcolor);

				return true;
			}
		}
		else
			break;
	}
	return false;
}

bool SEASON3B::CNewUIMiniMap::movement_automatic()
{
	if (this->IsMoving())
	{
		if (this->m_Map != World || Hero->Dead != 0)
		{
			this->StopMove();
			return false;
		}

		auto pos = (&m_WayPoint)->front();

		float distance = sqrt(pow((float)(Hero->PositionX - pos.first), 2) + pow((float)(Hero->PositionY - pos.second), 2));

		if (distance < 2)
		{
			(&m_WayPoint)->pop_front();

			if (!(&m_WayPoint)->empty())
			{
				pos = (&m_WayPoint)->front();

				if (Hero->Movement != 0 && PathFinding2(Hero->PositionX, Hero->PositionY, pos.first, pos.second, &Hero->Path, 0.f))
				{
					SendMove(Hero, &Hero->Object);
				}
			}
		}
		else if (distance >= 12.f)
		{
			this->StopMove();
		}
		else if (Hero->Movement == 0 && PathFinding2(Hero->PositionX, Hero->PositionY, pos.first, pos.second, &Hero->Path, 0.f))
		{
			Hero->Movement = true;
			SendMove(Hero, &Hero->Object);
		}

		return false;
	}

	return true;
}

void SEASON3B::CNewUIMiniMap::StopMove(CHARACTER* c)
{
	if (this->IsMoving())
	{
		(&m_WayPoint)->clear();

		if (Hero->Movement)
		{
			SetPlayerStop(Hero);
		}
	}
}

bool SEASON3B::CNewUIMiniMap::IsMoving()
{
	return !(&m_WayPoint)->empty();
}
