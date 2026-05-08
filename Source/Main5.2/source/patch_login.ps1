$path = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\LoginWin.cpp'
$text = [IO.File]::ReadAllText($path, [System.Text.Encoding]::Default)

$old1 = @"
	SAFE_DELETE(m_pPassInputBox);
	m_pPassInputBox = new CUITextInputBox;
	m_pPassInputBox->Init(gwinhandle->GethWnd(), 140, 14, MAX_PASSWORD_SIZE, TRUE);
	m_pPassInputBox->SetBackColor(0, 0, 0, 25);
	m_pPassInputBox->SetTextColor(255, 255, 230, 210);
	m_pPassInputBox->SetFont(g_hFixFont);
	m_pPassInputBox->SetState(UISTATE_NORMAL);
}
"@

$new1 = @"
	SAFE_DELETE(m_pPassInputBox);
	m_pPassInputBox = new CUITextInputBox;
	m_pPassInputBox->Init(gwinhandle->GethWnd(), 140, 14, MAX_PASSWORD_SIZE, TRUE);
	m_pPassInputBox->SetBackColor(0, 0, 0, 25);
	m_pPassInputBox->SetTextColor(255, 255, 230, 210);
	m_pPassInputBox->SetFont(g_hFixFont);
	m_pPassInputBox->SetState(UISTATE_NORMAL);

	if (gmProtect->remember_account && !stAccountMacro.empty())
	{
		m_pIDInputBox->SetText(stAccountMacro[0].NameId.c_str());
		m_pPassInputBox->SetText(stAccountMacro[0].PassID.c_str());
	}
}
"@

$text = $text.Replace($old1, $new1)

$old2 = @"
		for (auto& account : stAccountMacro)
		{
			if (account.NameId == NameId)
			{
				account.PassID = PassID;
				accountFound = true;
				break;
			}
		}
"@

$new2 = @"
		for (auto it = stAccountMacro.begin(); it != stAccountMacro.end(); ++it)
		{
			if (it->NameId == NameId)
			{
				it->PassID = PassID;
				LOGIN_ACCOUNT_REG temp = *it;
				stAccountMacro.erase(it);
				stAccountMacro.insert(stAccountMacro.begin(), temp);
				accountFound = true;
				break;
			}
		}
"@

$text = $text.Replace($old2, $new2)

$old3 = @"
		if (!accountFound)
		{
			if (stAccountMacro.size() < MAX_ACCOUNT_REG)
			{
				stAccountMacro.push_back({ NameId, PassID });
			}
		}
"@

$new3 = @"
		if (!accountFound)
		{
			if (stAccountMacro.size() >= MAX_ACCOUNT_REG)
			{
				stAccountMacro.pop_back();
			}
			stAccountMacro.insert(stAccountMacro.begin(), { NameId, PassID });
		}
"@

$text = $text.Replace($old3, $new3)

[IO.File]::WriteAllText($path, $text, [System.Text.Encoding]::Default)


$pathCB = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\CB_DangKyInGame.cpp'
$textCB = [IO.File]::ReadAllText($pathCB, [System.Text.Encoding]::Default)

$oldCB = @"
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.m_LoginWin.GetIDInputBox()->SetText(szID);
		rUIMng.m_LoginWin.GetPassInputBox()->SetText(szPass);
	}
	break;
"@

$newCB = @"
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.m_LoginWin.GetIDInputBox()->SetText(szID);
		rUIMng.m_LoginWin.GetPassInputBox()->SetText(szPass);

		if (gInterface->Data[eWindow_DangKyInGame].OnShow)
		{
			gInterface->Data[eWindow_DangKyInGame].OnShow = 0;
		}
		if (gmProtect->remember_account)
		{
			rUIMng.m_LoginWin.SaveAccount(szID, szPass);
		}
	}
	break;
"@

$textCB = $textCB.Replace($oldCB, $newCB)

[IO.File]::WriteAllText($pathCB, $textCB, [System.Text.Encoding]::Default)
