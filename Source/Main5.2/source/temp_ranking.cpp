		RequestServerRankingInfo(m_RankIndexCur);
	}
	else
	{
		m_RankMaxTop = Data->Value;
	}
}

void SEASON3B::CNewUIRankingTop::ReceiveRankingListInfo(BYTE* ReceiveBuffer)
{
	LPPHEADER_RANKING_LIST Data = (LPPHEADER_RANKING_LIST)ReceiveBuffer;

	m_RankList.clear();
	m_pScrollBar.SetPercent(0.0);

	this->m_RankIndexCur = Data->index;

	m_RankSelectIndex = -1;

	memset(this->m_RankName, 0, sizeof(this->m_RankName));

	strcpy_s(this->m_RankName, Data->rankname);

	strcpy_s(this->m_RankColum, Data->col2);

	int offset = sizeof(PHEADER_RANKING_LIST);

	for (int n = 0; n < Data->count; n++)
	{
		LPPCREATE_RANKING_INFO Data2 = (LPPCREATE_RANKING_INFO)(ReceiveBuffer + offset);

		BYTE baseClass = Data2->PlayerClass / 16;
		BYTE upgradeTier = Data2->PlayerClass % 16;
		BYTE Class = baseClass;
		if (upgradeTier == 1) Class |= 16; // 2nd Class
		else if (upgradeTier == 2) Class |= 32; // 3rd Class
		else if (upgradeTier >= 3) Class |= 64; // 4th Class

		m_RankList.push_back(TEMPLATE_RANKING(Data2->Name, gCharacterManager.GetCharacterClassText(Class), Class, Data2->LevelVip, Data2->TotalScore));

		offset += (sizeof(PCREATE_RANKING_INFO));
	}

	std::sort(m_RankList.begin(), m_RankList.end(), [](const TEMPLATE_RANKING& a, const TEMPLATE_RANKING& b) {
		return a.Score > b.Score; // Orden descendente
		});

	if (m_RankList.size() > 0)
	{
		m_RankSelectIndex = 0;
	}

	is_request = false;
}

void SEASON3B::CNewUIRankingTop::RequestServerRankingInfo(BYTE Index)
{
	if (is_request == false && (Index >= 0 && Index < m_RankMaxTop))
	{
		is_request = true;
		SendRequestRankingInfo(Index);
	}
}
