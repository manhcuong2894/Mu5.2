#pragma once
#include "NewUIBase.h"
#include "NewUIManager.h"


namespace SEASON3B
{
	struct TEMPLATE_RANKING
	{
		std::string Name;
		std::string Date;
		std::string Class;
		BYTE ClassByte;
		BYTE Vip;
		DWORD Score;
		BYTE IsOnline;
		int Map;
		TEMPLATE_RANKING(const std::string& n, const std::string& d, const std::string& c, BYTE cb, BYTE v, DWORD s, BYTE online, int m)
			: Name(n), Date(d), Class(c), ClassByte(cb), Vip(v), Score(s), IsOnline(online), Map(m)
		{
		}
		BYTE GetClassByte() { return ClassByte; }
		BYTE GetVip() { return Vip; }
		DWORD GetScore() { return Score; }
		BYTE GetIsOnline() { return IsOnline; }
		int GetMap() { return Map; }
		const char* GetName() {
			return Name.c_str();
		}
		const char* GetDate() {
			return Date.c_str();
		}
		const char* GetClass() {
			return Class.c_str();
		}
	};

	class CNewUIRankingTop : public CNewUIObj
	{
		enum IMAGE_LIST
		{
			IMAGE_TOP_BACK1 = BITMAP_IMAGE_FRAME_EMU + 1,
			IMAGE_TOP_BACK2 = BITMAP_IMAGE_FRAME_EMU + 2,
			IMAGE_TOP_BACK3 = BITMAP_IMAGE_FRAME_EMU + 3,
			IMAGE_TOP_LEVEL1 = BITMAP_IMAGE_FRAME_EMU + 4,
			IMAGE_TOP_LEVEL2 = BITMAP_IMAGE_FRAME_EMU + 5,
			IMAGE_TOP_LEVEL3 = BITMAP_IMAGE_FRAME_EMU + 6,
		};
	private:
		bool m_bShowDropdown;
		bool m_bRenderTopMostPass;
		std::vector<std::string> m_RankingNames;

		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;


		bool is_request;
		size_t m_RankListView;
		size_t m_RankMaxTop;
		size_t m_RankIndexCur;
		size_t m_RankSelectIndex;
		char m_RankName[50];
		char m_RankColum1[50];
		char m_RankColum2[50];
		CNewUIScrollBarHTML m_pScrollBar;
		std::vector<TEMPLATE_RANKING> m_RankList;
	public:

		CNewUIRankingTop();
		virtual ~CNewUIRankingTop();


		bool Create(CNewUIManager* pNewUIMng, float x, float y);
		void Release();
		void SetInfo();
		void SetPos(float x, float y);
		void LoadImages();
		void UnloadImages();
		bool UpdateKeyEvent();
		bool UpdateMouseEvent();
		bool Render();
		bool RenderTopMost();
		bool Update();
		float GetLayerDepth(); //. 10.5f

		void OpenningProcess();
		void ClosingProcess();

		void RenderFrame();
		void RenderTexte();


		void ReceiveRankingInfo(BYTE* ReceiveBuffer);
		void ReceiveRankingListInfo(BYTE* ReceiveBuffer);
	private:
		void RequestServerRankingInfo(BYTE Index);
	};
}





