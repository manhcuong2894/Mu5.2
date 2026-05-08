#include "StdAfx.h"
#include "Util.h"
#include "CEventName.h"

CEventName  gEventName;

CEventName::CEventName() : m_EventGlobal(MAX_SIZE_EVENT_TEMPLATE),
m_InvasionGlobal(MAX_SIZE_EVENT_TEMPLATE),
m_ArenaGlobal(MAX_SIZE_EVENT_TEMPLATE)
{
	m_EventTypeCount = 0;
	memset(m_EventTypeName, 0, sizeof(m_EventTypeName));
}

CEventName::~CEventName()
{
	m_EventGlobal.clear();
	m_ArenaGlobal.clear();
	m_InvasionGlobal.clear();
}

void CEventName::Release()
{
	// Reset in-place instead of clear()+resize() to avoid race condition
	// Other threads (DevilSquare, BloodCastle, etc.) access these vectors by index every second
	// clear() makes size=0 momentarily -> ACCESS_VIOLATION crash
	for (int i = 0; i < MAX_SIZE_EVENT_TEMPLATE; i++)
	{
		m_EventGlobal[i] = TEMPLATE_EVENT();
		m_ArenaGlobal[i] = TEMPLATE_EVENT();
		m_InvasionGlobal[i] = TEMPLATE_EVENT();
	}
}

void CEventName::OpenFile(std::string filename)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(filename.c_str());

	if (res.status != pugi::status_ok)
	{
		ErrorMessageBox("Error load fail: %s", filename.c_str());
		return;
	}

	this->Release();

	// Build into temp vector first, then swap to minimize race condition window
	std::vector<TEMPLATE_EVENT> tempClientData;
	pugi::xml_node root = file.child("EventList");

	// Parse EventType names and count from XML
	m_EventTypeCount = 0;
	memset(m_EventTypeName, 0, sizeof(m_EventTypeName));

	for (pugi::xml_node leaf = root.child("EventType"); leaf; leaf = leaf.next_sibling())
	{
		int switch_on = leaf.attribute("Index").as_int();
		const char* typeName = leaf.attribute("Name").as_string("Unknown");

		// Store EventType name
		if (switch_on >= 0 && switch_on < MAX_EVENT_TYPES)
		{
			strncpy(m_EventTypeName[switch_on], typeName, 31);
			m_EventTypeName[switch_on][31] = 0;
			if (switch_on >= m_EventTypeCount)
				m_EventTypeCount = switch_on + 1;
		}

		for (pugi::xml_node Element = leaf.child("info"); Element; Element = Element.next_sibling())
		{
			TEMPLATE_EVENT info;

			info.switch_on = switch_on;

			info.m_Key = Element.attribute("Key").as_int();

			info.m_Name = Element.attribute("Name").as_string();

			info.m_Des = Element.attribute("DesString").as_string();

			info.m_Gate = Element.attribute("NumberGate").as_int(-1);

			this->push_back(switch_on, info);
			tempClientData.push_back(info);
		}
	}

	// Swap at once instead of clear() + push_back() one by one
	this->m_SendClientDataEventTime.swap(tempClientData);
}

void CEventName::push_back(int switch_on, const TEMPLATE_EVENT& info)
{
	if (info.m_Key >= MAX_SIZE_EVENT_TEMPLATE)
		return;

	switch (switch_on)
	{
	case 0:
		m_EventGlobal[info.m_Key] = (info);
		break;
	case 1:
		m_InvasionGlobal[info.m_Key] = (info);
		break;
	case 2:
		m_ArenaGlobal[info.m_Key] = (info);
		break;
	default:
		break;
	}
}
int CEventName::GetTimeEventSwitch(int switch_on, int Index)
{
	int RemainTime = -1;

	switch (switch_on)
	{
	case 0:
		RemainTime = this->GlobalRemainTime(Index);
		break;
	case 1:
		RemainTime = this->InvasionRemainTime(Index);
		break;
	case 2:
		RemainTime = this->ArenaRemainTime(Index);
		break;
	default:
		break;
	}
	return RemainTime;
}

bool CEventName::IsArena(int Index)
{
	return (Index < MAX_SIZE_EVENT_TEMPLATE && m_ArenaGlobal[Index].m_Key != -1);
}

bool CEventName::IsGlobal(int Index)
{
	return (Index < MAX_SIZE_EVENT_TEMPLATE && m_EventGlobal[Index].m_Key != -1);
}

bool CEventName::IsInvasion(int Index)
{
	return (Index < MAX_SIZE_EVENT_TEMPLATE && m_InvasionGlobal[Index].m_Key != -1);
}

const char* CEventName::ArenaName(int Index)
{
	if (Index < MAX_SIZE_EVENT_TEMPLATE)
	{
		return m_ArenaGlobal[Index].GetName();
	}

	return "No Name";
}

const char* CEventName::GlobalName(int Index)
{
	if (Index < MAX_SIZE_EVENT_TEMPLATE)
	{
		return m_EventGlobal[Index].GetName();
	}

	return "No Name";
}

const char* CEventName::InvasionName(int Index)
{
	if (Index < MAX_SIZE_EVENT_TEMPLATE)
	{
		return m_InvasionGlobal[Index].GetName();
	}

	return "No Name";
}

int CEventName::ArenaRemainTime(int Index)
{
	int RemainTime = -1;

	if (Index < MAX_SIZE_EVENT_TEMPLATE && m_ArenaGlobal[Index].m_Key != -1)
	{
		RemainTime = m_ArenaGlobal[Index].m_RemainTime;
	}

	return RemainTime;
}

int CEventName::GlobalRemainTime(int Index)
{
	int RemainTime = -1;

	if (Index < MAX_SIZE_EVENT_TEMPLATE && m_EventGlobal[Index].m_Key != -1)
	{
		RemainTime = m_EventGlobal[Index].m_RemainTime;
	}

	return RemainTime;
}

int CEventName::InvasionRemainTime(int Index)
{
	int RemainTime = -1;

	if (Index < MAX_SIZE_EVENT_TEMPLATE && m_InvasionGlobal[Index].m_Key != -1)
	{
		RemainTime = m_InvasionGlobal[Index].m_RemainTime;
	}

	return RemainTime;
}

void CEventName::ArenaRemainTime(int Index, int Time)
{
	if (Index < MAX_SIZE_EVENT_TEMPLATE)
	{
		m_ArenaGlobal[Index].m_RemainTime = Time;
	}
}

void CEventName::GlobalRemainTime(int Index, int Time)
{
	if (Index < MAX_SIZE_EVENT_TEMPLATE)
	{
		m_EventGlobal[Index].m_RemainTime = Time;
	}
}

void CEventName::InvasionRemainTime(int Index, int Time)
{
	if (Index < MAX_SIZE_EVENT_TEMPLATE)
	{
		m_InvasionGlobal[Index].m_RemainTime = Time;
	}
}

const char* CEventName::GetEventTypeName(int typeIndex)
{
	if (typeIndex >= 0 && typeIndex < MAX_EVENT_TYPES)
	{
		return m_EventTypeName[typeIndex];
	}
	return "Unknown";
}

int CEventName::GetEventTypeCount()
{
	return m_EventTypeCount;
}
