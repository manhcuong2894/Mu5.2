///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "GuildCache.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "SMD.h"
#include "Local.h"
#include "MatchEvent.h"
#include "PhysicsManager.h"
#include "./Utilities/Log/ErrorReport.h"
#include "CSQuest.h"
#include "PersonalShopTitleImp.h"
#include "uicontrols.h"
#include "GOBoid.h"
#include "GMHellas.h"
#include "CSItemOption.h"
#include "GMBattleCastle.h"
#include "GMHuntingGround.h"
#include "GMAida.h"

extern void FlushGpuAssistShaderState();
namespace
{
	size_t GetUtf8CharByteLength(const char* pszText)
	{
		if (pszText == NULL || *pszText == '\0')
		{
			return 0;
		}

		const unsigned char byHead = static_cast<unsigned char>(*pszText);

		if ((byHead & 0x80) == 0x00)
		{
			return 1;
		}

		if (pszText[1] != '\0' &&
			(byHead & 0xE0) == 0xC0 &&
			(static_cast<unsigned char>(pszText[1]) & 0xC0) == 0x80)
		{
			return 2;
		}

		if (pszText[1] != '\0' &&
			pszText[2] != '\0' &&
			(byHead & 0xF0) == 0xE0 &&
			(static_cast<unsigned char>(pszText[1]) & 0xC0) == 0x80 &&
			(static_cast<unsigned char>(pszText[2]) & 0xC0) == 0x80)
		{
			return 3;
		}

		if (pszText[1] != '\0' &&
			pszText[2] != '\0' &&
			pszText[3] != '\0' &&
			(byHead & 0xF8) == 0xF0 &&
			(static_cast<unsigned char>(pszText[1]) & 0xC0) == 0x80 &&
			(static_cast<unsigned char>(pszText[2]) & 0xC0) == 0x80 &&
			(static_cast<unsigned char>(pszText[3]) & 0xC0) == 0x80)
		{
			return 4;
		}

		return 1;
	}

	int GetUiCharByteLength(const char* pszText)
	{
		if (pszText == NULL || *pszText == '\0')
		{
			return 0;
		}

		if (g_pMultiLanguage && g_pMultiLanguage->IsCharUTF8(pszText))
		{
			return static_cast<int>(GetUtf8CharByteLength(pszText));
		}

		const int iLength = _mbclen(reinterpret_cast<const unsigned char*>(pszText));
		return (iLength > 0) ? iLength : 1;
	}
}
#include "GMCrywolf1st.h"
#include "npcBreeder.h"
#include "CSPetSystem.h"
#include "GIPetManager.h"
#include "CComGem.h"
#include "UIMapName.h"	// rozy
#include "CustomChoTroi.h"
#include "./Time/Timer.h"
#include "Input.h"
#include "UIMng.h"
#include "LoadingScene.h"
#include "CDirection.h"
#include "CBDrawInterface.h"
#include "GM_Kanturu_3rd.h"
#ifdef MOVIE_DIRECTSHOW
#include <dshow.h>
#include "MovieScene.h"
#endif // MOVIE_DIRECTSHOW
#include "Event.h"
#include "./Utilities/Log/muConsoleDebug.h"
#include "MixMgr.h"
#include "GameCensorship.h"
#include "GM3rdChangeUp.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "PartyManager.h"
#include "w_CursedTemple.h"
#include "CameraMove.h"
#include "w_MapHeaders.h"
#include "w_PetProcess.h"
#include "PortalMgr.h"
#include "ServerListManager.h"
#include "ProtocolSend.h"
#include "MapManager.h"
#include "CGMProtect.h"
#include "CAIController.h"
#include "CameraManager.h"
#include "CGMHeadChat.h"
#include "ConnectVersionHex.h"

extern CUITextInputBox* g_pSingleTextInputBox;
extern CUITextInputBox* g_pSinglePasswdInputBox;
extern int g_iChatInputType;
extern BOOL g_bUseChatListBox;
extern DWORD g_dwMouseUseUIID;
extern DWORD g_dwActiveUIID;
extern DWORD g_dwKeyFocusUIID;
extern CUIMapName* g_pUIMapName;
extern bool HighLight;
extern CTimer* g_pTimer;

#ifdef MOVIE_DIRECTSHOW
extern CMovieScene* g_pMovieScene;
#endif // MOVIE_DIRECTSHOW


bool	g_bTimeCheck = false;
int 	g_iBackupTime = 0;

float	g_fMULogoAlpha = 0;


// extern CGuildCache g_GuildCache;

extern float g_fSpecialHeight;

short   g_shCameraLevel = 0;

int g_iLengthAuthorityCode = 20;

WORD g_ServerPort = 44405;
char* szServerIpAddress = "192.168.0.104";

#ifdef MOVIE_DIRECTSHOW
int  SceneFlag = MOVIE_SCENE;
#else // MOVIE_DIRECTSHOW
int  SceneFlag = WEBZEN_SCENE;
#endif // MOVIE_DIRECTSHOW

int MoveSceneFrame = 0;
int frame_scene_desplace = 0;
extern int g_iKeyPadEnable;


CPhysicsManager g_PhysicsManager;


char* g_lpszMp3[NUM_MUSIC] =
{
	"data\\music\\Pub.mp3",
	"data\\music\\Mutheme.mp3",
	"data\\music\\Church.mp3",
	"data\\music\\Devias.mp3",
	"data\\music\\Noria.mp3",
	"data\\music\\Dungeon.mp3",
	"data\\music\\atlans.mp3",
	"data\\music\\icarus.mp3",
	"data\\music\\tarkan.mp3",
	"data\\music\\lost_tower_a.mp3",
	"data\\music\\lost_tower_b.mp3",
	"data\\music\\kalima.mp3",
	"data\\music\\castle.mp3",
	"data\\music\\charge.mp3",
	"data\\music\\lastend.mp3",
	"data\\music\\huntingground.mp3",
	"data\\music\\Aida.mp3",
	"data\\music\\crywolf1st.mp3",
	"data\\music\\crywolf_ready-02.ogg",
	"data\\music\\crywolf_before-01.ogg",
	"data\\music\\crywolf_back-03.ogg",
	"data\\music\\main_theme.mp3",
	"data\\music\\kanturu_1st.mp3",
	"data\\music\\kanturu_2nd.mp3",
	"data\\music\\KanturuMayaBattle.mp3",
	"data\\music\\KanturuNightmareBattle.mp3",
	"data\\music\\KanturuTower.mp3",
	"data\\music\\BalgasBarrack.mp3",
	"data\\music\\BalgasRefuge.mp3",
	"data\\music\\cursedtemplewait.mp3",
	"data\\music\\cursedtempleplay.mp3",
	"data\\music\\elbeland.mp3",
	"data\\music\\login_theme.mp3",
	"data\\music\\SwampOfCalmness.mp3",
	"data\\music\\Raklion.mp3",
	"data\\music\\Raklion_Hatchery.mp3",
	"data\\music\\Santa_Village.mp3",
	"data\\music\\DuelArena.mp3",
	"data\\music\\PK_Field.mp3",
	"data\\music\\ImperialGuardianFort.mp3",
	"data\\music\\ImperialGuardianFort.mp3",
	"data\\music\\ImperialGuardianFort.mp3",
	"data\\music\\ImperialGuardianFort.mp3",
	"data\\music\\iDoppelganger.mp3",
	"data\\music\\iDoppelganger.mp3",
	"data\\music\\Karutan_A.mp3",
	"data\\music\\Karutan_B.mp3",
};

extern char Mp3FileName[256];

#define MAX_LENGTH_CMB	( 38)

DWORD   g_dwWaitingStartTick;
int     g_iRequestCount;

int     g_iMessageTextStart = 0;
char    g_cMessageTextCurrNum = 0;
char    g_cMessageTextNum = 0;
int     g_iNumLineMessageBoxCustom;
char    g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
int     g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];

int     g_iCustomMessageBoxButton_Cancel[NUM_PAR_BUTTON_CMB];

int		g_iCancelSkillTarget = 0;

#define NUM_LINE_DA		( 1)
int g_iCurrentDialogScript = -1;
int g_iNumAnswer = 0;
char g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];

int    CameraWalkCut;
int    CurrentCameraCount = -1;
int    CurrentCameraWalkType = 0;
int    CurrentCameraNumber = 0;
vec3_t CurrentCameraPosition;
vec3_t CurrentCameraAngle;
float  CurrentCameraWalkDelta[6];
float  CameraWalk[] =
{
	 0.f,-1000.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	200.f,-800.f,250.f,-87.f,0.f, -10.f,
};

bool MenuCancel = true;
bool EnableSocket = false;
bool InitLogIn = false;
bool InitLoading = false;
bool InitCharacterScene = false;
bool InitMainScene = false;
int  MenuY = 480;
int  MenuX = -200;
extern char LogInID[MAX_ID_SIZE + 1];
extern char m_ExeVersion[11];


extern DWORD g_dwBKConv;
extern DWORD g_dwBKSent;
extern BOOL g_bIMEBlock;

bool EnableMainRender = false;
extern int HeroKey;

int MenuStateCurrent = MENU_SERVER_LIST;
int MenuStateNext = MENU_SERVER_LIST;
int DeleteGuildIndex = -1;
int  ErrorMessage = NULL;
int	 ErrorMessageNext = NULL;

int SelectedHero = -1;

int TestTime = 0;
float g_Luminosity;
BYTE byte_E8CB13;
int TimeRemain = 40;

extern steady_clock_ ChatTime;
extern int  GrabScreen;
extern int g_iNoMouseTime;
extern bool g_bEnterPressed;
extern int WaterTextureNumber;

bool MoveMainCamera();
void MoveCharacter(CHARACTER* c, OBJECT* o);
extern GLvoid KillGLWindow(GLvoid);
extern BOOL Util_CheckOption(char* lpszCommandLine, unsigned char cOption, char* lpszString);

void StopMusic()
{
	for (int i = 0; i < NUM_MUSIC; ++i)
	{
		StopMp3(g_lpszMp3[i]);
	}
}

void SetCreateMaxClass(BYTE MaxClass)
{
	byte_E8CB13 = MaxClass;
}

BYTE GetCreateMaxClass()
{
	return byte_E8CB13;
}

bool CheckAbuseFilter(char* Text, bool bCheckSlash)
{
	if (bCheckSlash == true)
	{
		if (Text[0] == '/')
		{
			return false;
		}
	}

	int icntText = 0;
	char TmpText[2048];
	for (int i = 0; i < (int)strlen(Text); ++i)
	{
		if (Text[i] != 32)
		{
			TmpText[icntText] = Text[i];
			icntText++;
		}
	}
	TmpText[icntText] = 0;

	for (int i = 0; i < AbuseFilterNumber; i++)
	{
		if (FindText(TmpText, AbuseFilter[i]))
		{
			return true;
		}
	}
	return false;
}

bool CheckAbuseNameFilter(char* Text)
{
	int icntText = 0;
	char TmpText[256];
	for (int i = 0; i < (int)strlen(Text); ++i)
	{
		if (Text[i] != 32)
		{
			TmpText[icntText] = Text[i];
			icntText++;
		}
	}
	TmpText[icntText] = 0;

	for (int i = 0; i < AbuseNameFilterNumber; i++)
	{
		if (FindText(TmpText, AbuseNameFilter[i]))
		{
			return true;
		}
	}
	return false;
}

bool CheckName()
{
	if (CheckAbuseNameFilter(InputText[0]) || CheckAbuseFilter(InputText[0]) ||
		FindText(InputText[0], " ") || FindText(InputText[0], "¡¡") ||
		FindText(InputText[0], ".") || FindText(InputText[0], "¡¤") || FindText(InputText[0], "¡­") ||
		FindText(InputText[0], "Webzen") || FindText(InputText[0], "WebZen") || FindText(InputText[0], "webzen") || FindText(InputText[0], "WEBZEN") ||
		FindText(InputText[0], GlobalText[457]) || FindText(InputText[0], GlobalText[458]))
		return true;
	return false;
}

#ifdef MOVIE_DIRECTSHOW
void MovieScene(HDC hDC)
{
	if (g_pMovieScene->GetPlayNum() == 0)
	{
		g_pMovieScene->InitOpenGLClear(hDC);

		g_pMovieScene->Initialize_DirectShow(gwinhandle->GethWnd(), MOVIE_FILE_WMV);

		if (g_pMovieScene->IsFile() == FALSE || g_pMovieScene->IsFailDirectShow() == TRUE)
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			SceneFlag = WEBZEN_SCENE;
			return;
		}

		g_pMovieScene->PlayMovie();

		if (g_pMovieScene->IsEndMovie())
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			SceneFlag = WEBZEN_SCENE;
			return;
		}
		else
		{
			if (HIBYTE(GetAsyncKeyState(VK_ESCAPE)) == 128 || HIBYTE(GetAsyncKeyState(VK_RETURN)) == 128)
			{
				g_pMovieScene->Destroy();
				SAFE_DELETE(g_pMovieScene);
				SceneFlag = WEBZEN_SCENE;
				return;
			}
		}
	}
	else
	{
		g_pMovieScene->Destroy();
		SAFE_DELETE(g_pMovieScene);
		SceneFlag = WEBZEN_SCENE;
		return;
	}
}
#endif // MOVIE_DIRECTSHOW

bool InitWebzenScene = false;

void CreateWebzenScene()
{
	CUIMng& rUIMng = CUIMng::Instance();

	OpenFont();
	ClearInput();

	FogEnable = false;

	LoadBitmap("Interface\\New_lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
	LoadBitmap("Interface\\New_lo_back_02.jpg", BITMAP_TITLE + 1, GL_LINEAR);
	LoadBitmap("Interface\\MU_TITLE.tga", BITMAP_TITLE + 2, GL_LINEAR);
	LoadBitmap("Interface\\lo_121518.tga", BITMAP_TITLE + 3, GL_LINEAR);
	LoadBitmap("Interface\\New_lo_webzen_logo.tga", BITMAP_TITLE + 4, GL_LINEAR);
	LoadBitmap("Interface\\lo_back_s5_03.jpg", BITMAP_TITLE + 6, GL_LINEAR);
	LoadBitmap("Interface\\lo_back_s5_04.jpg", BITMAP_TITLE + 7, GL_LINEAR);
	LoadBitmap("Interface\\HUD\\lo_download.jpg", BITMAP_TITLE + 5, GL_LINEAR, GL_REPEAT);

	if (rand() % 100 <= 70)
	{
		LoadBitmap("Interface\\lo_back_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
	}
	else
	{
		LoadBitmap("Interface\\lo_back_s5_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
	}

	rUIMng.CreateTitleSceneUI();


#ifdef SOCKET_LAUNCHER
	GMConnectHex->Initialize();

	GMConnectHex->SetUpdateVersion(true);

	if (CreateSocket(szServerIpAddress, g_ServerPort))
	{
		SendRequestServerHWID();
		SendRequestServerUpdate(GMConnectHex->GetClientVersion());
	}
	else
	{
		GMConnectHex->SetUpdateVersion(false);
	}
#endif // SOCKET_LAUNCHER
}

void WebzenScene(HDC hDC)
{
	if (!InitWebzenScene)
	{
		CreateWebzenScene();
		InitWebzenScene = true;
	}

#ifdef SOCKET_LAUNCHER
	if (GMConnectHex->GetUpdateVersion() == false)
	{
		CUIMng& rUIMng = CUIMng::Instance();

		LoadBitmap("Interface\\lo_lo.jpg", BITMAP_TITLE + 5, GL_LINEAR, GL_REPEAT);

		EnableAlphaTest();

		OpenBasicData(hDC);

		g_pNewUISystem->LoadMainSceneInterface();

		CUIMng::Instance().RenderTitleSceneUI(hDC, 11, 12);

		rUIMng.ReleaseTitleSceneUI();

		DeleteBitmap(BITMAP_TITLE);
		DeleteBitmap(BITMAP_TITLE + 1);
		DeleteBitmap(BITMAP_TITLE + 2);
		DeleteBitmap(BITMAP_TITLE + 3);
		DeleteBitmap(BITMAP_TITLE + 4);
		DeleteBitmap(BITMAP_TITLE + 5);

		for (int i = 6; i < 14; ++i)
			DeleteBitmap(BITMAP_TITLE + i);

		g_ErrorReport.Write("> Loading ok.\r\n");

		SceneFlag = LOG_IN_SCENE;
	}
	else
	{
		GMConnectHex->RenderInterface(hDC);

		//if (SocketClient.GetSocket() == INVALID_SOCKET)
		//{
		//}
	}
#else
	CUIMng& rUIMng = CUIMng::Instance();

	EnableAlphaTest();

	OpenBasicData(hDC);

	g_pNewUISystem->LoadMainSceneInterface();

	CUIMng::Instance().RenderTitleSceneUI(hDC, 11, 12);

	rUIMng.ReleaseTitleSceneUI();
	DeleteBitmap(BITMAP_TITLE);
	DeleteBitmap(BITMAP_TITLE + 1);
	DeleteBitmap(BITMAP_TITLE + 2);
	DeleteBitmap(BITMAP_TITLE + 3);
	DeleteBitmap(BITMAP_TITLE + 4);
	DeleteBitmap(BITMAP_TITLE + 5);

	for (int i = 6; i < 14; ++i)
		DeleteBitmap(BITMAP_TITLE + i);

	g_ErrorReport.Write("> Loading ok.\r\n");

	SceneFlag = LOG_IN_SCENE;
#endif // SOCKET_LAUNCHER
}

void DeleteCharacter()
{
	CHARACTER* pCharacter = gmCharacters->GetCharacter(SelectedHero);

	if (pCharacter)
	{
		if (g_iChatInputType == 1)
		{
			g_pSinglePasswdInputBox->GetText(InputText[0]);
			g_pSinglePasswdInputBox->SetText(NULL);
			g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
		}
		SendRequestDeleteCharacter(pCharacter->ID, InputText[0]);

		MenuStateCurrent = MENU_DELETE_LEFT;
		MenuStateNext = MENU_NEW_DOWN;
		PlayBuffer(SOUND_MENU01);

		ClearInput();
		InputEnable = false;
	}
	SelectedHero = -1;
}

bool IsEnterPressed()
{
	return g_bEnterPressed;
}

void SetEnterPressed(bool enterpressed) {
	g_bEnterPressed = enterpressed;
}

BOOL CheckOptionMouseClick(int iOptionPos_y, BOOL bPlayClickSound = TRUE)
{
	if (CheckMouseIn((gwinhandle->GetScreenX() - 120) / 2, 30 + iOptionPos_y, 120, 22) && MouseLButtonPush)
	{
		MouseLButtonPush = false;
		MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;
		if (bPlayClickSound == TRUE) PlayBuffer(SOUND_CLICK01);
		return TRUE;
	}
	return FALSE;
}

int SeparateTextIntoLines(const char* lpszText, char* lpszSeparated, int iMaxLine, int iLineSize)
{
	int iLine = 0;
	const char* lpLineStart = lpszText;
	char* lpDst = lpszSeparated;
	const char* lpSpace = NULL;
	int iMbclen = 0;
	for (const char* lpSeek = lpszText; *lpSeek; lpSeek += iMbclen, lpDst += iMbclen)
	{
		iMbclen = GetUiCharByteLength(lpSeek);
		if (iMbclen + (int)(lpSeek - lpLineStart) >= iLineSize)
		{
			if (lpSpace && (int)(lpSeek - lpSpace) < min(10, iLineSize / 2))
			{
				lpDst -= (lpSeek - lpSpace - 1);
				lpSeek = lpSpace + 1;
			}

			lpLineStart = lpSeek;
			*lpDst = '\0';
			if (iLine >= iMaxLine - 1)
			{
				break;
			}
			++iLine;
			lpDst = lpszSeparated + iLine * iLineSize;
			lpSpace = NULL;
		}

		memcpy(lpDst, lpSeek, iMbclen);
		if (*lpSeek == ' ')
		{
			lpSpace = lpSeek;
		}
	}
	*lpDst = '\0';

	return (iLine + 1);
}

void SetEffectVolumeLevel(int level)
{
	if (level > 9)
		level = 9;
	if (level < 0)
		level = 0;

	if (level == 0)
	{
		SetMasterVolume(-10000);
	}
	else
	{
		long vol = -2000 * log10(10.f / float(level));
		SetMasterVolume(vol);
	}
}

void SetViewPortLevel(int Wheel)
{
	if ((HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128))
	{
		if (Wheel > 0)
			g_shCameraLevel--;
		else if (Wheel < 0)
			g_shCameraLevel++;

		MouseWheel = 0;

		if (g_shCameraLevel > 4)
			g_shCameraLevel = 4;
		if (g_shCameraLevel < 0)
			g_shCameraLevel = 0;
	}
}

void RenderInfomation3D()
{
	bool Success = false;

	if (((ErrorMessage == MESSAGE_TRADE_CHECK || ErrorMessage == MESSAGE_CHECK) && AskYesOrNo == 1)
		|| ErrorMessage == MESSAGE_USE_STATE
		|| ErrorMessage == MESSAGE_USE_STATE2)
	{
		Success = true;
	}

	if (ErrorMessage == MESSAGE_TRADE_CHECK && AskYesOrNo == 5)
	{
		Success = true;
	}
	if (ErrorMessage == MESSAGE_PERSONALSHOP_WARNING)
	{
		Success = true;
	}

	if (Success)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glViewport2(0, 0, WindowWidth, WindowHeight);
		gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), CameraViewNear, CameraViewFar);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		GetOpenGLMatrix(CameraMatrix);
		EnableDepthTest();
		EnableDepthMask();

		float Width, Height;
		float x = (gwinhandle->GetScreenX() - 150) / 2;
		float y;
		if (ErrorMessage == MESSAGE_TRADE_CHECK)
		{
			y = 60 + 55;
		}
		else
		{
			y = 60 + 55;
		}

		Width = 40.f; Height = 60.f;
		int iRenderType = ErrorMessage;
		if (AskYesOrNo == 5)
			iRenderType = MESSAGE_USE_STATE;
		switch (iRenderType)
		{
		case MESSAGE_USE_STATE:
		case MESSAGE_USE_STATE2:
		case MESSAGE_PERSONALSHOP_WARNING:
			RenderItem3D(x, y, Width, Height, TargetItem.Type, TargetItem.Level, TargetItem.Option1, TargetItem.ExtOption, true);
			break;

		default:
			RenderItem3D(x, y, Width, Height, PickItem.Type, PickItem.Level, PickItem.Option1, PickItem.ExtOption, true);
			break;
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		UpdateMousePositionn();
	}
}

void RenderInfomation()
{
	CUIMng::Instance().Render();

	RenderTopMostOverlayWindows();

	if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
	{
		RenderCursor();
	}

	RenderInfomation3D();
}

BOOL ShowCheckBox(int num, int index, int message)
{
	if (message == MESSAGE_USE_STATE || message == MESSAGE_USE_STATE2)
	{
		char Name[50] = { 0, };
		if (TargetItem.Type == ITEM_HELPER + 15)
		{
			switch ((TargetItem.Level >> 3) & 15)
			{
			case 0:sprintf(Name, "%s", GlobalText[168]); break;
			case 1:sprintf(Name, "%s", GlobalText[169]); break;
			case 2:sprintf(Name, "%s", GlobalText[167]); break;
			case 3:sprintf(Name, "%s", GlobalText[166]); break;
			case 4:sprintf(Name, "%s", GlobalText[1900]); break;
			}
		}

		if (message == MESSAGE_USE_STATE2)
			sprintf(g_lpszMessageBoxCustom[0], "( %s%s )", Name, GlobalText[1901]);
		else
			sprintf(g_lpszMessageBoxCustom[0], "( %s )", Name);

		num++;
		for (int i = 1; i < num; ++i)
		{
			sprintf(g_lpszMessageBoxCustom[i], GlobalText[index]);
		}
		g_iNumLineMessageBoxCustom = num;
	}
	else if (message == MESSAGE_PERSONALSHOP_WARNING)
	{
		char szGold[256];
		ConvertGold(InputGold, szGold);
		sprintf(g_lpszMessageBoxCustom[0], GlobalText[index], szGold);

		for (int i = 1; i < num; ++i)
		{
			sprintf(g_lpszMessageBoxCustom[i], GlobalText[index + i]);
		}
		g_iNumLineMessageBoxCustom = num;
	}
	else if (message == MESSAGE_CHAOS_CASTLE_CHECK)
	{
		g_iNumLineMessageBoxCustom = 0;
		for (int i = 0; i < num; ++i)
		{
			g_iNumLineMessageBoxCustom += SeparateTextIntoLines(GlobalText[index + i], g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], NUM_LINE_CMB, MAX_LENGTH_CMB);
		}
	}
	else if (message == MESSAGE_GEM_INTEGRATION3)
	{
		char tBuf[MAX_GLOBAL_TEXT_STRING];
		char tLines[2][30];
		for (int t = 0; t < 2; ++t) memset(tLines[t], 0, 20);
		g_iNumLineMessageBoxCustom = 0;
		if (COMGEM::isComMode())
		{
			if (COMGEM::m_cGemType == 0) sprintf(tBuf, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
			else sprintf(tBuf, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);

			g_iNumLineMessageBoxCustom += SeparateTextIntoLines(tBuf,
				tLines[g_iNumLineMessageBoxCustom], 2, 30);

			for (int t = 0; t < 2; ++t) strcpy(g_lpszMessageBoxCustom[t], tLines[t]);

			sprintf(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], GlobalText[1810], COMGEM::m_iValue);
			++g_iNumLineMessageBoxCustom;

		}
		else
		{
			int t_GemLevel = COMGEM::GetUnMixGemLevel() + 1;
			if (COMGEM::m_cGemType == 0) sprintf(tBuf, GlobalText[1813], GlobalText[1806], t_GemLevel);
			else sprintf(tBuf, GlobalText[1813], GlobalText[1807], t_GemLevel);

			g_iNumLineMessageBoxCustom += SeparateTextIntoLines(tBuf,
				tLines[g_iNumLineMessageBoxCustom], 2, 30);

			for (int t = 0; t < 2; ++t) strcpy(g_lpszMessageBoxCustom[t], tLines[t]);

			sprintf(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], GlobalText[1814], COMGEM::m_iValue);
			++g_iNumLineMessageBoxCustom;
		}
	}
	else if (message == MESSAGE_CANCEL_SKILL)
	{
		char tBuf[MAX_GLOBAL_TEXT_STRING];
		sprintf(tBuf, "%s%s", SkillAttribute[index].Name, GlobalText[2046]);
		g_iNumLineMessageBoxCustom = SeparateTextIntoLines(tBuf, g_lpszMessageBoxCustom[0], 2, MAX_LENGTH_CMB);
		g_iCancelSkillTarget = index;
	}
	else
	{
		for (int i = 0; i < num; ++i)
		{
			strcpy(g_lpszMessageBoxCustom[i], GlobalText[index + i]);
		}
		g_iNumLineMessageBoxCustom = num;
	}

	ZeroMemory(g_iCustomMessageBoxButton, NUM_BUTTON_CMB * NUM_PAR_BUTTON_CMB * sizeof(int));

	int iOkButton[5] = { 1,  21, 90, 70, 21 };
	int iCancelButton[5] = { 3, 120, 90, 70, 21 };

	if (message == MESSAGE_USE_STATE2)
	{
		iOkButton[1] = 22;
		iOkButton[2] = 92;	// y
		iOkButton[3] = 49;
		iOkButton[4] = 16;

		iCancelButton[1] = 82;
		iCancelButton[2] = 92;	// y
		iCancelButton[3] = 49;
		iCancelButton[4] = 16;

		g_iCustomMessageBoxButton_Cancel[0] = 5;
		g_iCustomMessageBoxButton_Cancel[1] = 142;	// x
		g_iCustomMessageBoxButton_Cancel[2] = 92;	// y
		g_iCustomMessageBoxButton_Cancel[3] = 49;	// width
		g_iCustomMessageBoxButton_Cancel[4] = 16;	// height
	}

	if (message == MESSAGE_CHAOS_CASTLE_CHECK)
	{
		iOkButton[2] = 120;
		iCancelButton[2] = 120;
	}

	memcpy(g_iCustomMessageBoxButton[0], iOkButton, 5 * sizeof(int));
	memcpy(g_iCustomMessageBoxButton[1], iCancelButton, 5 * sizeof(int));

	return true;
}

void MoveCharacterCamera(vec3_t Origin, vec3_t Position, vec3_t Angle)
{
	vec3_t TransformPosition;
	CameraAngle[0] = 0.f;
	CameraAngle[1] = 0.f;
	CameraAngle[2] = Angle[2];
	float Matrix[3][4];
	AngleMatrix(CameraAngle, Matrix);
	VectorIRotate(Position, Matrix, TransformPosition);
	VectorAdd(Origin, TransformPosition, CameraPosition);
	CameraAngle[0] = Angle[0];
}

void MoveCamera()
{
	if (CCameraMove::GetInstancePtr()->IsTourMode())
	{
		return;
	}

	if (CurrentCameraCount == -1)
	{
		for (int i = 0; i < 3; i++)
		{
			CurrentCameraPosition[i] = CameraWalk[i];
			CurrentCameraAngle[i] = CameraWalk[i + 3];
		}
		CurrentCameraNumber = 1;
		CurrentCameraWalkType = 1;

		for (int i = 0; i < 3; i++)
		{
			CurrentCameraWalkDelta[i] = (CameraWalk[CurrentCameraNumber * 6 + i] - CurrentCameraPosition[i]) / 128;
			CurrentCameraWalkDelta[i + 3] = (CameraWalk[CurrentCameraNumber * 6 + i + 3] - CurrentCameraAngle[i]) / 128;
		}
	}
	CurrentCameraCount++;
	if ((CameraWalkCut == 0 && CurrentCameraCount >= 40) || (CameraWalkCut > 0 && CurrentCameraCount >= 128))
	{
		CurrentCameraCount = 0;
		if (CameraWalkCut == 0)
		{
			CameraWalkCut = 1;
		}
		else
		{
			if (SceneFlag == LOG_IN_SCENE)
			{
				CurrentCameraNumber = rand() % 4 + 1;
				CurrentCameraWalkType = rand() % 2;
			}
			else
			{
				CurrentCameraNumber = 5;
				CurrentCameraWalkType = 0;
			}
		}
		for (int i = 0; i < 3; i++)
		{
			CurrentCameraWalkDelta[i] = (CameraWalk[CurrentCameraNumber * 6 + i] - CurrentCameraPosition[i]) / 128;
			CurrentCameraWalkDelta[i + 3] = (CameraWalk[CurrentCameraNumber * 6 + i + 3] - CurrentCameraAngle[i]) / 128;
		}
	}
	if (CurrentCameraWalkType == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			CurrentCameraPosition[i] += (CameraWalk[CurrentCameraNumber * 6 + i] - CurrentCameraPosition[i]) / 6;
			CurrentCameraAngle[i] += (CameraWalk[CurrentCameraNumber * 6 + i + 3] - CurrentCameraAngle[i]) / 6;
		}
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			CurrentCameraPosition[i] += CurrentCameraWalkDelta[i];
		}
	}
	CameraFOV = 45.f;
	vec3_t Position;
	Vector(0.f, 0.f, 0.f, Position);
	MoveCharacterCamera(Position, CurrentCameraPosition, CurrentCameraAngle);
}

void StartGame()
{
	CHARACTER* pCharacter = gmCharacters->GetCharacter(SelectedHero);

	if (pCharacter)
	{
		if (CTLCODE_01BLOCKCHAR & pCharacter->CtlCode)
		{
			CUIMng::Instance().PopUpMsgWin(MESSAGE_BLOCKED_CHARACTER);
		}
		else
		{
			CharacterAttribute->Level = pCharacter->Level;
			CharacterAttribute->Class = pCharacter->Class;
			CharacterAttribute->Skin = pCharacter->Skin;

			::strcpy(CharacterAttribute->Name, pCharacter->ID);

			::ReleaseCharacterSceneData();
			InitLoading = false;
			SceneFlag = LOADING_SCENE;
		}
	}
}

CHARACTER* CreateHeroLogIn(int Index, int Class, int Skin, float fScale, vec3_t Angle, vec3_t Position)
{
	CHARACTER* pCha = CreateHero(Index, Class, Skin);

	if (pCha)
	{
		OBJECT* o = &pCha->Object;

		o->Scale = fScale;
		VectorCopy(Angle, o->Angle);
		VectorCopy(Position, o->Position);
	}

	return pCha;
}

void CharacterTeleport(int key)
{
	CHARACTER* pCharacter;
	for (int i = 0; i < 5; i++)
	{
		pCharacter = gmCharacters->GetCharacter(i);

		OBJECT* pObject = &pCharacter->Object;

		if (pObject->Live)
		{
			if (key == i)
			{
				pObject->Position[0] = 13629.f;
				pObject->Position[1] = 12339.f;
				pObject->Angle[2] = 90.f;
			}
			else
			{
				pObject->Position[0] = 8046.f;
				pObject->Position[1] = 19400.f;
				pObject->Angle[2] = 90.f;
			}
		}
	}
}

void RenderInterfaceEdge()
{
	float Width, Height;
	float WindowX, WindowY;

	EnableAlphaTest();
	glColor3f(1.f, 1.f, 1.f);
	//interface edge
	WindowX = 320.0; WindowY = 0.0; Width = 256.0; Height = 86.0;
	RenderNoBitmap(BITMAP_LOG_IN + 8, WindowX, WindowY, Width, Height, 0.f, 0.f, Width / 256.f, Height / 128.f);
	WindowX = 64.0; WindowY = 0.0; Width = 256.0; Height = 86.0;
	RenderNoBitmap(BITMAP_LOG_IN + 8, WindowX, WindowY, Width, Height, Width / 256.f, 0.f, -Width / 256.f, Height / 128.f);

	WindowX = 576.0; WindowY = 0.0; Width = 64.0; Height = 256.0;
	RenderNoBitmap(BITMAP_LOG_IN + 9, WindowX, WindowY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 256.f);
	WindowX = 0.0; WindowY = 0.0; Width = 64.0; Height = 256;
	RenderNoBitmap(BITMAP_LOG_IN + 9, WindowX, WindowY, Width, Height, Width / 64.f, 0.f, -Width / 64.f, Height / 256.f);

	WindowX = 576.0; WindowY = 256.0; Width = 64.0; Height = 224.0;
	RenderNoBitmap(BITMAP_LOG_IN + 10, WindowX, WindowY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 256.f);
	WindowX = 0.0; WindowY = 256.0; Width = 64.0; Height = 224.0;
	RenderNoBitmap(BITMAP_LOG_IN + 10, WindowX, WindowY, Width, Height, Width / 64.f, 0.f, -Width / 64.f, Height / 256.f);

	WindowX = 320.0; WindowY = 448.0; Width = 256.0; Height = 32.0;
	RenderNoBitmap(BITMAP_LOG_IN + 14, WindowX, WindowY, Width, Height, 0.f, 0.f, Width / 256.f, Height / 32.f);
	WindowX = 64.0; WindowY = 448.0; Width = 256.0; Height = 32.0;
	RenderNoBitmap(BITMAP_LOG_IN + 14, WindowX, WindowY, Width, Height, Width / 256.f, 0.f, -Width / 256.f, Height / 32.f);
}

void CreateCharacterScene()
{
	g_pNewUIMng->ResetActiveUIObj();

	EnableMainRender = true;
	MouseOnWindow = false;
	ErrorMessage = NULL;

	if (gmProtect->SceneCharacter == 1)
	{
		vec3_t Angle, Position;

		World = -1;
		CurrentCameraPosition[0] = -80.0;
		CurrentCameraPosition[1] = 0.0;
		CurrentCameraPosition[2] = -10.0;
		CurrentCameraWalkType = 0;
		CurrentCameraNumber = 5;
		CurrentCameraCount = 0;
		CurrentCameraAngle[0] = 200.0;
		CurrentCameraAngle[1] = -800.0;
		CurrentCameraAngle[2] = 300.0;

		OpenCharacterSceneData();

		Vector(0.0, 0.0, 0.0, Angle);
		Vector(0.0, 0.0, 0.0, Position);
		CreateObject(MODEL_CARD, Position, Angle, 1.1f);
	}
	else
	{
		if (gmProtect->SceneCharacter == 2)
		{
			World = WD_54CHARACTERSCENE;
		}
		else if (gmProtect->SceneCharacter == 3)
		{
			World = WD_78NEW_CHARACTER_SCENE;
		}
		else if (gmProtect->SceneCharacter == 4)
		{
			World = WD_93NEW_CHARACTER_SCENE;
		}
		else
		{
			World = WD_74NEW_CHARACTER_SCENE;
		}

		gMapManager->LoadWorld(World);

		OpenCharacterSceneData();
	}

	CreateCharacterPointer(&CharacterView, MODEL_FACE + 1, 0, 0);
	CharacterView.Class = 1;
	CharacterView.Object.Kind = 0;

	SelectedHero = -1;
	CUIMng::Instance().CreateCharacterScene();

	ClearInventory();
	CharacterAttribute->SkillNumber = 0;

	for (int i = 0; i < MAX_MAGIC; i++)
		CharacterAttribute->Skill[i] = 0;

	for (int i = EQUIPMENT_WEAPON_RIGHT; i < EQUIPMENT_HELPER; i++)
		CharacterMachine->Equipment[i].Level = 0;

	g_pNewUISystem->HideAll();

	g_iKeyPadEnable = 0;
	GuildInputEnable = false;
	TabInputEnable = false;
	GoldInputEnable = false;
	InputEnable = true;
	ClearInput();
	InputIndex = 0;
	InputTextWidth = 90;
	InputNumber = 1;

	for (int i = 0; i < MAX_WHISPER; i++)
	{
		g_pChatListBox->AddText("", "", SEASON3B::TYPE_WHISPER_MESSAGE);
	}

	HIMC hIMC = ImmGetContext(gwinhandle->GethWnd());
	DWORD Conversion, Sentence;

	Conversion = IME_CMODE_NATIVE;
	Sentence = IME_SMODE_NONE;

	g_bIMEBlock = FALSE;
	RestoreIMEStatus();
	ImmSetConversionStatus(hIMC, Conversion, Sentence);
	ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
	SaveIMEStatus();
	ImmReleaseContext(gwinhandle->GethWnd(), hIMC);
	g_bIMEBlock = TRUE;

	g_ErrorReport.Write("> Character scene init success.\r\n");
}

void NewMoveCharacterScene()
{
	if (CurrentProtocolState >= RECEIVE_CHARACTERS_LIST)
	{
		if (!InitCharacterScene)
		{
			InitCharacterScene = true;
			CreateCharacterScene();
		}

		if (gmProtect->SceneCharacter == 1)
		{
			MoveObjects();
			gGoboidManager->MoveBugs();

			MoveCharactersClient();
			MoveCharacterClient(&CharacterView);
			MoveEffects();
			MoveJoints();
			MoveParticles();
		}
		else
		{
			InitTerrainLight();
			MoveObjects();
			gGoboidManager->MoveBugs();

			MoveCharactersClient();
			MoveCharacterClient(&CharacterView);

			MoveEffects();
			MoveJoints();
			MoveParticles();
			MoveBoids();
			ThePetProcess().UpdatePets();
		}

		MoveCamera();

		CInput& rInput = CInput::Instance();
		CUIMng& rUIMng = CUIMng::Instance();

		if (rInput.IsKeyDown(VK_RETURN))
		{
			if (!rUIMng.IsInterfaceCharacter() && SelectedHero > -1 && SelectedHero < 5)
			{
				PlayBuffer(SOUND_CLICK01);
				::StartGame();
			}
		}
		else if (rInput.IsKeyDown(VK_ESCAPE))
		{
			if (!rUIMng.IsInterfaceCharacter() && rUIMng.IsSysMenuWinShow())
			{
				::PlayBuffer(SOUND_CLICK01);
				rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
			}
		}

		if (rUIMng.IsCursorOnUI())
		{
			return;
		}

		if (rInput.IsLBtnDbl() && rUIMng.m_CharSelMainWin.IsShow())
		{
			if (SelectedCharacter < 0 || SelectedCharacter > 4)
			{
				return;
			}

			SelectedHero = SelectedCharacter;
			::StartGame();
		}
		else if (rInput.IsLBtnDn())
		{
			if (SelectedCharacter < 0 || SelectedCharacter > 4)
				SelectedHero = -1;
			else
				SelectedHero = SelectedCharacter;

			rUIMng.m_CharSelMainWin.UpdateDisplay();
		}
	}
}

bool NewRenderCharacterScene(HDC hDC)
{
	if (!InitCharacterScene)
	{
		return false;
	}
	if (CurrentProtocolState < RECEIVE_CHARACTERS_LIST)
	{
		return false;
	}

	FogEnable = false;

	if (gmProtect->SceneCharacter == 1)
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
		BeginOpengl(0, 0, 640, 480);
		CreateScreenVector(MouseX, MouseY, MouseTarget, true);

		CHARACTER* pCharacter = NULL;

		for (int i = 0; i < 5; i++)
		{
			pCharacter = gmCharacters->GetCharacter(i);
			Vector(0.0f, 0.0f, 0.0f, pCharacter->Object.Light);
		}

		pCharacter = gmCharacters->GetCharacter(SelectedHero);

		if (pCharacter)
		{
			OBJECT* pObject = &pCharacter->Object;

			if (pObject->Live)
			{
				Vector(1.0f, 1.0f, 1.0f, pObject->Light);
			}
		}

		RenderObjects();
		RenderCharactersClient();

		if (!CUIMng::Instance().IsCursorOnUI())
			SelectObjects();

		gGoboidManager->RenderBugs();
		RenderBlurs();
		RenderJoints();
		RenderEffects();
		CheckSprites();

		pCharacter = gmCharacters->GetCharacter(SelectedHero);

		if (pCharacter)
		{
			OBJECT* pObject = &pCharacter->Object;

			if (pObject->Live)
			{
				float Rotation = (int)WorldTime % 3600 / (float)10.f;

				EnableAlphaBlend();
				RenderCircle(BITMAP_MAGIC + 2, pObject->Position, 50.0, 70.0, 200.0, Rotation, 0.0, 0.0);
				RenderCircle(BITMAP_MAGIC + 2, pObject->Position, 50.0, 70.0, 200.0, Rotation, 0.0, 0.0);
				DisableAlphaBlend();
				g_csMapServer.SetHeroID(pCharacter->ID);
			}
		}

		BeginSprite();
		RenderSprites();
		RenderParticles();
		EndSprite();

		BeginBitmap();
		RenderInterfaceEdge();
		RenderInfomation();
		EndBitmap();
		EndOpengl();
	}
	else
	{
		vec3_t pos;

		if (gmProtect->SceneCharacter != 0)
		{
			gCameraManager->GetPosition(pos);
		}
		else
		{
			Vector(9758.0f, 18913.0f, 675.0f, pos);
		}

		MoveMainCamera();

		int Width, Height;

		glColor3f(1.f, 1.f, 1.f);
		if (gmProtect->SceneCharacter == 2 || gmProtect->SceneCharacter == 3)
		{
			BeginBitmap();
			Width = GetNoWideX(320.0);
			Height = GetNoWideY(320.0);
			SEASON3B::RenderImageF(BITMAP_LOG_IN + 9, 0, GetNoWideY(25.0), Width, Height, 0.f, 25.0, 511.75, 487.0);
			SEASON3B::RenderImageF(BITMAP_LOG_IN + 10, Width, GetNoWideY(25.0), Width, Height, 0.25, 25.0, 512.0, 487.0);
			EndBitmap();
		}

		Height = GetWindowsY;
		Width = GetScreenWidth();

		glClearColor(0.f, 0.f, 0.f, 1.f);
		BeginOpengl(0, 25, 640, 430);

		CreateFrustrum((float)Width / (float)GetWindowsX, pos);

		CreateScreenVector(MouseX, MouseY, MouseTarget);

		OBJECT* pObj = NULL;
		CHARACTER* pCharacter = NULL;

		for (int i = 0; i < 5; i++)
		{
			pCharacter = gmCharacters->GetCharacter(i);

			if (pCharacter != NULL)
			{
				Vector(0.0f, 0.0f, 0.0f, pCharacter->Object.Light);

				if (gmProtect->SceneCharacter == 4)
				{
					pObj = &pCharacter->Object;

					if (pObj->Live)
					{
						if (SelectedCharacter == i && SelectedHero == i)
						{
							if (pObj->Position[0] == 13629.f && pObj->Position[1] == 12339.f)
							{
								pObj->Angle[2] -= timefac(3);
							}
						}
						else
						{
							if (pObj->Position[0] == 13629.f && pObj->Position[1] == 12339.f && pObj->Angle[2] != 90.f)
							{
								pObj->Angle[2] = 90.f;
							}
						}
					}
				}
			}
		}

		pCharacter = gmCharacters->GetCharacter(SelectedHero);

		if (pCharacter)
		{
			OBJECT* o = &pCharacter->Object;

			if (o->Live)
			{
				EnableAlphaBlend();
				Vector(1.0f, 1.0f, 1.0f, o->Light);
				AddTerrainLight(o->Position[0], o->Position[1], o->Light, 1, PrimaryTerrainLight);
				DisableAlphaBlend();
			}
		}

		CHARACTER* pCha = NULL;

		for (int i = 0; i < 5; ++i)
		{
			pCha = gmCharacters->GetCharacter(i);

			if (pCha)
			{
				pObj = &pCha->Object;

				if (gmProtect->SceneCharacter == 3)
				{
					if (pCha->Helper.Type == MODEL_HELPER + 3)
						pObj->Position[2] = 55.0f;
					else
						pObj->Position[2] = 30.0f;
				}
				else
				{
					if (pCha->Helper.Type == MODEL_HELPER + 3)
						pObj->Position[2] = 194.5f;
					else
						pObj->Position[2] = 169.5f;
				}
			}
		}

		RenderTerrain(false);
		RenderObjects();
		RenderCharactersClient();

		if (!CUIMng::Instance().IsCursorOnUI())
			SelectObjects();

		gGoboidManager->RenderBugs();
		RenderBlurs();
		RenderJoints();
		RenderEffects();
		ThePetProcess().RenderPets();
		RenderBoids();
		RenderObjects_AfterCharacter();
		CheckSprites();

		pCharacter = gmCharacters->GetCharacter(SelectedHero);

		if (pCharacter)
		{
			OBJECT* o = &pCharacter->Object;

			if (o->Live)
			{
				vec3_t vLight;

				Vector(1.0f, 1.0f, 1.f, vLight);
				float fLumi = sinf(WorldTime * 0.0015f) * 0.3f + 0.5f;
				Vector(fLumi * vLight[0], fLumi * vLight[1], fLumi * vLight[2], vLight);

				EnableAlphaBlend();
				if (gmProtect->SceneCharacter == 3)
				{
					RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.5f, 1.5f, vLight, WorldTime * 0.01f);
					RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.f, 1.f, vLight, -WorldTime * 0.01f);
				}
				else
				{
					RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.8f, 1.8f, vLight, WorldTime * 0.01f);
					RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.2f, 1.2f, vLight, -WorldTime * 0.01f);
				}
				DisableAlphaBlend();

				float Rotation = (int)WorldTime % 3600 / (float)10.f;
				Vector(0.15f, 0.15f, 0.15f, o->Light);
				CreateParticleSync(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 4);
				CreateParticleSync(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 5);
				g_csMapServer.SetHeroID(pCharacter->ID);
			}
		}

		BeginSprite();
		RenderSprites();
		RenderParticles();
		RenderPoints();
		EndSprite();

		BeginBitmap();
		RenderInfomation();
#ifdef ENABLE_EDIT
		RenderDebugWindow();
#endif //ENABLE_EDIT
		EndBitmap();
		EndOpengl();
	}

	return true;
}

void CreateLogInScene()
{
	EnableMainRender = true;

	if (gmProtect->SceneLogin == 1)
	{
		vec3_t Angle, Position;

		World = -1;

		OpenLogoSceneData();

		Vector(0.0, 0.0, 0.0, Angle);
		Vector(0.0, -520.0, 0.0, Position);
		CreateObject(MODEL_LOGO, Position, Angle, 1.0);

		Vector(0.0, 0.0, 180.0, Angle);
		Vector(-700.0, 700.0, 0.0, Position);

		CreateObject(MODEL_SHIP, Position, Angle, 1.0);
		CreateObject(MODEL_WAVEBYSHIP, Position, Angle, 1.0);

		Vector(400.0, 400.0, 0.0, Position);
		CreateObject(MODEL_SHIP, Position, Angle, 1.0);
		CreateObject(MODEL_WAVEBYSHIP, Position, Angle, 1.0);

		Vector(-200.0, -400.0, 0.0, Position);
		CreateObject(MODEL_SHIP, Position, Angle, 1.0);
		CreateObject(MODEL_WAVEBYSHIP, Position, Angle, 1.0);

		Vector(0.0, 0.0, 0.0, Angle);
		Vector(-110.0, 1600.0, 50.0, Position);
		CreateObject(MODEL_LOGOSUN, Position, Angle, 1.0);

		Vector(0.0, -600.0, 500.0, Position);
		CreateObject(MODEL_MUGAME, Position, Angle, 1.0);


		CHARACTER* pCharacter;
		Vector(0.0, 0.0, 180.0, Angle);

		Vector(-200.0, -750.0, 250.0, Position);
		CreateHeroLogIn(1, Dark_Knight, 0, 0.6, Angle, Position);

		Vector(-240.0, -770.0, 250.0, Position);
		CreateHeroLogIn(2, Dark_Wizard, 0, 0.6, Angle, Position);

		Vector(400.0, 50.0, 250.0, Position);
		pCharacter = CreateHeroLogIn(3, Fairy_Elf, 0, 0.6, Angle, Position);

		if (pCharacter != NULL)
		{
			gGoboidManager->CreateBug(MODEL_HELPER, Position, &pCharacter->Object);
		}
		CurrentCameraCount = -1;
		Vector(0.0, 0.0, 0.0, CameraAngle);
		Vector(0.0, 0.0, 0.0, CameraPosition);
	}
	else
	{
		if (gmProtect->SceneLogin == 2)
		{
			World = WD_55LOGINSCENE;
		}
		else if (gmProtect->SceneLogin == 3)
		{
			World = WD_77NEW_LOGIN_SCENE;
		}
		else if (gmProtect->SceneLogin == 4)
		{
			World = WD_94NEW_LOGIN_SCENE;
		}
		else
		{
			World = WD_73NEW_LOGIN_SCENE;
		}

		gMapManager->LoadWorld(World);

		OpenLogoSceneData();
	}

	CUIMng::Instance().CreateLoginScene();

	CurrentProtocolState = REQUEST_JOIN_SERVER;

	if (g_pReconnectUI->ReconnectCreateConnection(szServerIpAddress, g_ServerPort))
	{
		SendRequestServerHWID();
	}

	EnableSocket = true;
	GuildInputEnable = false;
	TabInputEnable = false;
	GoldInputEnable = false;
	InputEnable = true;
	ClearInput();

	if (g_iChatInputType == 0)
	{
		strcpy(InputText[0], m_ID[5]);
		InputLength[0] = strlen(InputText[0]);
		InputTextMax[0] = MAX_ID_SIZE;

		if (InputLength[0] == 0)
			InputIndex = 0;
		else
			InputIndex = 1;
	}
	InputNumber = 2;
	InputTextHide[1] = 1;

	if (gmProtect->SceneLogin != 1)
	{
		if (gmProtect->SceneLogin != 4)
		{
			CCameraMove::GetInstancePtr()->PlayCameraWalk(Hero->Object.Position, 1000);
			CCameraMove::GetInstancePtr()->SetTourMode(TRUE, FALSE, (gmProtect->SceneLogin != 3));
		}
		MoveMainCamera();
	}

	g_fMULogoAlpha = 0.0;

	PlayMp3(g_lpszMp3[MUSIC_LOGIN_THEME]);

	g_ErrorReport.Write("> Login Scene init success.\r\n");
}

void NewMoveLogInScene()
{
	if (!InitLogIn)
	{
		InitLogIn = true;
		CreateLogInScene();
	}

	if (gmProtect->SceneLogin == 1)
	{
		if (!CUIMng::Instance().m_CreditWin.IsShow())
		{
			MoveObjects();
			gGoboidManager->MoveBugs();

			MoveCharactersClient();
			MoveParticles();
			MoveCamera();

			if (CameraWalkCut == 1 && !CurrentCameraCount)
				PlayBuffer(SOUND_TITLE01);
		}
	}
	else
	{
		if (!CUIMng::Instance().m_CreditWin.IsShow())
		{
			InitTerrainLight();
			MoveObjects();
			gGoboidManager->MoveBugs();

			MoveLeaves();
			MoveCharactersClient();
			MoveEffects();
			MoveJoints();
			MoveParticles();
			MoveBoids();
			ThePetProcess().UpdatePets();
			MoveCamera();
		}
	}

	if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		CUIMng* rUIMng = &CUIMng::Instance();

		if (rUIMng->IsInterfaceLogin())
		{
			PlayBuffer(SOUND_CLICK01);
			rUIMng->ShowWin(&(rUIMng->m_SysMenuWin));
		}
	}

	if (RECEIVE_LOG_IN_SUCCESS == CurrentProtocolState)
	{
		g_ErrorReport.Write("> Request Character list\r\n");
		CCameraMove::GetInstancePtr()->SetTourMode(FALSE);
		SceneFlag = CHARACTER_SCENE;
		SendRequestCharactersList(g_pMultiLanguage->GetLanguage());
		ReleaseLogoSceneData();
		ClearCharacters();
	}
	g_ConsoleDebug->UpdateMainScene();
}

bool NewRenderLogInScene(HDC hDC)
{
	if (!InitLogIn)
		return false;

	FogEnable = false;

	if (gmProtect->SceneLogin == 1)
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
		BeginOpengl(0, 80, 640, 320);

		if (!CUIMng::Instance().m_CreditWin.IsShow())
		{
			RenderCharactersClient();
			gGoboidManager->RenderBugs();
			RenderObjects();
		}
		CheckSprites();
		BeginSprite();
		RenderSprites();
		RenderParticles();
		EndSprite();
		BeginBitmap();

		//g_fMULogoAlpha += 0.02f;
		//if (g_fMULogoAlpha > 10.0f)
		//	g_fMULogoAlpha = 10.0f;
		//EnableAlphaBlend();
		//glColor4f(g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f);
		//RenderBitmap(BITMAP_LOG_IN + 17, AddMiddleX() + 320.0f - 128.0f * 0.8f, 25.0f, 256.0f * 0.8f, 128.0f * 0.8f);
		//EnableAlphaTest();
		//glColor4f(g_fMULogoAlpha, g_fMULogoAlpha, g_fMULogoAlpha, g_fMULogoAlpha);
		//RenderBitmap(BITMAP_LOG_IN + 16, AddMiddleX() + 320.0f - 128.0f * 0.8f, 25.0f, 256.0f * 0.8f, 128.0f * 0.8f);
	}
	else
	{
		vec3_t pos;
		float Width, Height;

		if (CCameraMove::GetInstancePtr()->IsCameraMove())
		{
			VectorCopy(CameraPosition, pos);
		}
		else
		{
			if (gmProtect->SceneLogin != 0)
			{
				gCameraManager->GetPosition(pos);
			}
		}

		MoveMainCamera();
		glColor3f(1.f, 1.f, 1.f);

		if (gmProtect->SceneLogin == 2 || gmProtect->SceneLogin == 3)
		{
			BeginBitmap();
			Width = (320.0);
			Height = (325.0);
			RenderNoBitmap(BITMAP_LOG_IN + 9, 0, 25.0, Width, Height, 0.0, 0.0, 511.75f / 512.f, 512.f / 512.f);
			RenderNoBitmap(BITMAP_LOG_IN + 10, Width, 25.0, Width, Height, 0.0, 0.0, 511.75f / 512.f, 512.f / 512.f);
			EndBitmap();
		}

		Width = ((float)GetScreenWidth() / GetWindowsX);

		glClearColor(0.f, 0.f, 0.f, 1.f);
		BeginOpengl(0, 25, 640, 430);
		CreateFrustrum(Width, pos);

		if (!CUIMng::Instance().m_CreditWin.IsShow())
		{
			if (CCameraMove::GetInstancePtr()->IsCameraMove())
			{
				CameraViewFar = CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 330.f;
			}
			if (gmProtect->SceneLogin == 3)
				BeginOpengl();

			RenderTerrain(false);
			CameraViewFar = 7000.f;
			RenderCharactersClient();
			gGoboidManager->RenderBugs();
			RenderObjects();
			RenderJoints();
			RenderEffects();
			CheckSprites();
			RenderLeaves();
			RenderBoids();
			RenderObjects_AfterCharacter();
			ThePetProcess().RenderPets();

			if (gmProtect->SceneLogin == 3)
				EndOpengl();
		}

		BeginSprite();
		RenderSprites();
		RenderParticles();
		EndSprite();
		BeginBitmap();

		if (CCameraMove::GetInstancePtr()->IsTourMode())
		{
			if (gmProtect->SceneLogin == 3)
			{
				EnableAlphaBlend4();
				glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
				float fScale = (sinf(WorldTime * 0.0005f) + 1.f) * 0.00011f;

				RenderNoBitmap(BITMAP_CHROME + 3, 0.0f, 0.0f, 640.0f, 480.0f, 800.f * fScale, 600.f * fScale, (800.f) / 1024.f - 800.f * fScale * 2, (600.f) / 1024.f - 600.f * fScale * 2);
				float fAngle = WorldTime * 0.00018f;
				float fLumi = 1.0f - (sinf(WorldTime * 0.0015f) + 1.f) * 0.25f;
				glColor4f(fLumi * 0.3f, fLumi * 0.3f, fLumi * 0.7f, fLumi);
				fScale = (sinf(WorldTime * 0.0015f) + 1.f) * 0.00021f;
				RenderNoBitmapLocalRotate(BITMAP_CHROME + 4, 320.0f, 240.0f, 1150.0f, 1150.0f, fAngle, fScale * 512.f, fScale * 512.f, (512.f) / 512.f - fScale * 2 * 512.f, (512.f) / 512.f - fScale * 2 * 512.f);

				EnableAlphaTest();
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
				RenderNoColor(0, 0, 640, 25);
				RenderNoColor(0, 480 - 25, 640, 25);

				glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
				RenderNoColor(0, 25, 640, 430);
			}

			g_fMULogoAlpha += 0.02f;
			if (g_fMULogoAlpha > 10.0f)
				g_fMULogoAlpha = 10.0f;

			EnableAlphaBlend();
			glColor4f(g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f);
			RenderBitmap(BITMAP_LOG_IN + 17, AddMiddleX() + 320.0f - 128.0f * 0.8f, 25.0f, 256.0f * 0.8f, 128.0f * 0.8f);

			EnableAlphaTest();
			glColor4f(g_fMULogoAlpha, g_fMULogoAlpha, g_fMULogoAlpha, g_fMULogoAlpha);
			RenderBitmap(BITMAP_LOG_IN + 16, AddMiddleX() + 320.0f - 128.0f * 0.8f, 25.0f, 256.0f * 0.8f, 128.0f * 0.8f);
		}
	}

	SIZE Size;
	char Text[100];

	g_pRenderText->SetFont(g_hFont);

	InputTextWidth = 256;
	glColor3f(0.8f, 0.7f, 0.6f);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 128);

	strcpy(Text, GlobalText[454]);
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
	g_pRenderText->RenderText(AddMiddleX() + 335 - Size.cx * gwinhandle->GetScreenX() / WindowWidth, gwinhandle->GetScreenY() - Size.cy * gwinhandle->GetScreenX() / WindowWidth - 1, Text);

	strcpy(Text, GlobalText[455]);

	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
	g_pRenderText->RenderText(AddMiddleX() + 335, gwinhandle->GetScreenY() - Size.cy * gwinhandle->GetScreenX() / WindowWidth - 1, Text);

	sprintf(Text, GlobalText[456], m_ExeVersion);

	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
	g_pRenderText->RenderText(0, gwinhandle->GetScreenY() - Size.cy * gwinhandle->GetScreenX() / WindowWidth - 1, Text);

	RenderInfomation();

#ifdef ENABLE_EDIT
	RenderDebugWindow();
#endif //ENABLE_EDIT

	EndBitmap();

	EndOpengl();

	return true;
}

float CameraDistanceTarget = 1000.f;
float CameraDistance = CameraDistanceTarget;

bool update_camera_in_real_time()
{
	if (SceneFlag == MAIN_SCENE)
	{
		float Matrix[3][4];
		vec3_t Position, TransformPosition;

		Vector(0.f, -CameraDistance, 0.f, Position);//-750
		AngleMatrix(CameraAngle, Matrix);
		VectorIRotate(Position, Matrix, TransformPosition);

		g_pCatapultWindow->GetCameraPos(Position);

		VectorAdd(Position, TransformPosition, CameraPosition);
	}

	return false;
}

void LoadingScene(HDC hDC)
{
	g_ConsoleDebug->Write(MCD_NORMAL, "LoadingScene_Start");

	CUIMng& rUIMng = CUIMng::Instance();
	if (!InitLoading)
	{
		LoadingWorld = 9999999;

		InitLoading = true;

		LoadBitmap("Interface\\LSBg01.JPG", BITMAP_TITLE, GL_LINEAR);
		LoadBitmap("Interface\\LSBg02.JPG", BITMAP_TITLE + 1, GL_LINEAR);
		LoadBitmap("Interface\\LSBg03.JPG", BITMAP_TITLE + 2, GL_LINEAR);
		LoadBitmap("Interface\\LSBg04.JPG", BITMAP_TITLE + 3, GL_LINEAR);

		::StopMp3(g_lpszMp3[MUSIC_LOGIN_THEME]);

		rUIMng.m_pLoadingScene = new CLoadingScene;
		rUIMng.m_pLoadingScene->Create();
	}

	FogEnable = false;
	BeginOpengl();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BeginBitmap();

	rUIMng.m_pLoadingScene->Render();

	EndBitmap();
	EndOpengl();
	glFlush();
	SwapBuffers(hDC);

	SAFE_DELETE(rUIMng.m_pLoadingScene);

	SceneFlag = MAIN_SCENE;
	for (int i = 0; i < 4; ++i)
		DeleteBitmap(BITMAP_TITLE + i);

	ClearInput();

	g_ConsoleDebug->Write(MCD_NORMAL, "LoadingScene_End");
}

bool MoveMainCamera()
{
	bool bLockCamera = false;

	CAMERA_INFO* CurrentCam = CameraFactorPtr->CurrentCam();

	int iWorld = World;

	if ((iWorld == WD_73NEW_LOGIN_SCENE || iWorld == WD_77NEW_LOGIN_SCENE) && CCameraMove::GetInstancePtr()->IsTourMode())
	{
		if (iWorld == WD_77NEW_LOGIN_SCENE)
			CameraFOV = 61.0;
		else
			CameraFOV = 65.0;
	}
	else if (SceneFlag == MAIN_SCENE)
	{
		CameraFOV = CurrentCam->zoom;
	}
	else
	{
		if (gCameraManager->Enable)
			CameraFOV = gCameraManager->GetFov();
		else
			CameraFOV = 35.0;
	}


#ifdef ENABLE_EDIT2
	{
		bool EditMove = false;
		if (!g_pUIManager->IsInputEnable())
		{
			if (HIBYTE(GetAsyncKeyState(VK_INSERT)) == 128)
				CameraAngle[2] += 15;
			if (HIBYTE(GetAsyncKeyState(VK_DELETE)) == 128)
				CameraAngle[2] -= 15;

			vec3_t p1, p2;
			Vector(0.f, 0.f, 0.f, p1);
			FLOAT Velocity = sqrtf(TERRAIN_SCALE * TERRAIN_SCALE) * 1.25f;

			if (HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128)// || (MouseX<=0 && MouseY>=100))
			{
				Vector(-Velocity, -Velocity, 0.f, p1);
				EditMove = true;
			}
			if (HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128)// || (MouseX>=639 && MouseY>=100))
			{
				Vector(Velocity, Velocity, 0.f, p1);
				EditMove = true;
			}
			if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128)// || (MouseY<=0 && MouseX>=100 && MouseX<540))
			{
				Vector(-Velocity, Velocity, 0.f, p1);
				EditMove = true;
			}
			if (HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128)// || (MouseY>=479))
			{
				Vector(Velocity, -Velocity, 0.f, p1);
				EditMove = true;
			}

			glPushMatrix();
			glLoadIdentity();
			glRotatef(-CameraAngle[2], 0.f, 0.f, 1.f);
			float Matrix[3][4];
			GetOpenGLMatrix(Matrix);
			glPopMatrix();
			VectorRotate(p1, Matrix, p2);
			VectorAdd(Hero->Object.Position, p2, Hero->Object.Position);
		}

		if (gMapManager->InChaosCastle() == false || !Hero->Object.m_bActionStart)
		{
			if (World == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
			{
			}
			else
				if (World == -1 || Hero->Helper.Type != MODEL_HELPER + 3 || Hero->SafeZone)
				{
					Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]);
				}
				else
				{
					if (World == WD_8TARKAN || World == WD_10HEAVEN)
						Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]) + 90.f;
					else
						Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]) + 30.f;
				}
		}

		if (EditMove)
		{
			BYTE PathX[1];
			BYTE PathY[1];
			PathX[0] = (BYTE)(Hero->Object.Position[0] / TERRAIN_SCALE);
			PathY[0] = (BYTE)(Hero->Object.Position[1] / TERRAIN_SCALE);
			SendCharacterMove(Hero->Key, Hero->Object.Angle[2], 1, PathX, PathY, PathX[0], PathY[0]);
			Hero->Path.PathNum = 0;
		}
	}
#endif //ENABLE_EDIT2

	CameraAngle[0] = 0.f;
	CameraAngle[1] = 0.f;

	if (CameraTopViewEnable)
	{
		CameraViewFar = 3200.f;
		CameraPosition[0] = Hero->Object.Position[0];
		CameraPosition[1] = Hero->Object.Position[1];
		CameraPosition[2] = CameraViewFar;
	}
	else
	{
		int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));
		vec3_t Position, TransformPosition;
		float Matrix[3][4];

		if (battleCastle::InBattleCastle2(Hero->Object.Position))
		{
			CameraViewFar = 3000.f;
		}
		else if (gMapManager->InBattleCastle() && SceneFlag == MAIN_SCENE)
		{
			CameraViewFar = 2500.f;
		}
		else if (iWorld == WD_51HOME_6TH_CHAR)
		{
			CameraViewFar = 3220.000;
		}
		else if (gMapManager->IsPKField() || IsDoppelGanger2())
		{
			CameraViewFar = 3700.0f;
		}
		else
		{
			if (g_shCameraLevel != 0)
			{
				switch (g_shCameraLevel)
				{
				case 1:
					CameraViewFar = 2500.000;
					break;
				case 2:
					CameraViewFar = 2600.000;
					break;
				case 3:
					CameraViewFar = 2950.000;
					break;
				case 5:
				case 4:
					CameraViewFar = 3200.000;
					break;
				}
			}
			else
			{
				if (SceneFlag == LOG_IN_SCENE)
				{
					if (CCameraMove::GetInstancePtr()->IsTourMode() == false)
					{
						if (gmProtect->SceneLogin != 0)
						{
							CameraViewFar = gCameraManager->CalcViewFar(2400.000);
						}
					}
				}
				else if (SceneFlag == CHARACTER_SCENE)
				{
					if (gmProtect->SceneCharacter != 0)
					{
						CameraViewFar = gCameraManager->CalcViewFar(2400.000);
					}
					else
					{
						CameraViewFar = 3500.f;
					}
				}
				else if (g_Direction.m_CKanturu.IsMayaScene())
				{
					CameraViewFar = 2300.0f;
				}
				else
				{
					CameraViewFar = CurrentCam->camWidthFar;
				}
			}
		}

		Vector(0.f, -CameraDistance, 0.f, Position);//-750
		AngleMatrix(CameraAngle, Matrix);
		VectorIRotate(Position, Matrix, TransformPosition);

		if (SceneFlag == MAIN_SCENE)
		{
			g_pCatapultWindow->GetCameraPos(Position);
		}
		else if (CCameraMove::GetInstancePtr()->IsTourMode())
		{
			CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
			CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
			CameraViewFar = 390.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();
		}

		if (g_Direction.IsDirection() && !g_Direction.m_bDownHero)
		{
			Hero->Object.Position[2] = 300.0f;
			g_shCameraLevel = g_Direction.GetCameraPosition(Position);
		}
		else if (gMapManager->IsPKField() || IsDoppelGanger2())
		{
			g_shCameraLevel = 5;
		}
		else if (IsDoppelGanger1())
		{
			g_shCameraLevel = 5;
		}
		else
		{
			g_shCameraLevel = 0;
		}

		if (gmProtect->SceneLogin != 3 && gmProtect->SceneLogin != 4)
		{
			if (CCameraMove::GetInstancePtr()->IsTourMode())
			{
				vec3_t temp = { 0.0f, 0.0f, -100.0f };
				VectorAdd(TransformPosition, temp, TransformPosition);
			}
		}

		VectorAdd(Position, TransformPosition, CameraPosition);

		if (gMapManager->InBattleCastle() == true)
		{
			CameraPosition[2] = 255.f;//700
		}
		else if (CCameraMove::GetInstancePtr()->IsTourMode() == false)
		{
			CameraPosition[2] = Hero->Object.Position[2];//700
		}

		if ((TerrainWall[iIndex] & TW_HEIGHT) == TW_HEIGHT)
		{
			CameraPosition[2] = g_fSpecialHeight = 1200.f + 1;
		}

		if (SceneFlag == MAIN_SCENE)
		{
			CameraPosition[2] += CameraDistance - CurrentCam->Rot.Z;
		}
		else
		{
			CameraPosition[2] += CameraDistance - 150.0;
		}

		if (CCameraMove::GetInstancePtr()->IsTourMode())
		{
			if (gmProtect->SceneLogin != 3)
			{
				CCameraMove::GetInstancePtr()->SetAngleFrustum(-112.5f);
				CameraAngle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
			}
			else
			{
				CameraAngle[0] = -78.5;
			}

			CameraAngle[1] = 0.0f;
			CameraAngle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
		}
		else if (SceneFlag == LOG_IN_SCENE)
		{
			if (gmProtect->SceneLogin != 0)
			{
				gCameraManager->GetAngle(CameraAngle);

				gCameraManager->GetPosition(CameraPosition);
			}
		}
		else if (SceneFlag == CHARACTER_SCENE)
		{
			if (gmProtect->SceneCharacter != 0)
			{
				gCameraManager->GetAngle(CameraAngle);
				gCameraManager->GetPosition(CameraPosition);
			}
			else
			{
				CameraAngle[0] = -84.5f;
				CameraAngle[1] = 0.0f;
				CameraAngle[2] = -75.0f;
				CameraPosition[0] = 9758.93f;
				CameraPosition[1] = 18913.11f;
				CameraPosition[2] = 675.5f;
			}
		}
		else
		{
			CameraAngle[0] = CurrentCam->Rot.Y;
		}

		CameraAngle[0] += EarthQuake;

		if ((TerrainWall[iIndex] & TW_CAMERA_UP) == TW_CAMERA_UP)
		{
			if (g_fCameraCustomDistance <= CUSTOM_CAMERA_DISTANCE1)
			{
				g_fCameraCustomDistance += timefac(10);
			}
		}
		else
		{
			if (g_fCameraCustomDistance > 0)
			{
				g_fCameraCustomDistance -= timefac(10);
			}
		}

		if (g_fCameraCustomDistance > 0)
		{
			vec3_t angle = { 0.f, 0.f, -45.f };
			Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
			AngleMatrix(angle, Matrix);
			VectorIRotate(Position, Matrix, TransformPosition);
			interpolatesum(CameraPosition, TransformPosition);
		}
		else if (g_fCameraCustomDistance < 0)
		{
			vec3_t angle = { 0.f, 0.f, -45.f };
			Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
			AngleMatrix(angle, Matrix);
			VectorIRotate(Position, Matrix, TransformPosition);
			interpolatesum(CameraPosition, TransformPosition);
		}
	}

	if (iWorld == WD_5UNKNOWN)
	{
		CameraAngle[0] += sinf(WorldTime * 0.0005f) * 2.f;
		CameraAngle[1] += sinf(WorldTime * 0.0008f) * 2.5f;
	}
	else if (CCameraMove::GetInstancePtr()->IsTourMode())
	{
		CameraDistanceTarget = 1100.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 0.1f;
		CameraDistance = CameraDistanceTarget;
	}
	else
	{
		if (gMapManager->InBattleCastle())
		{
			CameraDistanceTarget = 1100.f;
			CameraDistance = CameraDistanceTarget;
		}
		else
		{
			switch (g_shCameraLevel)
			{
			case 0:
				CameraDistanceTarget = 1000.f;
				CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
				break;
			case 1:
				CameraDistanceTarget = 1100.f;
				CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
				break;
			case 2:
				CameraDistanceTarget = 1200.f;
				CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
				break;
			case 3:
				CameraDistanceTarget = 1300.f;
				CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
				break;
			case 4:
				CameraDistanceTarget = 1400.f;
				CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
				break;
			case 5:
				CameraDistanceTarget = g_Direction.m_fCameraViewFar;
				CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
				break;
			}
		}
	}
	return bLockCamera;
}

void MoveMainScene()
{
	if (!InitMainScene)
	{
		g_pMainFrame->ResetSkillHotKey();

		CHARACTER* pCharacter = gmCharacters->GetCharacter(SelectedHero);

		InitMainScene = true;

		if (pCharacter)
		{
			g_ConsoleDebug->Write(MCD_NORMAL, "Join the game with the following character: %s", pCharacter->ID);

			g_ErrorReport.Write("> Character selected <%d> \"%s\"\r\n", SelectedHero + 1, pCharacter->ID);

			g_ConsoleDebug->Write(MCD_SEND, "SendRequestJoinMapServer");

			SendRequestJoinMapServer(pCharacter->ID);
		}

		CUIMng::Instance().CreateMainScene();

		CameraAngle[2] = -45.f;

		ClearInput();
		InputEnable = false;
		TabInputEnable = false;
		InputTextWidth = 256;
		InputTextMax[0] = 42;
		InputTextMax[1] = 10;
		InputNumber = 2;
		for (int i = 0; i < MAX_WHISPER; i++)
		{
			g_pChatListBox->AddText("", "", SEASON3B::TYPE_WHISPER_MESSAGE);
		}

		g_GuildNotice[0][0] = '\0';
		g_GuildNotice[1][0] = '\0';

		g_pPartyManager->Create();

		g_pChatListBox->ClearAll();

		g_pSlideHelpMgr->Init();

		g_pUIMapName->Init();

		g_GuildCache.Reset();

		g_PortalMgr.Reset();

		ClearAllObjectBlurs();

		SetFocus(gwinhandle->GethWnd());

		//CameraFactorPtr->recover_backup_cam();

		g_ErrorReport.Write("> Main Scene init success. ");
		g_ErrorReport.WriteCurrentTime();

		g_ConsoleDebug->Write(MCD_NORMAL, "MainScene Init Success");
	}

	if (CurrentProtocolState == RECEIVE_JOIN_MAP_SERVER)
	{
		EnableMainRender = true;
	}
	if (EnableMainRender == false)
	{
		return;
	}

	EarthQuake *= timepow(0.2f);

	InitTerrainLight();
	//return;

	if (gmAIController->IsRunning())
	{
		gmAIController->WhatToDoNext();
	}

	CheckInventory = NULL;
	CheckSkill = -1;
	MouseOnWindow = false;

	if (!CameraTopViewEnable && LoadingWorld < 30)
	{
		if (CheckMouseIn((int)PositionX_The_Mid(640), (int)(GetWindowsY - 51), 640, 51))
			MouseOnWindow = true;

		g_pPartyManager->Update();

		g_pNewUISystem->Update();

#if CUSTOM_CHOTROI
		gCusChoTroi.UpdateInputFocus();
#endif

		if (MouseLButton == true && false == g_pNewUISystem->CheckMouseUse() && g_dwMouseUseUIID == 0 && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
		{
			g_pWindowMgr->SetWindowsEnable(FALSE);

			g_pFriendMenu->HideMenu();

			g_dwKeyFocusUIID = 0;

			if (GetFocus() != gwinhandle->GethWnd())
			{
				SaveIMEStatus();
				SetFocus(gwinhandle->GethWnd());
			}
		}
		MoveInterface();
		MoveTournamentInterface();
		if (ErrorMessage != MESSAGE_LOG_OUT)
			g_pUIManager->UpdateInput();
	}

	if (ErrorMessage != NULL)
		MouseOnWindow = true;

	MoveObjects();
	if (!CameraTopViewEnable)
		MoveItems();

	if ((World == WD_0LORENCIA && HeroTile != 4) ||
		(World == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
		|| World == WD_3NORIA
		|| World == WD_7ATLANSE
		|| gMapManager->InDevilSquare() == true
		|| World == WD_10HEAVEN
		|| gMapManager->InChaosCastle() == true
		|| gMapManager->InBattleCastle() == true
		|| M31HuntingGround::IsInHuntingGround() == true
		|| M33Aida::IsInAida() == true
		|| M34CryWolf1st::IsCyrWolf1st() == true
		|| World == WD_42CHANGEUP3RD_2ND
		|| IsIceCity()
		|| IsSantaTown()
		|| gMapManager->IsPKField()
		|| IsDoppelGanger2()
		|| gMapManager->IsEmpireGuardian1()
		|| gMapManager->IsEmpireGuardian2()
		|| gMapManager->IsEmpireGuardian3()
		|| gMapManager->IsEmpireGuardian4()
		|| IsUnitedMarketPlace()
		)
	{
		MoveLeaves();
	}
	MoveBoids();
	MoveFishs();

	gmHeadChat->MoveChat();

	UpdatePersonalShopTitleImp();
	MoveHero();
	MoveCharactersClient();
	MoveMainCamera();
	ThePetProcess().UpdatePets();
	gGoboidManager->MoveBugs();


	MovePoints();
	MovePlanes();
	MoveEffects();
	MoveJoints();
	MoveParticles();
	MovePointers();

	g_Direction.CheckDirection();

#ifdef ENABLE_EDIT
	EditObjects();
#endif //ENABLE_EDIT

	g_GameCensorship->Update();

	g_ConsoleDebug->UpdateMainScene();
}

namespace
{
	double g_DebugFrameMs = 0.0;
	double g_DebugMoveMs = 0.0;
	double g_DebugRenderMs = 0.0;
	double g_DebugTerrainMs = 0.0;
	double g_DebugObjectMs = 0.0;
	double g_DebugCharacterMs = 0.0;
	double g_DebugCharClientMs = 0.0;
	double g_DebugCharItemsMs = 0.0;
	double g_DebugCharPetsMs = 0.0;
	double g_DebugCharBodyMs = 0.0;
	double g_DebugCharPartMs = 0.0;
	double g_DebugPlayerCharBodyMs = 0.0;
	double g_DebugPlayerCharPartMs = 0.0;
	double g_DebugPartObjectMs = 0.0;
	double g_DebugPartTransformMs = 0.0;
	double g_DebugPartBodyMs = 0.0;
	double g_DebugPartFxMs = 0.0;
	double g_DebugPartFxBrightMs = 0.0;
	int g_DebugPartFxBrightCount = 0;
	int g_DebugPartFxSpriteCount = 0;
	int g_DebugPartFxJointCount = 0;
	int g_DebugPartFxParticleCount = 0;
	int g_DebugPartFxEffectCount = 0;
	double g_DebugPartLinkMs = 0.0;
	double g_DebugPlayerPartObjectMs = 0.0;
	double g_DebugPlayerPartTransformMs = 0.0;
	double g_DebugPlayerPartBodyMs = 0.0;
	double g_DebugPlayerPartFxMs = 0.0;
	double g_DebugPlayerPartLinkMs = 0.0;
	double g_DebugPlayerPartFxMeshMs = 0.0;
	int g_DebugPlayerPartFxMeshCount = 0;
	int g_DebugPlayerPartFxMeshHitCount = 0;
	int g_DebugPlayerPartFxMeshEarlyCount = 0;
	int g_DebugPlayerPartFxMeshCpuDrawCount = 0;
	int g_DebugPlayerPartFxGpuFailFlag = 0;
	int g_DebugPlayerPartFxGpuFailResolved = 0;
	int g_DebugPlayerPartFxGpuFailMesh = 0;
	int g_DebugPlayerPartFxGpuFailScale = 0;
	int g_DebugPlayerPartFxGpuFailOff = 0;
	int g_DebugPlayerPartFxCpuDrawTexture = 0;
	int g_DebugPlayerPartFxCpuDrawBright = 0;
	int g_DebugPlayerPartFxCpuDrawChrome = 0;
	int g_DebugPlayerPartFxCpuDrawOther = 0;
	int g_DebugPlayerPartFxOffWeapon = 0;
	int g_DebugPlayerPartFxOffArmor = 0;
	int g_DebugPlayerPartFxOffWing = 0;
	int g_DebugPlayerPartFxOffHelper = 0;
	int g_DebugPlayerPartFxOffGrade15 = 0;
	int g_DebugPlayerPartFxOffOther = 0;
	int g_DebugPlayerPartFxSpriteCount = 0;
	int g_DebugPlayerPartFxJointCount = 0;
	int g_DebugPlayerPartFxParticleCount = 0;
	int g_DebugPlayerPartFxEffectCount = 0;
	double g_DebugCharFxMs = 0.0;
	double g_DebugCharBattleCastleMs = 0.0;
	int g_DebugGpuAssistPrep = 0;
	int g_DebugGpuAssistOff = 0;
	int g_DebugGpuAssistType = 0;
	int g_DebugGpuAssistFlag = 0;
	int g_DebugGpuAssistFlagScale = 0;
	int g_DebugGpuAssistFlagResolved = 0;
	int g_DebugGpuAssistFlagChrome = 0;
	int g_DebugGpuAssistFlagMetal = 0;
	int g_DebugGpuAssistFlagLightmap = 0;
	int g_DebugGpuAssistFlagWave = 0;
	int g_DebugGpuAssistFlagOther = 0;
	int g_DebugGpuAssistMesh = 0;
	int g_DebugGpuAssistHit = 0;
	int g_DebugGpuAssistHitRenderMesh = 0;
	int g_DebugGpuAssistHitBatch = 0;
	int g_DebugGpuAssistHitTranslate = 0;
	int g_DebugGpuEffectBatchCheck = 0;
	int g_DebugGpuEffectBatchUsed = 0;
	int g_DebugGpuEffectBatchBuildFail = 0;
	int g_DebugGpuEffectBatchEmpty = 0;
	int g_DebugGpuEffectBatchRejectNum = 0;
	int g_DebugGpuEffectBatchRejectHidden = 0;
	int g_DebugGpuEffectBatchRejectTexture = 0;
	int g_DebugGpuEffectBatchRejectTransform = 0;
	int g_DebugGpuEffectBatchRejectScale = 0;
	int g_DebugGpuEffectBatchRejectFlag = 0;
	int g_DebugGpuEffectBatchRejectPass = 0;
	int g_DebugGpuEffectBatchRejectShader = 0;
	int g_DebugGpuEffectBatchRejectReady = 0;
	int g_DebugGpuAssistFallback = 0;
	int g_DebugGpuAssistBoneUpload = 0;
	int g_DebugGpuAssistBoneUploadMatrices = 0;
	int g_DebugGpuAssistShaderBind = 0;
	int g_DebugGpuAssistColorRead = 0;
	int g_DebugPlayerGpuAssistHit = 0;
	int g_DebugPlayerGpuAssistFallback = 0;
	double g_DebugEffectMs = 0.0;
	double g_DebugEffectJointMs = 0.0;
	double g_DebugEffectModelMs = 0.0;
	double g_DebugEffectSpriteMs = 0.0;
	double g_DebugEffectParticleMs = 0.0;
	double g_DebugEffectAfterMs = 0.0;
	double g_DebugEffectOtherMs = 0.0;
	int g_DebugRenderSpriteCount = 0;
	int g_DebugRenderSpriteStateChanges = 0;
	int g_DebugRenderSpriteTextureChanges = 0;
	int g_DebugRenderSpriteDrawCalls = 0;
	int g_DebugCrowdFxCap0 = 0;
	int g_DebugCrowdFxCap1 = 0;
	int g_DebugCrowdFxCap2 = 0;
	int g_DebugCrowdFxCap3 = 0;
	int g_DebugCrowdFxCap4 = 0;
	double g_DebugUiMs = 0.0;

	double DebugNowMs()
	{
		return (g_pTimer != NULL) ? g_pTimer->GetAbsTime() : static_cast<double>(timeGetTime());
	}

	void ResetRenderBreakdown()
	{
		g_DebugTerrainMs = 0.0;
		g_DebugObjectMs = 0.0;
		g_DebugCharacterMs = 0.0;
		g_DebugCharClientMs = 0.0;
		g_DebugCharItemsMs = 0.0;
		g_DebugCharPetsMs = 0.0;
		g_DebugCharBodyMs = 0.0;
		g_DebugCharPartMs = 0.0;
		g_DebugPlayerCharBodyMs = 0.0;
		g_DebugPlayerCharPartMs = 0.0;
		g_DebugPartObjectMs = 0.0;
		g_DebugPartTransformMs = 0.0;
		g_DebugPartBodyMs = 0.0;
		g_DebugPartFxMs = 0.0;
		g_DebugPartFxBrightMs = 0.0;
		g_DebugPartFxBrightCount = 0;
		g_DebugPartFxSpriteCount = 0;
		g_DebugPartFxJointCount = 0;
		g_DebugPartFxParticleCount = 0;
		g_DebugPartFxEffectCount = 0;
		g_DebugPartLinkMs = 0.0;
		g_DebugPlayerPartObjectMs = 0.0;
		g_DebugPlayerPartTransformMs = 0.0;
		g_DebugPlayerPartBodyMs = 0.0;
		g_DebugPlayerPartFxMs = 0.0;
		g_DebugPlayerPartLinkMs = 0.0;
		g_DebugPlayerPartFxMeshMs = 0.0;
		g_DebugPlayerPartFxMeshCount = 0;
		g_DebugPlayerPartFxMeshHitCount = 0;
		g_DebugPlayerPartFxMeshEarlyCount = 0;
		g_DebugPlayerPartFxMeshCpuDrawCount = 0;
		g_DebugPlayerPartFxGpuFailFlag = 0;
		g_DebugPlayerPartFxGpuFailResolved = 0;
		g_DebugPlayerPartFxGpuFailMesh = 0;
		g_DebugPlayerPartFxGpuFailScale = 0;
		g_DebugPlayerPartFxGpuFailOff = 0;
		g_DebugPlayerPartFxCpuDrawTexture = 0;
		g_DebugPlayerPartFxCpuDrawBright = 0;
		g_DebugPlayerPartFxCpuDrawChrome = 0;
		g_DebugPlayerPartFxCpuDrawOther = 0;
		g_DebugPlayerPartFxOffWeapon = 0;
		g_DebugPlayerPartFxOffArmor = 0;
		g_DebugPlayerPartFxOffWing = 0;
		g_DebugPlayerPartFxOffHelper = 0;
		g_DebugPlayerPartFxOffGrade15 = 0;
		g_DebugPlayerPartFxOffOther = 0;
		g_DebugPlayerPartFxSpriteCount = 0;
		g_DebugPlayerPartFxJointCount = 0;
		g_DebugPlayerPartFxParticleCount = 0;
		g_DebugPlayerPartFxEffectCount = 0;
		g_DebugCharFxMs = 0.0;
		g_DebugCharBattleCastleMs = 0.0;
		g_DebugGpuAssistPrep = 0;
		g_DebugGpuAssistOff = 0;
		g_DebugGpuAssistType = 0;
		g_DebugGpuAssistFlag = 0;
		g_DebugGpuAssistFlagScale = 0;
		g_DebugGpuAssistFlagResolved = 0;
		g_DebugGpuAssistFlagChrome = 0;
		g_DebugGpuAssistFlagMetal = 0;
		g_DebugGpuAssistFlagLightmap = 0;
		g_DebugGpuAssistFlagWave = 0;
		g_DebugGpuAssistFlagOther = 0;
		g_DebugGpuAssistMesh = 0;
		g_DebugGpuAssistHit = 0;
		g_DebugGpuAssistHitRenderMesh = 0;
		g_DebugGpuAssistHitBatch = 0;
		g_DebugGpuAssistHitTranslate = 0;
		g_DebugGpuEffectBatchCheck = 0;
		g_DebugGpuEffectBatchUsed = 0;
		g_DebugGpuEffectBatchBuildFail = 0;
		g_DebugGpuEffectBatchEmpty = 0;
		g_DebugGpuEffectBatchRejectNum = 0;
		g_DebugGpuEffectBatchRejectHidden = 0;
		g_DebugGpuEffectBatchRejectTexture = 0;
		g_DebugGpuEffectBatchRejectTransform = 0;
		g_DebugGpuEffectBatchRejectScale = 0;
		g_DebugGpuEffectBatchRejectFlag = 0;
		g_DebugGpuEffectBatchRejectPass = 0;
		g_DebugGpuEffectBatchRejectShader = 0;
		g_DebugGpuEffectBatchRejectReady = 0;
		g_DebugGpuAssistFallback = 0;
		g_DebugGpuAssistBoneUpload = 0;
		g_DebugGpuAssistBoneUploadMatrices = 0;
		g_DebugGpuAssistShaderBind = 0;
		g_DebugGpuAssistColorRead = 0;
		g_DebugPlayerGpuAssistHit = 0;
		g_DebugPlayerGpuAssistFallback = 0;
		g_DebugEffectMs = 0.0;
		g_DebugEffectJointMs = 0.0;
		g_DebugEffectModelMs = 0.0;
		g_DebugEffectSpriteMs = 0.0;
		g_DebugEffectParticleMs = 0.0;
		g_DebugEffectAfterMs = 0.0;
		g_DebugEffectOtherMs = 0.0;
		g_DebugRenderSpriteCount = 0;
		g_DebugRenderSpriteStateChanges = 0;
		g_DebugRenderSpriteTextureChanges = 0;
		g_DebugRenderSpriteDrawCalls = 0;
		g_DebugCrowdFxCap0 = 0;
		g_DebugCrowdFxCap1 = 0;
		g_DebugCrowdFxCap2 = 0;
		g_DebugCrowdFxCap3 = 0;
		g_DebugCrowdFxCap4 = 0;
		g_DebugUiMs = 0.0;
	}

	void DebugAddMs(double& target, double startMs)
	{
		target += DebugNowMs() - startMs;
	}

	void DebugAddEffectMs(double& target, double startMs)
	{
		const double elapsedMs = DebugNowMs() - startMs;
		target += elapsedMs;
		g_DebugEffectMs += elapsedMs;
	}

	void RenderFrameDebugOverlay()
	{
		if (SceneFlag != MAIN_SCENE || g_pRenderText == NULL)
		{
			return;
		}

		char text[192];
		const float x = 6.0f;
		float y = static_cast<float>(gwinhandle->GetScreenY()) - 562.0f;
		if (y < 20.0f)
		{
			y = 20.0f;
		}

		BeginBitmap();
		glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
		RenderColor(x, y, 392.0f, 554.0f, 0.0f, 0);
		EndRenderColor();

		g_pRenderText->SetFont(g_hFontBold ? g_hFontBold : g_hFont);
		g_pRenderText->SetBgColor(0, 0, 0, 160);
		g_pRenderText->SetTextColor(255, 230, 80, 255);

		sprintf(text, "FRAME  : %.2f ms", g_DebugFrameMs);
		g_pRenderText->RenderText(x + 10.0f, y + 8.0f, text);
		sprintf(text, "MOVE   : %.2f ms", g_DebugMoveMs);
		g_pRenderText->RenderText(x + 10.0f, y + 28.0f, text);
		sprintf(text, "RENDER : %.2f ms", g_DebugRenderMs);
		g_pRenderText->RenderText(x + 10.0f, y + 48.0f, text);
		sprintf(text, "TERRAIN: %.2f ms", g_DebugTerrainMs);
		g_pRenderText->RenderText(x + 10.0f, y + 68.0f, text);
		sprintf(text, "OBJECT : %.2f ms", g_DebugObjectMs);
		g_pRenderText->RenderText(x + 10.0f, y + 88.0f, text);
		sprintf(text, "CHAR   : %.2f ms", g_DebugCharacterMs);
		g_pRenderText->RenderText(x + 10.0f, y + 108.0f, text);
		sprintf(text, "CHARACT: %.2f ms", g_DebugCharClientMs);
		g_pRenderText->RenderText(x + 10.0f, y + 128.0f, text);
		sprintf(text, "ITEMS  : %.2f ms", g_DebugCharItemsMs);
		g_pRenderText->RenderText(x + 10.0f, y + 148.0f, text);
		sprintf(text, "PETS   : %.2f ms", g_DebugCharPetsMs);
		g_pRenderText->RenderText(x + 10.0f, y + 168.0f, text);
		sprintf(text, "CH_BODY: %.2f ms", g_DebugCharBodyMs);
		g_pRenderText->RenderText(x + 10.0f, y + 188.0f, text);
		sprintf(text, "CH_PART: %.2f ms", g_DebugCharPartMs);
		g_pRenderText->RenderText(x + 10.0f, y + 208.0f, text);
		sprintf(text, "PT O/X/B/F/L: %.2f/%.2f/%.2f/%.2f/%.2f", g_DebugPartObjectMs, g_DebugPartTransformMs, g_DebugPartBodyMs, g_DebugPartFxMs, g_DebugPartLinkMs);
		g_pRenderText->RenderText(x + 10.0f, y + 228.0f, text);
		sprintf(text, "FX BR/C: %.2f/%d", g_DebugPartFxBrightMs, g_DebugPartFxBrightCount);
		g_pRenderText->RenderText(x + 10.0f, y + 248.0f, text);
		sprintf(text, "FX S/J/P/E: %d/%d/%d/%d", g_DebugPartFxSpriteCount, g_DebugPartFxJointCount, g_DebugPartFxParticleCount, g_DebugPartFxEffectCount);
		g_pRenderText->RenderText(x + 10.0f, y + 268.0f, text);
		sprintf(text, "CH_FX  : %.2f ms", g_DebugCharFxMs);
		g_pRenderText->RenderText(x + 10.0f, y + 288.0f, text);
		sprintf(text, "CH_BC  : %.2f ms", g_DebugCharBattleCastleMs);
		g_pRenderText->RenderText(x + 10.0f, y + 308.0f, text);
		sprintf(text, "PLY B/P: %.2f/%.2f", g_DebugPlayerCharBodyMs, g_DebugPlayerCharPartMs);
		g_pRenderText->RenderText(x + 10.0f, y + 328.0f, text);
		sprintf(text, "PP O/X/B/F/L: %.2f/%.2f/%.2f/%.2f/%.2f", g_DebugPlayerPartObjectMs, g_DebugPlayerPartTransformMs, g_DebugPlayerPartBodyMs, g_DebugPlayerPartFxMs, g_DebugPlayerPartLinkMs);
		g_pRenderText->RenderText(x + 10.0f, y + 348.0f, text);
		sprintf(text, "EFFECT : %.2f ms", g_DebugEffectMs);
		g_pRenderText->RenderText(x + 10.0f, y + 368.0f, text);
		sprintf(text, "EF J/E/S/P/A/O: %.2f/%.2f/%.2f/%.2f/%.2f/%.2f", g_DebugEffectJointMs, g_DebugEffectModelMs, g_DebugEffectSpriteMs, g_DebugEffectParticleMs, g_DebugEffectAfterMs, g_DebugEffectOtherMs);
		g_pRenderText->RenderText(x + 10.0f, y + 388.0f, text);
		sprintf(text, "PFX M/C H/E/D: %.2f/%d %d/%d/%d", g_DebugPlayerPartFxMeshMs, g_DebugPlayerPartFxMeshCount, g_DebugPlayerPartFxMeshHitCount, g_DebugPlayerPartFxMeshEarlyCount, g_DebugPlayerPartFxMeshCpuDrawCount);
		g_pRenderText->RenderText(x + 10.0f, y + 408.0f, text);
		sprintf(text, "GPU SRC M/B/T: %d/%d/%d", g_DebugGpuAssistHitRenderMesh, g_DebugGpuAssistHitBatch, g_DebugGpuAssistHitTranslate);
		g_pRenderText->RenderText(x + 10.0f, y + 428.0f, text);
		sprintf(text, "GPU EB C/U/B/E: %d/%d/%d/%d", g_DebugGpuEffectBatchCheck, g_DebugGpuEffectBatchUsed, g_DebugGpuEffectBatchBuildFail, g_DebugGpuEffectBatchEmpty);
		g_pRenderText->RenderText(x + 10.0f, y + 448.0f, text);
		sprintf(text, "GPU EB N/H/T/X/SC/FL/P/S/R: %d/%d/%d/%d/%d/%d/%d/%d/%d", g_DebugGpuEffectBatchRejectNum, g_DebugGpuEffectBatchRejectHidden, g_DebugGpuEffectBatchRejectTexture, g_DebugGpuEffectBatchRejectTransform, g_DebugGpuEffectBatchRejectScale, g_DebugGpuEffectBatchRejectFlag, g_DebugGpuEffectBatchRejectPass, g_DebugGpuEffectBatchRejectShader, g_DebugGpuEffectBatchRejectReady);
		g_pRenderText->RenderText(x + 10.0f, y + 468.0f, text);
		sprintf(text, "GPU H/F B/M/S/C: %d/%d %d/%d/%d/%d", g_DebugPlayerGpuAssistHit, g_DebugPlayerGpuAssistFallback, g_DebugGpuAssistBoneUpload, g_DebugGpuAssistBoneUploadMatrices, g_DebugGpuAssistShaderBind, g_DebugGpuAssistColorRead);
		g_pRenderText->RenderText(x + 10.0f, y + 488.0f, text);
		sprintf(text, "PING   : %d ms", gsteady_clock->get_ping_time());
		g_pRenderText->RenderText(x + 10.0f, y + 508.0f, text);

		EndBitmap();
	}
}

void DebugAddCharBodyMs(double elapsedMs)
{
	g_DebugCharBodyMs += elapsedMs;
}

void DebugAddCharPartMs(double elapsedMs)
{
	g_DebugCharPartMs += elapsedMs;
}

void DebugAddPlayerCharBodyMs(double elapsedMs)
{
	g_DebugCharBodyMs += elapsedMs;
	g_DebugPlayerCharBodyMs += elapsedMs;
}

void DebugAddPlayerCharPartMs(double elapsedMs)
{
	g_DebugCharPartMs += elapsedMs;
	g_DebugPlayerCharPartMs += elapsedMs;
}

void DebugAddPartObjectMs(double elapsedMs)
{
	g_DebugPartObjectMs += elapsedMs;
}

void DebugAddPartTransformMs(double elapsedMs)
{
	g_DebugPartTransformMs += elapsedMs;
}

void DebugAddPartBodyMs(double elapsedMs)
{
	g_DebugPartBodyMs += elapsedMs;
}

void DebugAddPartFxMs(double elapsedMs)
{
	g_DebugPartFxMs += elapsedMs;
}

void DebugAddPartLinkMs(double elapsedMs)
{
	g_DebugPartLinkMs += elapsedMs;
}

void DebugAddPlayerPartObjectMs(double elapsedMs)
{
	g_DebugPlayerPartObjectMs += elapsedMs;
}

void DebugAddPlayerPartTransformMs(double elapsedMs)
{
	g_DebugPlayerPartTransformMs += elapsedMs;
}

void DebugAddPlayerPartBodyMs(double elapsedMs)
{
	g_DebugPlayerPartBodyMs += elapsedMs;
}

void DebugAddPlayerPartFxMs(double elapsedMs)
{
	g_DebugPlayerPartFxMs += elapsedMs;
}

void DebugAddPlayerPartLinkMs(double elapsedMs)
{
	g_DebugPlayerPartLinkMs += elapsedMs;
}

void DebugAddPlayerPartFxMeshMs(double elapsedMs)
{
	g_DebugPlayerPartFxMeshMs += elapsedMs;
	++g_DebugPlayerPartFxMeshCount;
}

void DebugAddPlayerPartFxMeshHitCount()
{
	++g_DebugPlayerPartFxMeshHitCount;
}

void DebugAddPlayerPartFxMeshEarlyCount()
{
	++g_DebugPlayerPartFxMeshEarlyCount;
}

void DebugAddPlayerPartFxMeshCpuDrawCount()
{
	++g_DebugPlayerPartFxMeshCpuDrawCount;
}

void DebugAddPlayerPartFxGpuFailFlag()
{
	++g_DebugPlayerPartFxGpuFailFlag;
}

void DebugAddPlayerPartFxGpuFailResolved()
{
	++g_DebugPlayerPartFxGpuFailResolved;
}

void DebugAddPlayerPartFxGpuFailMesh()
{
	++g_DebugPlayerPartFxGpuFailMesh;
}

void DebugAddPlayerPartFxGpuFailScale()
{
	++g_DebugPlayerPartFxGpuFailScale;
}

void DebugAddPlayerPartFxGpuFailOff()
{
	++g_DebugPlayerPartFxGpuFailOff;
}

void DebugAddPlayerPartFxCpuDrawTexture()
{
	++g_DebugPlayerPartFxCpuDrawTexture;
}

void DebugAddPlayerPartFxCpuDrawBright()
{
	++g_DebugPlayerPartFxCpuDrawBright;
}

void DebugAddPlayerPartFxCpuDrawChrome()
{
	++g_DebugPlayerPartFxCpuDrawChrome;
}

void DebugAddPlayerPartFxCpuDrawOther()
{
	++g_DebugPlayerPartFxCpuDrawOther;
}

void DebugAddPlayerPartFxOffWeapon()
{
	++g_DebugPlayerPartFxOffWeapon;
}

void DebugAddPlayerPartFxOffArmor()
{
	++g_DebugPlayerPartFxOffArmor;
}

void DebugAddPlayerPartFxOffWing()
{
	++g_DebugPlayerPartFxOffWing;
}

void DebugAddPlayerPartFxOffHelper()
{
	++g_DebugPlayerPartFxOffHelper;
}

void DebugAddPlayerPartFxOffGrade15()
{
	++g_DebugPlayerPartFxOffGrade15;
}

void DebugAddPlayerPartFxOffOther()
{
	++g_DebugPlayerPartFxOffOther;
}

void DebugAddPlayerPartFxSpriteCount()
{
	++g_DebugPlayerPartFxSpriteCount;
}

void DebugAddPlayerPartFxJointCount()
{
	++g_DebugPlayerPartFxJointCount;
}

void DebugAddPlayerPartFxParticleCount()
{
	++g_DebugPlayerPartFxParticleCount;
}

void DebugAddPlayerPartFxEffectCount()
{
	++g_DebugPlayerPartFxEffectCount;
}

void DebugAddPartFxBrightMs(double elapsedMs)
{
	g_DebugPartFxBrightMs += elapsedMs;
}

void DebugAddPartFxBrightCount()
{
	++g_DebugPartFxBrightCount;
}

void DebugAddPartFxSpriteCount()
{
	++g_DebugPartFxSpriteCount;
}

void DebugAddPartFxJointCount()
{
	++g_DebugPartFxJointCount;
}

void DebugAddPartFxParticleCount()
{
	++g_DebugPartFxParticleCount;
}

void DebugAddPartFxEffectCount()
{
	++g_DebugPartFxEffectCount;
}

void DebugAddCharFxMs(double elapsedMs)
{
	g_DebugCharFxMs += elapsedMs;
}

void DebugAddCharBattleCastleMs(double elapsedMs)
{
	g_DebugCharBattleCastleMs += elapsedMs;
}

void DebugAddGpuAssistPrep()
{
	++g_DebugGpuAssistPrep;
}

void DebugAddGpuAssistOff()
{
	++g_DebugGpuAssistOff;
}

void DebugAddGpuAssistType()
{
	++g_DebugGpuAssistType;
}

void DebugAddGpuAssistFlag()
{
	++g_DebugGpuAssistFlag;
}

void DebugAddGpuAssistFlagScale()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagScale;
}

void DebugAddGpuAssistFlagResolved()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagResolved;
}

void DebugAddGpuAssistFlagChrome()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagChrome;
}

void DebugAddGpuAssistFlagMetal()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagMetal;
}

void DebugAddGpuAssistFlagLightmap()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagLightmap;
}

void DebugAddGpuAssistFlagWave()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagWave;
}

void DebugAddGpuAssistFlagOther()
{
	++g_DebugGpuAssistFlag;
	++g_DebugGpuAssistFlagOther;
}

void DebugAddGpuAssistMesh()
{
	++g_DebugGpuAssistMesh;
}
void DebugAddGpuAssistHit()
{
	++g_DebugGpuAssistHit;
}

void DebugAddGpuAssistHitRenderMesh()
{
	++g_DebugGpuAssistHit;
	++g_DebugGpuAssistHitRenderMesh;
}

void DebugAddGpuAssistHitBatch()
{
	++g_DebugGpuAssistHit;
	++g_DebugGpuAssistHitBatch;
}

void DebugAddGpuAssistHitTranslate()
{
	++g_DebugGpuAssistHit;
	++g_DebugGpuAssistHitTranslate;
}

void DebugAddGpuEffectBatchCheck()
{
	++g_DebugGpuEffectBatchCheck;
}

void DebugAddGpuEffectBatchUsed()
{
	++g_DebugGpuEffectBatchUsed;
}

void DebugAddGpuEffectBatchBuildFail()
{
	++g_DebugGpuEffectBatchBuildFail;
}

void DebugAddGpuEffectBatchEmpty()
{
	++g_DebugGpuEffectBatchEmpty;
}

void DebugAddGpuEffectBatchRejectNum()
{
	++g_DebugGpuEffectBatchRejectNum;
}

void DebugAddGpuEffectBatchRejectHidden()
{
	++g_DebugGpuEffectBatchRejectHidden;
}

void DebugAddGpuEffectBatchRejectTexture()
{
	++g_DebugGpuEffectBatchRejectTexture;
}

void DebugAddGpuEffectBatchRejectTransform()
{
	++g_DebugGpuEffectBatchRejectTransform;
}

void DebugAddGpuEffectBatchRejectScale()
{
	++g_DebugGpuEffectBatchRejectScale;
}

void DebugAddGpuEffectBatchRejectFlag()
{
	++g_DebugGpuEffectBatchRejectFlag;
}

void DebugAddGpuEffectBatchRejectPass()
{
	++g_DebugGpuEffectBatchRejectPass;
}

void DebugAddGpuEffectBatchRejectShader()
{
	++g_DebugGpuEffectBatchRejectShader;
}

void DebugAddGpuEffectBatchRejectReady()
{
	++g_DebugGpuEffectBatchRejectReady;
}
void DebugAddGpuAssistFallback()
{
	++g_DebugGpuAssistFallback;
}

void DebugAddGpuAssistBoneUpload()
{
	++g_DebugGpuAssistBoneUpload;
}

void DebugAddGpuAssistBoneUploadMatrices(int count)
{
	g_DebugGpuAssistBoneUploadMatrices += count;
}

void DebugAddGpuAssistShaderBind()
{
	++g_DebugGpuAssistShaderBind;
}

void DebugAddGpuAssistColorRead()
{
	++g_DebugGpuAssistColorRead;
}

void DebugAddPlayerGpuAssistHit()
{

	++g_DebugPlayerGpuAssistHit;
}

void DebugAddPlayerGpuAssistFallback()
{

	++g_DebugPlayerGpuAssistFallback;
}

void DebugAddRenderSpriteCount()
{
	++g_DebugRenderSpriteCount;
}

void DebugAddRenderSpriteStateChange()
{
	++g_DebugRenderSpriteStateChanges;
}

void DebugAddRenderSpriteTextureChange()
{
	++g_DebugRenderSpriteTextureChanges;
}

void DebugAddRenderSpriteDrawCall()
{
	++g_DebugRenderSpriteDrawCalls;
}

void DebugAddCrowdFxCap(int cap)
{
	switch (cap)
	{
	case 0:
		++g_DebugCrowdFxCap0;
		break;
	case 1:
		++g_DebugCrowdFxCap1;
		break;
	case 2:
		++g_DebugCrowdFxCap2;
		break;
	case 3:
		++g_DebugCrowdFxCap3;
		break;
	case 4:
		++g_DebugCrowdFxCap4;
		break;
	}
}

bool RenderMainScene()
{
	if (EnableMainRender == false)
	{
		return false;
	}

	if ((LoadingWorld) > 30)
	{
		return false;
	}

	ResetRenderBreakdown();

	FogEnable = CameraFactorPtr->IsEnable();

	vec3_t pos;

	if (MoveMainCamera() == true)
	{
		VectorCopy(Hero->Object.StartPosition, pos);
	}
	else
	{
		g_pCatapultWindow->GetCameraPos(pos);

		if (g_Direction.IsDirection() && g_Direction.m_bDownHero == false)
		{
			g_Direction.GetCameraPosition(pos);
		}
	}

	int Width, Height;

	BYTE byWaterMap = 0;

	if (CameraTopViewEnable == false)
	{
#if MAIN_UPDATE > 303
		if (WindowWidth > 800 || (GMProtect->LookAndFeelType() == 3 || GMProtect->LookAndFeelType() == 4))
		{
			Height = GetWindowsY;
		}
		else
		{
			Height = GetWindowsY - 48;
		}
#else
		Height = GetWindowsY;
#endif // MAIN_UPDATE == 303
	}
	else
	{
		Height = GetWindowsY;
	}

#if MAIN_UPDATE > 303
	if (gmProtect->ScreenType == 0 && (GMProtect->LookAndFeelType() != 3 && GMProtect->LookAndFeelType() != 4))
	{
		Width = GetScreenWidth();
		Height = GetWindowsY - 48;
	}
	else
	{
		if (WindowWidth > 800 || (GMProtect->LookAndFeelType() == 3 || GMProtect->LookAndFeelType() == 4))
		{
			Width = GetWindowsX;
		}
		else
		{
			Width = GetScreenWidth();
		}
	}
#else
	Width = GetWindowsX;
#endif // MAIN_UPDATE == 303

	if (World == WD_0LORENCIA)
	{
		glClearColor(10 / 256.f, 20 / 256.f, 14 / 256.f, 1.f);
	}
	else if (World == WD_2DEVIAS)
	{
		glClearColor(0.f / 256.f, 0.f / 256.f, 10.f / 256.f, 1.f);
	}
	else if (World == WD_10HEAVEN)
	{
		glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
	}
	else if (gMapManager->InChaosCastle() == true)
	{
		glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
	}
	else if (World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6)
	{
		glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
	}
	else if (gMapManager->InHellas() == true)
	{
		byWaterMap = 1;
		glClearColor(0.f / 256.f, 0.f / 256.f, 0.f / 256.f, 1.f);
	}
	else
	{
		glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
	}

	BeginOpengl(0, 0, Width, Height, true);

	CreateFrustrum((float)Width / (float)GetScreenWidth(), pos);

	if (gMapManager->InBattleCastle())
	{
		if (battleCastle::InBattleCastle2(Hero->Object.Position))
		{
			vec3_t Color = { 0.f, 0.f, 0.f };
			battleCastle::StartFog(Color);
		}
		else
		{
			glDisable(GL_FOG);
		}
	}

	CreateScreenVector(MouseX, MouseY, MouseTarget);

	if (gwinhandle->CheckPerformance())
	{
		if (IsWaterTerrain() == false)
		{
			if (World == WD_39KANTURU_3RD)
			{
				if (!g_Direction.m_CKanturu.IsMayaScene())
					{ double t = DebugNowMs(); RenderTerrain(false); DebugAddMs(g_DebugTerrainMs, t); }
			}
			else if (World != WD_10HEAVEN && World != -1)
			{
				if (gMapManager->IsPKField() || IsDoppelGanger2())
				{
					{ double t = DebugNowMs(); RenderObjects(); DebugAddMs(g_DebugObjectMs, t); }
				}
				{ double t = DebugNowMs(); RenderTerrain(false); DebugAddMs(g_DebugTerrainMs, t); }
			}
		}

		if (!gMapManager->IsPKField() && !IsDoppelGanger2())
			{ double t = DebugNowMs(); RenderObjects(); DebugAddMs(g_DebugObjectMs, t); }

		{ double t = DebugNowMs(); RenderEffectShadows(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		{ double t = DebugNowMs(); RenderBoids(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }

		{ double t = DebugNowMs(); RenderCharactersClient(); DebugAddMs(g_DebugCharacterMs, t); DebugAddMs(g_DebugCharClientMs, t); }
		FlushGpuAssistShaderState();

		if (EditFlag != EDIT_NONE)
			{ double t = DebugNowMs(); RenderTerrain(true); DebugAddMs(g_DebugTerrainMs, t); }

		if (!CameraTopViewEnable)
			{ double t = DebugNowMs(); RenderItems(); DebugAddMs(g_DebugCharacterMs, t); DebugAddMs(g_DebugCharItemsMs, t); }

		{ double t = DebugNowMs(); RenderFishs(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		{ double t = DebugNowMs(); gGoboidManager->RenderBugs(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		{ double t = DebugNowMs(); RenderLeaves(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }

		if (!gMapManager->InChaosCastle())
			{ double t = DebugNowMs(); ThePetProcess().RenderPets(); DebugAddMs(g_DebugCharacterMs, t); DebugAddMs(g_DebugCharPetsMs, t); }

		{ double t = DebugNowMs(); RenderBoids(true); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		{ double t = DebugNowMs(); RenderObjects_AfterCharacter(); DebugAddMs(g_DebugObjectMs, t); }

		{ double t = DebugNowMs(); RenderJoints(byWaterMap); DebugAddEffectMs(g_DebugEffectJointMs, t); }
		{ double t = DebugNowMs(); RenderEffects(); DebugAddEffectMs(g_DebugEffectModelMs, t); }
		{ double t = DebugNowMs(); RenderBlurs(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		{ double t = DebugNowMs(); CheckSprites(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		BeginSprite();

		if ((World == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
			|| IsIceCity()
			|| IsSantaTown()
			|| gMapManager->IsPKField()
			|| IsDoppelGanger2()
			|| gMapManager->IsEmpireGuardian1()
			|| gMapManager->IsEmpireGuardian2()
			|| gMapManager->IsEmpireGuardian3()
			|| gMapManager->IsEmpireGuardian4()
			|| IsUnitedMarketPlace()
			)
		{
			{ double t = DebugNowMs(); RenderLeaves(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		}

		{ double t = DebugNowMs(); RenderSprites(); DebugAddEffectMs(g_DebugEffectSpriteMs, t); }
		{ double t = DebugNowMs(); RenderParticles(); DebugAddEffectMs(g_DebugEffectParticleMs, t); }

		if (IsWaterTerrain() == false)
		{
			{ double t = DebugNowMs(); RenderPoints(byWaterMap); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
		}

		EndSprite();

		{ double t = DebugNowMs(); RenderAfterEffects(); DebugAddEffectMs(g_DebugEffectAfterMs, t); }

		if (IsWaterTerrain() == true)
		{
			byWaterMap = 2;

			EndOpengl();
			BeginOpengl(0, 0, Width, Height, true);
			{ double t = DebugNowMs(); RenderWaterTerrain(); DebugAddMs(g_DebugTerrainMs, t); }
			{ double t = DebugNowMs(); RenderJoints(byWaterMap); DebugAddEffectMs(g_DebugEffectJointMs, t); }
			{ double t = DebugNowMs(); RenderEffects(true); DebugAddEffectMs(g_DebugEffectModelMs, t); }
			{ double t = DebugNowMs(); RenderBlurs(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
			{ double t = DebugNowMs(); CheckSprites(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }
			BeginSprite();

			if (World == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
				{ double t = DebugNowMs(); RenderLeaves(); DebugAddEffectMs(g_DebugEffectOtherMs, t); }

			{ double t = DebugNowMs(); RenderSprites(byWaterMap); DebugAddEffectMs(g_DebugEffectSpriteMs, t); }
			{ double t = DebugNowMs(); RenderParticles(byWaterMap); DebugAddEffectMs(g_DebugEffectParticleMs, t); }
			{ double t = DebugNowMs(); RenderPoints(byWaterMap); DebugAddEffectMs(g_DebugEffectOtherMs, t); }

			EndSprite();
			EndOpengl();
			BeginOpengl(0, 0, Width, Height, true);
		}

		if (gMapManager->InBattleCastle())
		{
			if (battleCastle::InBattleCastle2(Hero->Object.Position))
			{
				battleCastle::EndFog();
			}
		}
	}

	{ double t = DebugNowMs(); SelectObjects(); DebugAddMs(g_DebugUiMs, t); }
	BeginBitmap();
	{ double t = DebugNowMs(); RenderNotices(); DebugAddMs(g_DebugUiMs, t); }
	{ double t = DebugNowMs(); RenderObjectDescription(); DebugAddMs(g_DebugUiMs, t); }

	if (gwinhandle->CheckPerformance())
	{
		if (CameraTopViewEnable == false)
		{
			{ double t = DebugNowMs(); RenderInterface(true); DebugAddMs(g_DebugUiMs, t); }
		}
		{ double t = DebugNowMs(); RenderTournamentInterface(); DebugAddMs(g_DebugUiMs, t); }
		EndBitmap();

		{ double t = DebugNowMs(); g_pPartyManager->Render(); DebugAddMs(g_DebugUiMs, t); }
		{ double t = DebugNowMs(); g_pNewUISystem->Render(); DebugAddMs(g_DebugUiMs, t); }

		BeginBitmap();

		{ double t = DebugNowMs(); RenderInfomation(); DebugAddMs(g_DebugUiMs, t); }

#ifdef ENABLE_EDIT
		{ double t = DebugNowMs(); RenderDebugWindow(); DebugAddMs(g_DebugUiMs, t); }
#endif //ENABLE_EDIT

		EndBitmap();
		BeginBitmap();

		{ double t = DebugNowMs(); RenderCursor(); DebugAddMs(g_DebugUiMs, t); }

		EndBitmap();
		EndOpengl();
	}

	return true;
}

void MoveClientManager()
{
	if (SceneFlag == LOG_IN_SCENE && (InitLogIn && !CUIMng::Instance().m_CreditWin.IsShow()))
	{
		InitTerrainLight();
		MoveObjects();
		MoveEffects();
		MoveJoints();
		MoveParticles();
	}
	else if (SceneFlag == CHARACTER_SCENE && (CurrentProtocolState >= RECEIVE_CHARACTERS_LIST && InitCharacterScene))
	{
		InitTerrainLight();
		MoveObjects();
		MoveEffects();
		MoveJoints();
		MoveParticles();
	}
	else if (SceneFlag == MAIN_SCENE && (CurrentProtocolState == RECEIVE_JOIN_MAP_SERVER && EnableMainRender && InitMainScene))
	{
		InitTerrainLight();
		MoveObjects();
		MoveEffects();
		MoveJoints();
		MoveParticles();
	}
}

void MainScene(HDC hDC)
{
	const double debugFrameStart = DebugNowMs();
	const uintmax_t fixedDeltaTime = gsteady_clock->Getframe_per_second();

	static uintmax_t accumulatedTime = fixedDeltaTime;

	gsteady_clock->LoadInformationFps();

	const double debugMoveStart = DebugNowMs();

	//while (accumulatedTime >= fixedDeltaTime)
	{
		if(accumulatedTime >= fixedDeltaTime)
			accumulatedTime -= fixedDeltaTime;

		g_pNewKeyInput->ScanAsyncKeyState();

		if (LOG_IN_SCENE == SceneFlag || CHARACTER_SCENE == SceneFlag)
		{
			double dDeltaTick = g_pTimer->GetTimeElapsed();
			dDeltaTick = MIN(dDeltaTick, 200.0);
			g_pTimer->ResetTimer();

			CInput::Instance().Update();
			CUIMng::Instance().Update(dDeltaTick);
		}

		g_dwMouseUseUIID = 0;

		switch (SceneFlag)
		{
		case LOG_IN_SCENE:
			NewMoveLogInScene();
			break;
		case CHARACTER_SCENE:
			NewMoveCharacterScene();
			break;
		case MAIN_SCENE:
			MoveMainScene();
			break;
		}

		g_PhysicsManager.Move(0.005f);

		MoveNotices();

		if (PressKey(VK_SNAPSHOT))
		{
			if (GrabEnable)
				GrabEnable = false;
			else
				GrabEnable = true;
		}

		if (ChatTime > 0)
			ChatTime -= timefac(1.f);

		if (MacroTime > 0)
			MacroTime --;

		if (checkNormalizer)
		{
			WaterTextureNumber++;
			WaterTextureNumber %= 32;
		}

		MoveSceneFrame++;
		frame_scene_desplace++;
	}

	g_DebugMoveMs = DebugNowMs() - debugMoveStart;

	if (Destroy)
	{
		return;
	}

	const double debugRenderStart = DebugNowMs();

	Bitmaps.Manage();

	Set3DSoundPosition();

	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(GrabFileName, GMProtect->GetScreenPath(), st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
	char Text[256];
	sprintf(Text, GlobalText[459], GrabFileName);
	char lpszTemp[64];
	wsprintf(lpszTemp, " [%s / %s]", g_ServerListManager->GetSelectServerName(), Hero->ID);
	strcat(Text, lpszTemp);
	int iCaptureMode = 1;

	if (HIBYTE(GetAsyncKeyState(VK_SHIFT)))
	{
		iCaptureMode = 1 - iCaptureMode;
	}
	if (GrabEnable && iCaptureMode == 1)
	{
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
	}

	if (World == WD_10HEAVEN)
	{
		glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
	}
	else if (World == WD_73NEW_LOGIN_SCENE || World == WD_74NEW_CHARACTER_SCENE)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (gMapManager->InHellas())
	{
		glClearColor(30.f / 256.f, 40.f / 256.f, 40.f / 256.f, 1.f);
	}
	else if (gMapManager->InChaosCastle() == true)
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
	}
	else if (gMapManager->InBattleCastle() && battleCastle::InBattleCastle2(Hero->Object.Position))
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
	}
	else if (World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6)
	{
		glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
	}
	else if (World == WD_94NEW_LOGIN_SCENE)
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
	}
	else if (World == WD_51HOME_6TH_CHAR)
	{
		glClearColor(178.f / 256.f, 178.f / 256.f, 178.f / 256.f, 1.f);
	}
	else if (World == WD_65DOPPLEGANGER1)
	{
		glClearColor(148.f / 256.f, 179.f / 256.f, 223.f / 256.f, 1.f);
	}
	else
	{
		if (gmProtect->SceneLogin == 3 && SceneFlag == LOG_IN_SCENE)
		{
			glClearColor(178.f / 256.f, 178.f / 256.f, 178.f / 256.f, 1.f);
		}
		else if (gmProtect->SceneCharacter == 3 && SceneFlag == CHARACTER_SCENE)
		{
			glClearColor(0.f, 0.f, 0.f, 1.f);
		}
		else
		{
			glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	bool Success = false;

	auto thread_tick = gsteady_clock->GetthreadTime();

	switch (SceneFlag)
	{
	case LOG_IN_SCENE:
		Success = NewRenderLogInScene(hDC);
		break;
	case CHARACTER_SCENE:
		Success = NewRenderCharacterScene(hDC);
		break;
	case MAIN_SCENE:
		Success = RenderMainScene();
		break;
	}

	g_PhysicsManager.Render();

	g_DebugRenderMs = DebugNowMs() - debugRenderStart;

	if (GrabEnable)
	{
		SaveScreen();
	}

	if (GrabEnable && iCaptureMode == 0)
	{
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
	}
	GrabEnable = false;


	RenderFrameDebugOverlay();

	if (Success)
	{
		glFlush();
		SwapBuffers(hDC);
	}

	uintmax_t DifTimer = gsteady_clock->thread_sleep(thread_tick);

	g_DebugFrameMs = DebugNowMs() - debugFrameStart;

	accumulatedTime += DifTimer;

	if (EnableSocket && SceneFlag == MAIN_SCENE)
	{
		if (SocketClient.GetSocket() == INVALID_SOCKET)
		{
			if (g_pReconnectUI->GetReconnectStatus() != RECONNECT_STATUS_RECONNECT)
			{
				static BOOL s_bClosed = FALSE;

				if (!s_bClosed)
				{
					s_bClosed = TRUE;
					g_ErrorReport.Write("> Connection closed. ");
					g_ErrorReport.WriteCurrentTime();
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CServerLostMsgBoxLayout));
				}
			}
		}
	}

	if (SceneFlag == MAIN_SCENE)
	{
		switch (World)
		{
		case WD_0LORENCIA:
			if (HeroTile == 4)
			{
				StopBuffer(SOUND_WIND01, true);
				StopBuffer(SOUND_RAIN01, true);
			}
			else
			{
				PlayBuffer(SOUND_WIND01, NULL, true);
				if (RainCurrent > 0)
					PlayBuffer(SOUND_RAIN01, NULL, true);
			}
			break;
		case WD_1DUNGEON:
			PlayBuffer(SOUND_DUNGEON01, NULL, true);
			break;
		case WD_2DEVIAS:
			if (HeroTile == 3 || HeroTile >= 10)
				StopBuffer(SOUND_WIND01, true);
			else
				PlayBuffer(SOUND_WIND01, NULL, true);
			break;
		case WD_3NORIA:
			PlayBuffer(SOUND_WIND01, NULL, true);
			if (rand() % 512 == 0)
				PlayBuffer(SOUND_FOREST01);
			break;
		case WD_4LOSTTOWER:
			PlayBuffer(SOUND_TOWER01, NULL, true);
			break;
		case WD_5UNKNOWN:
			//PlayBuffer(SOUND_BOSS01,NULL,true);
			break;
		case WD_7ATLANSE:
			PlayBuffer(SOUND_WATER01, NULL, true);
			break;
		case WD_8TARKAN:
			PlayBuffer(SOUND_DESERT01, NULL, true);
			break;
		case WD_10HEAVEN:
			PlayBuffer(SOUND_HEAVEN01, NULL, true);
			if ((rand() % 100) == 0)
			{
				//PlayBuffer(SOUND_HEAVEN01);
			}
			else if ((rand() % 10) == 0)
			{
				//PlayBuffer(SOUND_THUNDERS02);
			}
			break;
		case WD_58ICECITY_BOSS:
			PlayBuffer(SOUND_WIND01, NULL, true);
			break;
		case WD_79UNITEDMARKETPLACE:
		{
			PlayBuffer(SOUND_WIND01, NULL, true);
			PlayBuffer(SOUND_RAIN01, NULL, true);
		}
		break;
		case WD_80KARUTAN1:
			PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
			break;
		case WD_81KARUTAN2:
			if (HeroTile == 12)
			{
				StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
				PlayBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, NULL, true);
			}
			else
			{
				StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
				PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
			}
			break;
		}
		if (World != WD_0LORENCIA && World != WD_2DEVIAS && World != WD_3NORIA && World != WD_58ICECITY_BOSS && World != WD_79UNITEDMARKETPLACE)
		{
			StopBuffer(SOUND_WIND01, true);
		}
		if (World != WD_0LORENCIA && gMapManager->InDevilSquare() == false && World != WD_79UNITEDMARKETPLACE)
		{
			StopBuffer(SOUND_RAIN01, true);
		}
		if (World != WD_1DUNGEON)
		{
			StopBuffer(SOUND_DUNGEON01, true);
		}
		if (World != WD_3NORIA)
		{
			StopBuffer(SOUND_FOREST01, true);
		}
		if (World != WD_4LOSTTOWER)
		{
			StopBuffer(SOUND_TOWER01, true);
		}
		if (World != WD_7ATLANSE)
		{
			StopBuffer(SOUND_WATER01, true);
		}
		if (World != WD_8TARKAN)
		{
			StopBuffer(SOUND_DESERT01, true);
		}
		if (World != WD_10HEAVEN)
		{
			StopBuffer(SOUND_HEAVEN01, true);
		}
		if (World != WD_51HOME_6TH_CHAR)
		{
			StopBuffer(SOUND_ELBELAND_VILLAGEPROTECTION01, true);
			StopBuffer(SOUND_ELBELAND_WATERFALLSMALL01, true);
			StopBuffer(SOUND_ELBELAND_WATERWAY01, true);
			StopBuffer(SOUND_ELBELAND_ENTERDEVIAS01, true);
			StopBuffer(SOUND_ELBELAND_WATERSMALL01, true);
			StopBuffer(SOUND_ELBELAND_RAVINE01, true);
			StopBuffer(SOUND_ELBELAND_ENTERATLANCE01, true);
		}

		if (!IsKarutanMap())
			StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
		if (World != WD_80KARUTAN1)
			StopBuffer(SOUND_KARUTAN_INSECT_ENV, true);
		if (World != WD_81KARUTAN2)
			StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);

		if (World == WD_0LORENCIA)
		{
			if (Hero->SafeZone)
			{
				if (HeroTile == 4)
					PlayMp3(g_lpszMp3[MUSIC_PUB]);
				else
					PlayMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
			}
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_PUB]);
			StopMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		}
		if (World == WD_2DEVIAS)
		{
			if (Hero->SafeZone)
			{
				if ((Hero->PositionX) >= 205 && (Hero->PositionX) <= 214 && (Hero->PositionY) >= 13 && (Hero->PositionY) <= 31)
				{
					PlayMp3(g_lpszMp3[MUSIC_CHURCH]);
				}
				else
				{
					PlayMp3(g_lpszMp3[MUSIC_DEVIAS]);
				}
			}
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_CHURCH]);
			StopMp3(g_lpszMp3[MUSIC_DEVIAS]);
		}
		if (World == WD_3NORIA)
		{
			if (Hero->SafeZone)
			{
				PlayMp3(g_lpszMp3[MUSIC_NORIA]);
			}
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_NORIA]);
		}
		if (World == WD_1DUNGEON || World == WD_5UNKNOWN)
		{
			PlayMp3(g_lpszMp3[MUSIC_DUNGEON]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_DUNGEON]);
		}

		if (World == WD_7ATLANSE)
		{
			PlayMp3(g_lpszMp3[MUSIC_ATLANS]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_ATLANS]);
		}
		if (World == WD_10HEAVEN)
		{
			PlayMp3(g_lpszMp3[MUSIC_ICARUS]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_ICARUS]);
		}
		if (World == WD_8TARKAN)
		{
			PlayMp3(g_lpszMp3[MUSIC_TARKAN]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_TARKAN]);
		}
		if (World == WD_4LOSTTOWER)
		{
			PlayMp3(g_lpszMp3[MUSIC_LOSTTOWER_A]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_LOSTTOWER_A]);
		}

		if (gMapManager->InHellas())
		{
			PlayMp3(g_lpszMp3[MUSIC_KALIMA]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_KALIMA]);
		}

		if (World == WD_31HUNTING_GROUND)
		{
			PlayMp3(g_lpszMp3[MUSIC_BC_HUNTINGGROUND]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_BC_HUNTINGGROUND]);
		}

		if (World == WD_33AIDA)
		{
			PlayMp3(g_lpszMp3[MUSIC_BC_ADIA]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_BC_ADIA]);
		}

		M34CryWolf1st::ChangeBackGroundMusic(World);
		M39Kanturu3rd::ChangeBackGroundMusic(World);

		if (World == WD_37KANTURU_1ST)
			PlayMp3(g_lpszMp3[MUSIC_KANTURU_1ST]);
		else
			StopMp3(g_lpszMp3[MUSIC_KANTURU_1ST]);

		M38Kanturu2nd::PlayBGM();
		SEASON3A::CGM3rdChangeUp::Instance().PlayBGM();
		if (gMapManager->IsCursedTemple())
		{
			g_CursedTemple->PlayBGM();
		}
		if (World == WD_51HOME_6TH_CHAR)
		{
			PlayMp3(g_lpszMp3[MUSIC_ELBELAND]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_ELBELAND]);
		}

		if (World == WD_56MAP_SWAMP_OF_QUIET)
		{
			PlayMp3(g_lpszMp3[MUSIC_SWAMP_OF_QUIET]);
		}
		else
		{
			StopMp3(g_lpszMp3[MUSIC_SWAMP_OF_QUIET]);
		}

		g_Raklion.PlayBGM();
		g_SantaTown.PlayBGM();
		g_PKField.PlayBGM();
		g_DoppelGanger1.PlayBGM();
		g_EmpireGuardian1.PlayBGM();
		g_EmpireGuardian2.PlayBGM();
		g_EmpireGuardian3.PlayBGM();
		g_EmpireGuardian4.PlayBGM();
		g_UnitedMarketPlace.PlayBGM();
		g_Karutan1.PlayBGM();

		if (World > WD_81KARUTAN2)
		{
			gMapManager->MapIndexPlayMP3();
		}
	}

	TimeRemain = accumulatedTime;
}

void Scene(HDC hDC)
{
	g_Luminosity = sinf(WorldTime * 0.004f) * 0.15f + 0.6f;

	switch (SceneFlag)
	{
#ifdef MOVIE_DIRECTSHOW
	case MOVIE_SCENE:
		MovieScene(hDC);
		break;
#endif // MOVIE_DIRECTSHOW
	case WEBZEN_SCENE:
		WebzenScene(hDC);
		break;
	case LOADING_SCENE:
		LoadingScene(hDC);
		break;
	case LOG_IN_SCENE:
	case CHARACTER_SCENE:
	case MAIN_SCENE:
		MainScene(hDC);
		break;
	}
}

bool GetTimeCheck(int DelayTime)
{
	int PresentTime = timeGetTime();

	if (g_bTimeCheck)
	{
		g_iBackupTime = PresentTime;
		g_bTimeCheck = false;
	}

	if (g_iBackupTime + DelayTime <= PresentTime)
	{
		g_bTimeCheck = true;
		return true;
	}
	return false;
}
