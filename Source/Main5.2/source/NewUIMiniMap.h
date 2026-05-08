//////////////////////////////////////////////////////////////////////
// NewUIMiniMap.h - Flat 2D MiniMap with zoom & pan
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMainFrameWindow.h"
#include "NewUIChatLogWindow.h"
#include "NewUIMyInventory.h"
#include "PathFinding.h"

namespace SEASON3B
{
	class CNewUIMiniMap : public CNewUIObj, public CPathFinding
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_MINIMAP_INTERFACE = BITMAP_MINI_MAP_BEGIN,
		};

	private:
		unicode::t_string		m_TooltipText;
		CNewUIManager*			m_pNewUIMng;
		POINT					m_Pos;
		SIZE					m_Size;
		POINT					m_Lenth[6];
		int						m_MiniPos;
		CNewUIButton			m_BtnExit;
		MINI_MAP				m_Mini_Map_Data[MAX_MINI_MAP_DATA];
		float					m_Btn_Loc[MAX_MINI_MAP_DATA][4];

		// Zoom & Pan
		float					m_fZoom;        // 1.0 = full map, 2.0 = 2x zoom, etc.
		float					m_fPanX;        // Pan offset in map coords (0..255)
		float					m_fPanY;
		bool					m_bDragging;    // Right-click dragging
		int						m_iDragStartX;  // Mouse pos when drag started
		int						m_iDragStartY;
		float					m_fDragStartPanX;
		float					m_fDragStartPanY;
		bool					m_bRenderTopMostPass;

	public:
		bool					m_bSuccess;
		CNewUIMiniMap();
		virtual ~CNewUIMiniMap();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();

		void SetPos(int x, int y);
		void SetBtnPos(int Num, float x, float y, float nx, float ny);

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		bool RenderTopMost();
		bool ValidateMove(int ToX, int ToY);
		float GetLayerDepth();

		void OpenningProcess();
		void ClosingProcess();
		void LoadImages(const char* Filename);
		void UnloadImages();
		void StopMove(CHARACTER* c = NULL);
		int m_Map;
		int m_MoveX;
		int m_MoveY;
		bool movement_automatic();
		bool IsMoving();

		bool IsSuccess();

		// Convert screen pixel to map coordinate considering zoom/pan
		void ScreenToMap(int screenX, int screenY, float& mapX, float& mapY);
		// Convert map coordinate to screen pixel considering zoom/pan
		void MapToScreen(float mapX, float mapY, float& screenX, float& screenY);
		void ClampPan();

	private:
		bool Check_Mouse(int mx, int my);
		bool Check_Btn(int mx, int my);
	};
}
