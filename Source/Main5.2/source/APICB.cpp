#include "StdAfx.h"
#include "APICB.h"
#include "Protocol.h"
#include "ZzzInfomation.h"
#include "ZzzScene.h"
//#include "PrintPlayer.h"
//#include "HackCheck.h"

namespace
{
	bool APICBInitialized = false;
	DWORD FrameValue = 0;
	DWORD SpeedValue = 0;
	DWORD MainTickCount = 0;
	DWORD SyncTickCount = 0;
	DWORD CountModifier = 0;
	DWORD DelayModifier = 0;
	DWORD HasteModifier = 0;
	DWORD SleepModifier = 0;
	DWORD SpeedModifier1 = 0;
	DWORD SpeedModifier2 = 0;
	DWORD ModelModifier1 = 0;
	DWORD ModelModifier2 = 0;
	DWORD ModelModifier3 = 0;
	DWORD ViewPoint = 0;
	DWORD ViewStrength = 0;
	DWORD ViewDexterity = 0;
	DWORD ViewVitality = 0;
	DWORD ViewEnergy = 0;
	DWORD ViewLeadership = 0;
	DWORD ViewAddStrength = 0;
	DWORD ViewAddDexterity = 0;
	DWORD ViewAddVitality = 0;
	DWORD ViewAddEnergy = 0;
	DWORD ViewAddLeadership = 0;
	DWORD ViewPhysiSpeed = 0;
	DWORD ViewMagicSpeed = 0;

	void UpdateAPICBRuntimeValues()
	{
		MainTickCount = GetTickCount();
		SyncTickCount = MainTickCount;
		FrameValue++;

		if (CharacterAttribute == 0)
		{
			return;
		}

		ViewPoint = CharacterAttribute->LevelUpPoint;
		ViewStrength = CharacterAttribute->Strength;
		ViewDexterity = CharacterAttribute->Dexterity;
		ViewVitality = CharacterAttribute->Vitality;
		ViewEnergy = CharacterAttribute->Energy;
		ViewLeadership = CharacterAttribute->Charisma;
		ViewAddStrength = CharacterAttribute->AddStrength;
		ViewAddDexterity = CharacterAttribute->AddDexterity;
		ViewAddVitality = CharacterAttribute->AddVitality;
		ViewAddEnergy = CharacterAttribute->AddEnergy;
		ViewAddLeadership = CharacterAttribute->AddCharisma;
		ViewPhysiSpeed = CharacterAttribute->AttackSpeed;
		ViewMagicSpeed = CharacterAttribute->MagicSpeed;
	}

	void BindAPICBRuntimeValues()
	{
		API_FrameValue = &FrameValue;
		API_SpeedValue = &SpeedValue;
		API_MainTickCount = &MainTickCount;
		API_SyncTickCount = &SyncTickCount;
		API_CountModifier = &CountModifier;
		API_DelayModifier = &DelayModifier;
		API_HasteModifier = &HasteModifier;
		API_SleepModifier = &SleepModifier;
		API_SpeedModifier1 = &SpeedModifier1;
		API_SpeedModifier2 = &SpeedModifier2;
		API_ModelModifier1 = &ModelModifier1;
		API_ModelModifier2 = &ModelModifier2;
		API_ModelModifier3 = &ModelModifier3;
		API_ViewPoint = &ViewPoint;
		API_ViewStrength = &ViewStrength;
		API_ViewDexterity = &ViewDexterity;
		API_ViewVitality = &ViewVitality;
		API_ViewEnergy = &ViewEnergy;
		API_ViewLeadership = &ViewLeadership;
		API_ViewAddStrength = &ViewAddStrength;
		API_ViewAddDexterity = &ViewAddDexterity;
		API_ViewAddVitality = &ViewAddVitality;
		API_ViewAddEnergy = &ViewAddEnergy;
		API_ViewAddLeadership = &ViewAddLeadership;
		API_ViewPhysiSpeed = &ViewPhysiSpeed;
		API_ViewMagicSpeed = &ViewMagicSpeed;
	}
}


APICB gAPICB;
void CBDataSendCallback(BYTE* lpMsg, DWORD size)
{
	DataSend(lpMsg, size);
}
APICB::APICB()
{
	SetAPIDATA_SEND(CBDataSendCallback);
}


APICB::~APICB()
{
}
void APICB::Work()
{
	UpdateAPICBRuntimeValues();
	if (SceneFlag != MAIN_SCENE || CharacterAttribute == 0)
	{
		return;
	}

	if (APICBInitialized == false)
	{
		CBAnihack_Init();
		APICBInitialized = true;
	}

	CBAnihack_Work();
}
void APICB::Init()
{
	UpdateAPICBRuntimeValues();
	BindAPICBRuntimeValues();
	APICBInitialized = false;
}
void APICB::Recv(BYTE* Recv)
{
	UpdateAPICBRuntimeValues();
	CBAnihack_Recv(Recv);
}
void APICB::Attack()
{
	UpdateAPICBRuntimeValues();
	CBAnihack_Attack();
}
