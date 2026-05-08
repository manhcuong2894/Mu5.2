#pragma once

enum eTopMostOverlayWindow
{
	eTopMostOverlayQuaPhucLoi = 0,
	eTopMostOverlayEventTime,
	eTopMostOverlayVongQuay,
	eTopMostOverlayMocNap,
	eTopMostOverlayMiniMap,
	eTopMostOverlayRankingTop,
	eTopMostOverlayChangeItem,
};

bool IsTopMostOverlayWindowVisible(eTopMostOverlayWindow windowType);
bool CanOpenTopMostOverlayWindow(eTopMostOverlayWindow windowType);
void RenderTopMostOverlayWindows();

class CBDrawInterface
{
	CBDrawInterface();
	virtual ~CBDrawInterface();
public:
	static CBDrawInterface* Instance();
	void RenderFrame();
};
#define gCBDrawInterface			(CBDrawInterface::Instance())
