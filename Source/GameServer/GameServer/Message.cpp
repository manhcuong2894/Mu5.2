// Message.cpp: implementation of the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Message.h"
#include "MemScript.h"
#include "Util.h"

CMessage gMessage;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessage::CMessage() // OK
{
	memset(this->m_DefaultMessage,0,sizeof(this->m_DefaultMessage));

	this->m_MessageInfo.clear();
}

CMessage::~CMessage() // OK
{

}

void CMessage::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_MessageInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			MESSAGE_INFO info;

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Text,lpMemScript->GetAsString());

			this->m_MessageInfo.insert(std::pair<int,MESSAGE_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

char* CMessage::GlobalText(int index) // OK
{
	std::map<int,MESSAGE_INFO>::iterator it = this->m_MessageInfo.find(index);

	if(it == this->m_MessageInfo.end())
	{
		wsprintf(this->m_DefaultMessage,"Could not find message %d!",index);
		return this->m_DefaultMessage;
	}
	else
	{
		return it->second.Text;
	}
}

char* CMessage::GetMessage(int Index, int ID) // OK
{
	char Default[50];

	wsprintf(Default, "Message %d-%d not found!", Index, ID);

	std::map<int, MESSAGE_GROUP_INFO>::iterator img = this->m_MessageGrInfo.find(Index);

	if (img != this->m_MessageGrInfo.end())
	{
		std::map<int, MESSAGE_INFO>::iterator im = img->second.Message.find(ID);

		if (im != img->second.Message.end())
		{
			return im->second.Text;
		}
	}

	return Default;
}
