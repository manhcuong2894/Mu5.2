#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIRankingTop.h"
#include "ZzzTexture.h"
#include "wsclientinline.h"
#include "Protocol.h"
#include "CharacterManager.h"
#include "CGMFrame.h"
#include "CBInterface.h"
#include "TextClient.h"
#include "MapManager.h"

struct PMSG_CUSTOM_RANKING_COUNT_RECV_GS
{
	PSWMSG_HEAD header;
	int count;
	char RankNames[10][30];
};

namespace
{
	constexpr float kRankingWindowWidth = 520.0f;
	constexpr float kRankingWindowHeight = 320.0f;
	constexpr float kRankingSidebarX = 10.0f;
	constexpr float kRankingSidebarY = 30.0f;
	constexpr float kRankingSidebarWidth = 110.0f;
	constexpr float kRankingSidebarHeight = kRankingWindowHeight - 50.0f;
	constexpr float kRankingTabDrawSize = 150.0f;
	constexpr float kRankingTabWidth = kRankingSidebarWidth + 5.0f;
	constexpr float kRankingTabHeight = (kRankingTabDrawSize * 20.0f) / 100.0f;
	constexpr float kRankingTabStepY = 30.0f;
	constexpr float kRankingContentX = kRankingSidebarX + kRankingSidebarWidth + 10.0f;
	constexpr float kRankingContentY = 30.0f;
	constexpr float kRankingContentRightPadding = 6.0f;
	constexpr float kRankingContentWidth = kRankingWindowWidth - kRankingContentX - kRankingContentRightPadding;
	constexpr float kRankingContentHeight = kRankingWindowHeight - 40.0f;
	constexpr float kRankingListTopOffset = 48.0f;
	constexpr float kRankingRowHeight = 16.0f;
	constexpr float kRankingScrollGutterWidth = 16.0f;
	constexpr float kRankingScrollOffsetX = 8.0f;
	constexpr float kRankingScrollTopOffset = 52.0f;
	constexpr float kRankingScrollHeight = kRankingContentHeight - 62.0f;

	size_t GetVisibleRankingRows()
	{
		int visibleRows = (int)((kRankingContentHeight - 64.0f) / kRankingRowHeight);
		return ((visibleRows > 1) ? (size_t)visibleRows : 1u);
	}

	void DrawActiveRankingTabGlow(float x, float y, float width, float height)
	{
		float perimeter = (width + height) * 2.0f;
		int glowLength = (int)(perimeter / 7.0f);

		if (glowLength < 24)
		{
			glowLength = 24;
		}
		else if (glowLength > 52)
		{
			glowLength = 52;
		}

		gInterface->DrawGlowAroundBox2(x, y, width, height, glowLength, 0.15f, 1.0f, 0.55f);
	}
}

SEASON3B::CNewUIRankingTop::CNewUIRankingTop()
{
	m_pNewUIMng = NULL;
	m_Pos.x = 0;
	m_Pos.y = 0;

	is_request = false;
	m_RankListView = GetVisibleRankingRows();
	m_RankMaxTop = 0;
	m_RankIndexCur = 0;
	m_RankSelectIndex = -1;
	memset(this->m_RankName, 0, sizeof(this->m_RankName));
	memset(this->m_RankColum1, 0, sizeof(this->m_RankColum1));
	memset(this->m_RankColum2, 0, sizeof(this->m_RankColum2));

	m_bShowDropdown = false;
	m_bRenderTopMostPass = false;
	m_RankingNames.clear();
}

SEASON3B::CNewUIRankingTop::~CNewUIRankingTop()
{
	Release();
}

bool SEASON3B::CNewUIRankingTop::Create(CNewUIManager* pNewUIMng, float x, float y)
{
	bool Success = false;

	if (pNewUIMng)
	{
		m_pNewUIMng = pNewUIMng;
		m_pNewUIMng->AddUIObj(INTERFACE_RANKING_TOP, this);
		this->LoadImages();
		this->SetPos(x, y);
		this->SetInfo();
		this->Show(false);
		Success = true;
	}

	return Success;
}

void SEASON3B::CNewUIRankingTop::Release()
{
	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		this->UnloadImages();
		m_RankList.clear();
	}
}

void SEASON3B::CNewUIRankingTop::SetInfo()
{
	m_RankListView = GetVisibleRankingRows();
	m_pScrollBar.Create(m_Pos.x + kRankingContentX + kRankingContentWidth - kRankingScrollOffsetX,
		m_Pos.y + kRankingContentY + kRankingScrollTopOffset, kRankingScrollHeight);
	m_pScrollBar.SetPercent(0.0);
}

void SEASON3B::CNewUIRankingTop::SetPos(float x, float y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_pScrollBar.SetPos(m_Pos.x + kRankingContentX + kRankingContentWidth - kRankingScrollOffsetX,
		m_Pos.y + kRankingContentY + kRankingScrollTopOffset);
}

void SEASON3B::CNewUIRankingTop::LoadImages()
{
	LoadBitmap("Interface\\HUD\\VipLevel1.tga", IMAGE_TOP_LEVEL1, GL_LINEAR, GL_CLAMP_TO_EDGE, true, false);
	LoadBitmap("Interface\\HUD\\VipLevel2.tga", IMAGE_TOP_LEVEL2, GL_LINEAR, GL_CLAMP_TO_EDGE, true, false);
	LoadBitmap("Interface\\HUD\\VipLevel3.tga", IMAGE_TOP_LEVEL3, GL_LINEAR, GL_CLAMP_TO_EDGE, true, false);
}

void SEASON3B::CNewUIRankingTop::UnloadImages()
{
	DeleteBitmap(IMAGE_TOP_LEVEL1);
	DeleteBitmap(IMAGE_TOP_LEVEL2);
	DeleteBitmap(IMAGE_TOP_LEVEL3);
}

bool SEASON3B::CNewUIRankingTop::UpdateKeyEvent()
{
	if (IsVisible() == true)
	{
		if (gInterface->Data[eWindowRankingTop].OnShow == false)
		{
			g_pNewUISystem->Hide(INTERFACE_RANKING_TOP);
			return false;
		}

		if (SEASON3B::IsPress(VK_ESCAPE))
		{
			g_pNewUISystem->Hide(INTERFACE_RANKING_TOP);
			return false;
		}

		if (SEASON3B::IsRelease(VK_LEFT) && m_RankIndexCur > 0)
		{
			this->RequestServerRankingInfo((BYTE)(m_RankIndexCur - 1));
			return false;
		}

		if (SEASON3B::IsRelease(VK_RIGHT) && (m_RankIndexCur + 1) < m_RankMaxTop)
		{
			this->RequestServerRankingInfo((BYTE)(m_RankIndexCur + 1));
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIRankingTop::UpdateMouseEvent()
{
	if (IsVisible() == false)
	{
		return true;
	}

	if (gInterface->Data[eWindowRankingTop].OnShow == false)
	{
		g_pNewUISystem->Hide(INTERFACE_RANKING_TOP);
		return false;
	}

	float windowX = (float)m_Pos.x;
	float windowY = (float)m_Pos.y;
	float listX = windowX + kRankingContentX + 6.0f;
	float listY = windowY + kRankingContentY + kRankingListTopOffset;
	float listWidth = kRankingContentWidth - kRankingScrollGutterWidth;
	float listHeight = kRankingContentHeight - kRankingListTopOffset - 8.0f;
	bool isInsideWindow = (SEASON3B::CheckMouseIn(windowX, windowY - 10.0f, kRankingWindowWidth + 24.0f, kRankingWindowHeight + 20.0f) == 1);

	if (isInsideWindow)
	{
		size_t secure = 0;
		size_t current = 0;
		size_t good_count = m_RankList.size();

		if (good_count > m_RankListView)
		{
			if (SEASON3B::CheckMouseIn(listX, listY, listWidth + kRankingScrollGutterWidth, listHeight))
			{
				double nextPercent = m_pScrollBar.GetPercent();

				if (MouseWheel <= 0)
				{
					if (MouseWheel < 0)
					{
						MouseWheel = 0;
						nextPercent += 0.1;
						if (nextPercent > 1.0)
						{
							nextPercent = 1.0;
						}
						m_pScrollBar.SetPercent((float)nextPercent);
					}
				}
				else
				{
					MouseWheel = 0;
					nextPercent -= 0.1;
					if (nextPercent < 0.0)
					{
						nextPercent = 0.0;
					}
					m_pScrollBar.SetPercent((float)nextPercent);
				}
			}

			m_pScrollBar.UpdateMouseEvent();
			current = (int)((double)(unsigned int)(good_count - m_RankListView) * m_pScrollBar.GetPercent());
		}

		for (size_t i = current; i < good_count && secure < m_RankListView; i++, secure++)
		{
			float rowY = listY + (secure * kRankingRowHeight);

			if (SEASON3B::CheckMouseIn(listX, rowY, listWidth, kRankingRowHeight) && SEASON3B::IsRelease(VK_LBUTTON))
			{
				m_RankSelectIndex = i;
				return false;
			}
		}

		return false;
	}

	return true;
}

bool SEASON3B::CNewUIRankingTop::Render()
{
	if (IsVisible() && gInterface->Data[eWindowRankingTop].OnShow == false)
	{
		g_pNewUISystem->Hide(INTERFACE_RANKING_TOP);
		return true;
	}

	if (m_bRenderTopMostPass == false)
	{
		return true;
	}

	EnableAlphaTest(true);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	this->RenderFrame();
	if (gInterface->Data[eWindowRankingTop].OnShow == false)
	{
		return true;
	}

	this->RenderTexte();
	DisableAlphaBlend();

	return true;
}

bool SEASON3B::CNewUIRankingTop::RenderTopMost()
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

bool SEASON3B::CNewUIRankingTop::Update()
{
	if (IsVisible())
	{
		if (gInterface->Data[eWindowRankingTop].OnShow == false)
		{
			g_pNewUISystem->Hide(INTERFACE_RANKING_TOP);
			return true;
		}

		SetPos(gInterface->Data[eWindowRankingTop].X, gInterface->Data[eWindowRankingTop].Y);
		m_pScrollBar.Update();
	}

	return true;
}

float SEASON3B::CNewUIRankingTop::GetLayerDepth()
{
	return 20.0f;
}

void SEASON3B::CNewUIRankingTop::OpenningProcess()
{
	is_request = false;
	m_RankMaxTop = 0;
	m_RankIndexCur = 0;
	m_RankSelectIndex = -1;
	m_RankingNames.clear();
	m_RankList.clear();
	m_bShowDropdown = false;
	m_pScrollBar.SetPercent(0.0);

	SetPos(PositionX_The_Mid(kRankingWindowWidth), PositionY_The_Mid(kRankingWindowHeight));
	gInterface->Data[eWindowRankingTop].Open();
	gInterface->Data[eWindowRankingTop].FirstLoad = false;
	gInterface->Data[eWindowRankingTop].AllowMove = true;

	SendRequestMaxRanking();
}

void SEASON3B::CNewUIRankingTop::ClosingProcess()
{
	m_bShowDropdown = false;
	is_request = false;
	gInterface->Data[eWindowRankingTop].Close();
	gInterface->Data[eWindowRankingTop].FirstLoad = false;
}

void SEASON3B::CNewUIRankingTop::RenderFrame()
{
	float startX = (float)m_Pos.x;
	float startY = (float)m_Pos.y;

	if (gInterface->gDrawWindowCustom(&startX, &startY, kRankingWindowWidth, kRankingWindowHeight, eWindowRankingTop, "%s", gTextClient.txtClient_Ranking[0]) == false)
	{
		g_pNewUISystem->Hide(INTERFACE_RANKING_TOP);
		return;
	}

	SetPos(startX, startY);

	gInterface->DrawInfoBox(startX + kRankingSidebarX, startY + kRankingSidebarY, kRankingSidebarWidth, kRankingSidebarHeight, 0x00000096, 0);
	gInterface->DrawBarForm(startX + kRankingContentX, startY + kRankingContentY, kRankingContentWidth, kRankingContentHeight, 0.0f, 0.0f, 0.0f, 0.7f);
}

void SEASON3B::CNewUIRankingTop::RenderTexte()
{
	float windowX = (float)m_Pos.x;
	float windowY = (float)m_Pos.y;
	float sidebarX = windowX + kRankingSidebarX;
	float sidebarY = windowY + kRankingSidebarY;
	float contentX = windowX + kRankingContentX;
	float contentY = windowY + kRankingContentY;
	float headerX = contentX + 6.0f;
	float headerY = contentY + 24.0f;
	float rowX = headerX;
	float rowY = contentY + kRankingListTopOffset;
	float rowWidth = kRankingContentWidth - kRankingScrollGutterWidth;
	float rankColWidth = 30.0f;
	float nameColWidth = 65.0f;
	float classColWidth = 75.0f;
	float scoreColWidth = 130.0f;
	float stateColWidth = rowWidth - rankColWidth - nameColWidth - classColWidth - scoreColWidth;
	size_t tabCount = ((m_RankingNames.size() < m_RankMaxTop) ? m_RankingNames.size() : m_RankMaxTop);
	size_t secure = 0;
	size_t current = 0;
	size_t good_count = m_RankList.size();
	char textBuffer[128];

	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(-1);
	g_pRenderText->SetFont(g_hFontBold);

	for (size_t i = 0; i < tabCount; ++i)
	{
		float tabY = sidebarY + (i * kRankingTabStepY);

		if (gInterface->DrawButton(sidebarX, tabY, kRankingTabDrawSize, 12, m_RankingNames[i].c_str(), kRankingTabWidth) && i != m_RankIndexCur)
		{
			this->RequestServerRankingInfo((BYTE)i);
		}

		if (m_RankIndexCur == i)
		{
			DrawActiveRankingTabGlow(sidebarX - 1.0f, tabY - 1.0f, kRankingTabWidth + 2.0f, kRankingTabHeight);
		}
	}

	if (tabCount == 0)
	{
		SEASON3B::TextDraw((HFONT)g_hFont, sidebarX + 6.0f, sidebarY + 8.0f, 0xE0D19DFF, 0x0, (int)kRankingSidebarWidth, 0, 1, "Dang tai BXH...");
	}

	const char* currentRankName = ((m_RankIndexCur < m_RankingNames.size()) ? m_RankingNames[m_RankIndexCur].c_str() : ((m_RankName[0] != 0) ? m_RankName : gTextClient.txtClient_Ranking[0]));
	SEASON3B::TextDraw((HFONT)g_hFontBig, contentX + 4.0f, contentY, 0x19FF9FFF, 0x0, 0, 0, 1, "%s", currentRankName);
	SEASON3B::TextDraw((HFONT)g_hFont, contentX + kRankingContentWidth - 85.0f, contentY + 2.0f, 0xE0D19DFF, 0x0, 0, 0, 1, "Top %d", (int)m_RankList.size());

	gInterface->DrawBarForm(headerX, headerY, rowWidth, 18.0f, 0.20f, 0.22f, 0.26f, 1.0f);

	g_pRenderText->RenderFont((int)headerX, (int)headerY, "#", (int)rankColWidth, 18, RT3_SORT_CENTER);
	g_pRenderText->RenderFont((int)(headerX + rankColWidth + 15), (int)headerY, gTextClient.txtClient_Ranking[6], (int)nameColWidth, 18, RT3_SORT_LEFT);
	g_pRenderText->RenderFont((int)(headerX + rankColWidth + nameColWidth), (int)headerY, this->m_RankColum1, (int)classColWidth, 18, RT3_SORT_CENTER);
	g_pRenderText->RenderFont((int)(headerX + rankColWidth + nameColWidth + classColWidth), (int)headerY, this->m_RankColum2, (int)scoreColWidth, 18, RT3_SORT_CENTER);
	g_pRenderText->RenderFont((int)(headerX + rankColWidth + nameColWidth + classColWidth + scoreColWidth), (int)headerY, gTextClient.txtClient_Ranking[8], (int)stateColWidth, 18, RT3_SORT_CENTER);

	if (good_count > m_RankListView)
	{
		double prev = m_pScrollBar.GetPercent();
		current = (int)((double)(unsigned int)(good_count - m_RankListView) * prev);
	}

	if (good_count == 0)
	{
		const char* emptyText = (is_request ? "Dang tai du lieu BXH..." : "Chua co du lieu BXH.");
		SEASON3B::TextDraw((HFONT)g_hFont, contentX + 12.0f, rowY + 12.0f, 0xE0D19DFF, 0x0, (int)(kRankingContentWidth - 24.0f), 0, 3, "%s", emptyText);
	}

	for (size_t i = current; i < good_count && secure < m_RankListView; i++, secure++)
	{
		float currentRowY = rowY + (secure * kRankingRowHeight);
		BYTE vipLevel = m_RankList[i].GetVip();

		if (SEASON3B::CheckMouseIn(rowX, currentRowY, rowWidth, kRankingRowHeight) || m_RankSelectIndex == i)
		{
			gInterface->DrawBarForm(rowX, currentRowY, rowWidth, kRankingRowHeight - 1.0f, 0.31f, 0.34f, 0.39f, 0.85f);
		}

		sprintf_s(textBuffer, "%02d", (int)(i + 1));
		g_pRenderText->SetTextColor(-1);
		g_pRenderText->RenderFont((int)rowX, (int)currentRowY, textBuffer, (int)rankColWidth, (int)kRankingRowHeight, RT3_SORT_CENTER);

		if (vipLevel > 0)
		{
			SEASON3B::RenderImage(IMAGE_TOP_BACK3 + vipLevel, rowX + rankColWidth, currentRowY + 3.0f, 10.0f, 10.0f, 0.0, 0.0, 1.0f, 1.0f, -1);
		}

		g_pRenderText->RenderFont((int)(rowX + rankColWidth + 14.0f), (int)currentRowY, m_RankList[i].GetName(), (int)(nameColWidth - 14.0f), (int)kRankingRowHeight, RT3_SORT_LEFT);
		g_pRenderText->RenderFont((int)(rowX + rankColWidth + nameColWidth), (int)currentRowY, m_RankList[i].GetClass(), (int)classColWidth, (int)kRankingRowHeight, RT3_SORT_CENTER);

		if (m_RankIndexCur == 0)
		{
			int totalScore = m_RankList[i].GetScore();
			int resets = totalScore / 10000;
			int level = totalScore % 10000;

			if (strlen(m_RankList[i].GetDate()) > 0)
			{
				sprintf_s(textBuffer, "%d/%d - %s", level, resets, m_RankList[i].GetDate());
			}
			else
			{
				sprintf_s(textBuffer, "%d/%d", level, resets);
			}
		}
		else
		{
			if (strlen(m_RankList[i].GetDate()) > 0)
			{
				sprintf_s(textBuffer, "%02d - %s", m_RankList[i].GetScore(), m_RankList[i].GetDate());
			}
			else
			{
				sprintf_s(textBuffer, "%02d", m_RankList[i].GetScore());
			}
		}

		g_pRenderText->RenderFont((int)(rowX + rankColWidth + nameColWidth + classColWidth), (int)currentRowY, textBuffer, (int)scoreColWidth, (int)kRankingRowHeight, RT3_SORT_CENTER);

		if (m_RankList[i].GetIsOnline() > 0)
		{
			g_pRenderText->SetTextColor(100, 255, 100, 255);
			g_pRenderText->RenderFont((int)(rowX + rankColWidth + nameColWidth + classColWidth + scoreColWidth), (int)currentRowY, gMapManager->GetMapName(m_RankList[i].GetMap()), (int)stateColWidth, (int)kRankingRowHeight, RT3_SORT_CENTER);
		}
		else
		{
			g_pRenderText->SetTextColor(150, 150, 150, 255);
			g_pRenderText->RenderFont((int)(rowX + rankColWidth + nameColWidth + classColWidth + scoreColWidth), (int)currentRowY, "Offline", (int)stateColWidth, (int)kRankingRowHeight, RT3_SORT_CENTER);
		}
	}

	g_pRenderText->SetTextColor(-1);
	if (good_count > m_RankListView)
	{
		m_pScrollBar.Render();
	}
}

void SEASON3B::CNewUIRankingTop::ReceiveRankingInfo(BYTE* ReceiveBuffer)
{
	PMSG_CUSTOM_RANKING_COUNT_RECV_GS* Data = (PMSG_CUSTOM_RANKING_COUNT_RECV_GS*)ReceiveBuffer;

	m_RankSelectIndex = -1;

	if (Data->count > 0 && Data->count <= 10)
	{
		m_RankMaxTop = Data->count;

		m_RankingNames.clear();
		for (int i = 0; i < m_RankMaxTop; ++i)
		{
			m_RankingNames.push_back(Data->RankNames[i]);
		}

		if (m_RankIndexCur >= m_RankMaxTop)
		{
			m_RankIndexCur = 0;
		}
	}

	RequestServerRankingInfo((BYTE)m_RankIndexCur);
}

void SEASON3B::CNewUIRankingTop::ReceiveRankingListInfo(BYTE* ReceiveBuffer)
{
	LPPHEADER_RANKING_LIST Data = (LPPHEADER_RANKING_LIST)ReceiveBuffer;

	m_RankList.clear();
	m_pScrollBar.SetPercent(0.0);
	this->m_RankIndexCur = Data->index;
	m_RankSelectIndex = -1;

	memset(this->m_RankName, 0, sizeof(this->m_RankName));
	strcpy_s(this->m_RankName, Data->rankname);
	strcpy_s(this->m_RankColum1, Data->col1);
	strcpy_s(this->m_RankColum2, Data->col2);

	int offset = sizeof(PHEADER_RANKING_LIST);
	for (int n = 0; n < Data->count; n++)
	{
		LPPCREATE_RANKING_INFO Data2 = (LPPCREATE_RANKING_INFO)(ReceiveBuffer + offset);

		BYTE baseClass = Data2->PlayerClass / 16;
		BYTE upgradeTier = Data2->PlayerClass % 16;
		BYTE Class = baseClass;

		if (upgradeTier == 1)
		{
			Class |= 16;
		}
		else if (upgradeTier == 2)
		{
			Class |= 32;
		}
		else if (upgradeTier >= 3)
		{
			Class |= 64;
		}

		m_RankList.push_back(TEMPLATE_RANKING(Data2->Name, Data2->szDate, gCharacterManager.GetCharacterClassText(Class), Class, Data2->LevelVip, Data2->TotalScore, Data2->IsOnline, Data2->Map));
		offset += sizeof(PCREATE_RANKING_INFO);
	}

	std::sort(m_RankList.begin(), m_RankList.end(), [](const TEMPLATE_RANKING& a, const TEMPLATE_RANKING& b)
	{
		return a.Score > b.Score;
	});

	if (m_RankList.empty() == false)
	{
		m_RankSelectIndex = 0;
	}

	is_request = false;
}

void SEASON3B::CNewUIRankingTop::RequestServerRankingInfo(BYTE Index)
{
	if (Index >= m_RankMaxTop || is_request == true)
	{
		return;
	}

	if (Index == m_RankIndexCur && m_RankList.empty() == false)
	{
		return;
	}

	is_request = true;
	m_RankIndexCur = Index;
	m_RankSelectIndex = -1;
	m_RankList.clear();
	m_pScrollBar.SetPercent(0.0);
	SendRequestRankingInfo(Index);
}
