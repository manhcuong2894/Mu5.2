#include "StdAfx.h"
#include "TextClient.h"
#include "Util.h"
#include <stdlib.h>
#include "windows.h"
//#include "atlstr.h"
#include "Interface.h"
#include "stdio.h"
#include "./Utilities/Log/muConsoleDebug.h"

#include "zzzinventory.h"
#include "dsplaysound.h"
#include "CSItemOption.h"

cTextClient gTextClient;

void cTextClient::Load()
{
	char strFileName[255];
	memset(strFileName, 0, sizeof(strFileName));
	sprintf(strFileName, "./Data/Custom/TextClient/TextClient_%s.ini", g_strSelectedML.c_str());

	char GetFotText[35] = { 0 };
	for (int st = 0; st < 50; st++)
	{
		wsprintf(GetFotText, "Text%d", st);
		GetPrivateProfileStringA("ThongTin",		GetFotText, "Null",		txtClient_ThongTin[st],			sizeof(txtClient_ThongTin[st]),			strFileName);
		GetPrivateProfileStringA("Khac",			GetFotText, "Null",		txtClient_Khac[st],				sizeof(txtClient_Khac[st]),				strFileName);
		GetPrivateProfileStringA("Ketnoi",			GetFotText, "Null",		txtClient_KetNoi[st],			sizeof(txtClient_KetNoi[st]),			strFileName);
		GetPrivateProfileStringA("BotMix",			GetFotText, "Null",		txtClient_BotMix[st],			sizeof(txtClient_BotMix[st]),			strFileName);
		GetPrivateProfileStringA("JewelBank",		GetFotText, "Null",		txtClient_JewelBank[st],		sizeof(txtClient_JewelBank[st]),		strFileName);
		GetPrivateProfileStringA("Ranking",			GetFotText, "Null",		txtClient_Ranking[st],			sizeof(txtClient_Ranking[st]),			strFileName);
		GetPrivateProfileStringA("ViewCharInfo",	GetFotText, "Null",		txtClient_ViewCharInfo[st],		sizeof(txtClient_ViewCharInfo[st]),		strFileName);
		GetPrivateProfileStringA("ChangeItem",		GetFotText, "Null",		txtClient_ChangeItem[st],		sizeof(txtClient_ChangeItem[st]),		strFileName);
		GetPrivateProfileStringA("MenuUser",		GetFotText, "Null",		txtClient_MenuUser[st],			sizeof(txtClient_MenuUser[st]),			strFileName);
		GetPrivateProfileStringA("VQMM",			GetFotText, "Null",		txtClient_VQMM[st],				sizeof(txtClient_VQMM[st]),				strFileName);
		GetPrivateProfileStringA("Donate",			GetFotText, "Null",		txtClient_Donate[st],			sizeof(txtClient_Donate[st]),			strFileName);
		GetPrivateProfileStringA("ChoTroi",			GetFotText, "Null",		txtClient_ChoTroi[st],			sizeof(txtClient_ChoTroi[st]),			strFileName);

		g_ConsoleDebug->Write(5, "[Load text client: %d: %s]", st, txtClient_ThongTin[st]);
	}

	

}