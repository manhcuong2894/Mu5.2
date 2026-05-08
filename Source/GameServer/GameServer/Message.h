// Message.h: interface for the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct MESSAGE_INFO
{
	int Index;
	char Text[128];
};

struct MESSAGE_GROUP_INFO
{
	int index;
	std::map<int, MESSAGE_INFO> Message;
};

class CMessage
{
public:
	CMessage();
	virtual ~CMessage();
	void Load(char* path);
	char* GlobalText(int index);

	char* GetMessage(int Index, int ID);
private:
	char m_DefaultMessage[128];
	std::map<int,MESSAGE_INFO> m_MessageInfo;

	std::map<int, MESSAGE_GROUP_INFO> m_MessageGrInfo;
};

extern CMessage gMessage;
