#ifndef _TextClient_H
#define _TextClient_H

class cTextClient
{
public:
	void Load();

	char txtClient_ThongTin[50][255];
	char txtClient_Khac[50][255];

	char txtClient_BotMix[50][120];
	char txtClient_KetNoi[50][120];
	char txtClient_JewelBank[50][120];
	char txtClient_Ranking[50][120];
	char txtClient_ViewCharInfo[50][120];
	char txtClient_ChangeItem[50][120];
	char txtClient_MenuUser[50][120];
	char txtClient_VQMM[50][256];
	char txtClient_Donate[50][256];
	char txtClient_ChoTroi[50][256];

#if(CongHuongV2)	
	char TextVN_CongHuongTrangBi[50][120];
#endif

};
//===================================================

#define MENU_GAME_FILE "./Data/Custom/TextClient/TextClient.ini"


extern cTextClient gTextClient;

#endif
