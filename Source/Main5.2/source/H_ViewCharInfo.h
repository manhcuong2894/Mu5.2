#pragma once
#include "Protocol.h"
#include "UIWindows.h"
#if(H_VIEWCHARINFO)
class H_ViewCharPhotoViewer : public CUIPhotoViewer
{
public:
	void SetPreviewAngle(float angle)
	{
		m_fSettingAngle = angle;
		m_fCurrentAngle = angle;
	}

	void ResetPreviewHelperState()
	{
		m_PhotoHelper.Live = false;
		m_PhotoHelper.movementType = Movement::None;
		m_PhotoChar.Object.movementType = Movement::None;
		m_PhotoChar.Helper.Type = -1;
		m_PhotoChar.Helper.Level = 0;
		m_PhotoChar.Helper.Option1 = 0;
		m_PhotoChar.Helper.ExtOption = 0;
	}
};

class H_ViewCharInfo
{

public:
#pragma pack(push, 1)
	struct PMSG_COUNTLIST_VIEWCHAR
	{
		PSWMSG_HEAD header;
		int Count;
		char Name[11];
		int Reset;
		int Level;
		int MasterLevel;
		int TotalPoints;
		char GuildName[9];
		int AccountLevel;
		BYTE Class;
		DWORD Equipment[EQUIPMENT_LENGTH];
	};
	struct GETINFOCHAR_DATA
	{
		BYTE Dur;
		BYTE Item[12];
		int  PeriodTime;
	};
#pragma pack(pop)

	H_ViewCharInfo();
	~H_ViewCharInfo();
	void OpenClose();
	void Close();
	void DrawWindow();
	void SendRequestViewItem(int aIndex);
	void RecvProtocol(BYTE* Recv);
	void UpdateMouseInput();
	bool IsOpen() const;
	bool ShouldBlockEscapeKey() const;

private:


	typedef struct tagEQUIPMENT_ITEM
	{
		int x, y;
		int width, height;
		DWORD dwBgImage;
	} EQUIPMENT_ITEM;
#if(H_EXPANDSLOTITEM)
	EQUIPMENT_ITEM m_ViewCharItemSlots[MAX_NEW_EQUIPMENT];
	ITEM			CharItemInfo[MAX_NEW_EQUIPMENT];
	std::vector<GETINFOCHAR_DATA> m_CharItemInfo;
#else
	EQUIPMENT_ITEM m_ViewCharItemSlots[MAX_EQUIPMENT];
	ITEM			CharItemInfo[MAX_EQUIPMENT];
#endif
	std::string		CharViewName;
	int				ViewReset;
	int				ViewLevel;
	int				ViewMaster;
	int				ViewPoint;
	std::string		ViewGuild;
	std::string		ViewVip;
	BYTE			ViewClass;
	DWORD			ViewPreviewEquipment[EQUIPMENT_LENGTH];
	bool			m_HasPreviewData;
	bool			m_PreviewViewerInitialized;
	bool			m_PreviewDirty;
	float			m_PreviewAngle;
	float			m_LastPreviewMouseX;
	bool			m_IsPreviewRotating;
	bool			m_BlockEscapeKey;
	H_ViewCharPhotoViewer	m_PhotoViewer;
	void ClearData();
	void SetImgSlot(float x, float y);
	void EnsurePreviewViewer();
	void UpdatePreviewViewer();
	bool GetPreviewRect(float& x, float& y, float& width, float& height) const;
	void HandlePreviewMouse(float x, float y, float width, float height);
	void DrawCharacterPreview(float x, float y, float width, float height);
	
};

extern H_ViewCharInfo* gH_ViewCharInfo;
#endif
