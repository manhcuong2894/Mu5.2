#include "stdafx.h"
#include "NewUISystem.h"
#include "CGMJewelOfAction.h"


CGMJewelOfAction::CGMJewelOfAction()
{
}

CGMJewelOfAction::~CGMJewelOfAction()
{
	this->pack_of_jewel.clear();
}

void CGMJewelOfAction::ReadData(FILE* fp)
{
	readVector(fp, this->pack_of_jewel);
}

void CGMJewelOfAction::LoadItemModel()
{
	for (size_t i = 0; i < pack_of_jewel.size(); i++)
	{
		int modelId = pack_of_jewel[i].ItemIndex + MODEL_ITEM;

		if (pack_of_jewel[i].ModelName[0] != '\0')
		{
			gmClientModels->AccessModel(modelId, "Data\\Item\\", (pack_of_jewel[i].ModelName));
			gmClientModels->OpenTexture(modelId, "Item\\Texture\\", GL_REPEAT, GL_NEAREST);
		}
	}
}

__int8 CGMJewelOfAction::GetOptCount(int Option)
{
	return 0;
}

__int8 CGMJewelOfAction::GetExcCount(int Option)
{
	int count = 0;
	int maskedOption = Option & 0x3F;  // Limita a los primeros 6 bits

	while (maskedOption)
	{
		maskedOption &= (maskedOption - 1);  // Apaga el bit menos significativo que est� encendido
		count++;
	}
	return count;
}

bool CGMJewelOfAction::CheckOfJewelByItem(int nType)
{
	return (bool)(this->find(nType) != NULL);
}

bool CGMJewelOfAction::CheckOfJewelReinforce(int sorceIndex, const ITEM* pTargetItem)
{
	const CUSTOM_JEWEL_INFO* pack = this->find(sorceIndex);

	if (pack)
	{
		int targetIndex = pTargetItem->Type;
		int ItemLevel = ((pTargetItem->Level >> 3) & 0xF);
		int excelItem = GetExcCount(pTargetItem->Option1);
		int socketItem = pTargetItem->SocketCount;
		int Option3 = ((pTargetItem->Level) & 3) + ((pTargetItem->Option1) & 64) / 64 * 4;

		if ((pack->EnableSlotWeapon == 0 && (targetIndex >= ITEM_SWORD && targetIndex < ITEM_HELM))
			|| (pack->EnableSlotArmor == 0 && (targetIndex >= ITEM_HELM && targetIndex < ITEM_WING))
			|| (pack->EnableSlotWing == 0 && g_pNewItemTooltip->IsEnableOptionWing(targetIndex))
			)
		{
			return false;
		}

		if ((pack->MinItemLevel != -1 && !(ItemLevel >= pack->MinItemLevel)) || (pack->MaxItemLevel != -1 && !(ItemLevel <= pack->MaxItemLevel)))
		{
			return false;
		}

		if ((pack->MinItemOption3 != -1 && !(Option3 >= pack->MinItemOption3)) || (pack->MaxItemOption3 != -1 && !(Option3 <= pack->MaxItemOption3)))
		{
			return false;
		}

		if ((pack->MinItemNewOption != -1 && !(excelItem >= pack->MinItemNewOption)) || (pack->MaxItemNewOption != -1 && !(excelItem <= pack->MaxItemNewOption)))
		{
			return false;
		}

		if ((pack->MinItemSocketOption != -1 && !(socketItem >= pack->MinItemSocketOption)) || (pack->MaxItemSocketOption != -1 && !(socketItem <= pack->MaxItemSocketOption)))
		{
			return false;
		}

		return true;
	}

	return false;
}

const CUSTOM_JEWEL_INFO* CGMJewelOfAction::find(int nType)
{
	auto it = std::find_if(pack_of_jewel.begin(), pack_of_jewel.end(), [&nType](const CUSTOM_JEWEL_INFO& value) {
		return value.ItemIndex == nType;
		});

	return (it != pack_of_jewel.end()) ? &(*it) : NULL;
}

CGMJewelOfAction* CGMJewelOfAction::Instance()
{
	static CGMJewelOfAction s_Instance;
	return &s_Instance;
}