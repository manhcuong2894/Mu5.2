#include "stdafx.h"
#include "CustomBow.h"

CCustomBow gCustomBow;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomBow::CCustomBow() // OK
{
	this->Clear();
}

CCustomBow::~CCustomBow() // OK
{

}

DWORD SkillBow(DWORD BowItem)
{
	return gCustomBow.GetCustomSkill(BowItem);
}

void CCustomBow::Clear()
{
	this->m_CustomBowInfo.clear();
}

void CCustomBow::Load(const std::vector<CUSTOM_BOW_INFO>& info) // OK
{
	this->Clear();

	for (size_t n = 0; n < info.size(); n++)
	{
		if (info[n].Index < 0 || info[n].Index >= MAX_CUSTOM_BOW)
		{
			continue;
		}

		this->m_CustomBowInfo.insert(std::pair<int, CUSTOM_BOW_INFO>(info[n].ItemIndex, info[n]));
	}
}

CUSTOM_BOW_INFO* CCustomBow::GetInfoByItem(int ItemIndex) // OK
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it != this->m_CustomBowInfo.end())
	{
		return &it->second;
	}

	return 0;
}

bool CCustomBow::CheckCustomBow(int ItemIndex) // OK
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it != this->m_CustomBowInfo.end())
	{
		return it->second.ItemType == 0;
	}

	return false;
}

bool CCustomBow::CheckCustomCross(int ItemIndex) // OK
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it != this->m_CustomBowInfo.end())
	{
		return it->second.ItemType == 1;
	}

	return false;
}

int CCustomBow::GetCustomSkill(int ItemIndex) // OK
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if (it != this->m_CustomBowInfo.end())
	{
		return it->second.SkillShot;
	}
	return -1;
}
