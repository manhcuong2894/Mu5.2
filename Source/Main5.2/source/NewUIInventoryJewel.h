#if !defined(AFX_NEWUIINVENTORYJEWEL_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
#define AFX_NEWUIINVENTORYJEWEL_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_
#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIButton.h"
#include "NewUIMyInventory.h"

class CUITextInputBox;

namespace SEASON3B
{
	class WareHoly
	{
	public:
		WareHoly() :bKeyIndex(0), bItemindex(-1), bItemLevel(-1), bItemCount(0), bBundledIndex(-1) {};
		WareHoly(BYTE KeyIndex, int itemindex, int itemLevel, __int64 ItemCount, int bundledIndex = -1) :
			bKeyIndex(KeyIndex), bItemindex(itemindex), bItemLevel(itemLevel), bItemCount(ItemCount), bBundledIndex(bundledIndex){
		};
		~WareHoly() {};

		void setIndex(int index) {
			bItemindex = index;
		};
		BYTE GetKeyIndex() const {
			return bKeyIndex;
		};
		int GetIndex() const {
			return bItemindex;
		};
		int GetLevel() const {
			if (bItemLevel != -1)
				return bItemLevel;
			else
				return 0;
		};
		void setValue(__int64 value) {
			bItemCount = value;
		};
		__int64 GetValue() const {
			return bItemCount;
		};
		void setBundledIndex(int index) {
			bBundledIndex = index;
		};
		int GetBundledIndex() const {
			return bBundledIndex;
		};

	private:
		BYTE bKeyIndex;
		int bItemindex;
		int bItemLevel;
		__int64 bItemCount;
		int bBundledIndex;
	};

	class CNewUIInventoryJewel : public CNewUIObj
	{
		enum IMAGE_LIST
		{
			IMAGE_INVENTORY_BACK = CNewUIMyInventory::IMAGE_INVENTORY_BACK,
			IMAGE_INVENTORY_BACK_TOP2 = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,
			IMAGE_INVENTORY_BACK_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_INVENTORY_BACK_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_INVENTORY_BACK_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			IMAGE_OPTION_BUTTON = Bitmap_jewel_bank_button,

		};
		enum
		{
			PAGE_ROW_COUNT = 7,
			QUICK_BUTTON_COUNT = 3,
		};
	public:
		CNewUIInventoryJewel();
		virtual~CNewUIInventoryJewel();
	private:
	/*+012*/ CNewUIManager* m_pNewUIMng;
	/*+020*/ POINT m_Pos;
	int		m_dwCurIndex;
	int		m_dwSelIndex;
	int		m_nSelPage;
	int		m_nMaxPage;
	RECT	m_nRectItem;

	CNewUIButton m_ButtonNext;
	CNewUIButton m_ButtonBack;
	CNewUIButton m_ButtonWithdrawSingle[PAGE_ROW_COUNT];
	CNewUIButton m_ButtonWithdrawQuick[PAGE_ROW_COUNT][QUICK_BUTTON_COUNT];
	CNewUIButton m_ButtonWithdrawRaw[PAGE_ROW_COUNT];
	CNewUIButton m_ButtonWithdrawPopupOk;
	CNewUIButton m_ButtonWithdrawPopupClose;
	CUITextInputBox* m_pWithdrawEditBox;
	bool	m_bIsEnableWithdrawPopup;

	std::vector<WareHoly>m_bItems;
	public:
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Initialize();
		void Release();
		void SetPos(int x, int y);
		void InitButtons();
		bool Render();
		bool Update();
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		float GetLayerDepth();
		bool CheckExpansionInventory();
		void OpenningProcess();
		void ClosingProcess();
		void RemoveData();
		int GetCurrentPage() const;
		void SetCurrentPage(int nPage);
		void InsertData(BYTE Index, short ItemIndex, short ItemLevel, __int64 count, short BundledIndex = -1);
		bool check_budget(DWORD _iCount);
		void ProcessInvenItem();
	private:
		void ConfigureActionButton(CNewUIButton& button, const char* text, float width);
		void LayoutButtons();
		void LayoutWithdrawPopup();
		void UpdatePageButtons();
		void OpenWithdrawPopup();
		void CloseWithdrawPopup(bool bClearText = true);
		bool UpdateWithdrawPopupMouseEvent();
		bool SubmitWithdrawPopup();
		int GetMaxPageFromCount(int itemCount) const;
		int GetVectorIndexByRow(int row) const;
		WareHoly* GetItemByRow(int row);
		const WareHoly* GetItemByRow(int row) const;
		void RenderFrame();
		void RenderTexts();
		void RenderInter();
		void RenderButtons();
		void RenderHoly();
		void RenderWithdrawPopup();
		void FrameTarget(float iPos_x, float iPos_y, float width, float height, DWORD color);
		void render_option_group();
		int FindBankIndexByItemType(int nItemType);
	};
}

#endif
