#pragma once
#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIScrollBar.h"


namespace SEASON3B
{
	class CNewUIMenuUser : public CNewUIObj
	{
	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;
		CNewUIScrollBarHTML m_MenuScrollBar;
	public:
		CNewUIMenuUser();
		virtual~CNewUIMenuUser();
		bool Create(CNewUIManager* pNewUIMng, float x, float y);
		void Release();

		void SetPos(float x, float y);
		void LoadImages();
		void UnloadImages();

		bool UpdateKeyEvent();
		bool UpdateMouseEvent();
		bool Render();
		bool Update();
		float GetLayerDepth(); //. 10.5f
		float GetKeyEventOrder();

		void OpenningProcess();
		void ClosingProcess();
	private:
		bool ExecuteMenuAction(int actionId);
		void RenderFrame();
		void RenderTexte();
		void RenderButtons();
		void RenderButton(float x, float y, float width, float height);
	};

}
