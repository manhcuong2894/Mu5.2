import os

def patch_bytes(filepath, replacements):
    with open(filepath, 'rb') as f:
        content = f.read()
    
    for old_str, new_str in replacements:
        old_b = old_str.encode('windows-1252', errors='ignore')
        new_b = new_str.encode('windows-1252', errors='ignore')
        
        # Fallback to precise byte sequences if encoding shifts
        if old_b in content:
            content = content.replace(old_b, new_b)
            print(f'Successfully replaced in {filepath}')
        else:
            print(f'Could not find target in {filepath}')
            
    with open(filepath, 'wb') as f:
        f.write(content)

lw = r'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\LoginWin.cpp'
# LoginWin.cpp create() patch
lw_old1 = '''	SAFE_DELETE(m_pPassInputBox);
	m_pPassInputBox = new CUITextInputBox;
	m_pPassInputBox->Init(gwinhandle->GethWnd(), 140, 14, MAX_PASSWORD_SIZE, TRUE);
	m_pPassInputBox->SetBackColor(0, 0, 0, 25);
	m_pPassInputBox->SetTextColor(255, 255, 230, 210);
	m_pPassInputBox->SetFont(g_hFixFont);
	m_pPassInputBox->SetState(UISTATE_NORMAL);
}'''

lw_new1 = '''	SAFE_DELETE(m_pPassInputBox);
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
}'''

# LoginWin.cpp SaveAccount() patch
lw_old2 = '''void CLoginWin::SaveAccount(std::string NameId, std::string PassID)
{
	if (gmProtect->remember_account)
	{
		bool accountFound = false;

		for (auto& account : stAccountMacro)
		{
			if (account.NameId == NameId)
			{
				account.PassID = PassID;
				accountFound = true;
				break;
			}
		}

		// Si no se encontr, agregamos la nueva cuenta
		if (!accountFound)
		{
			if (stAccountMacro.size() < MAX_ACCOUNT_REG)
			{
				stAccountMacro.push_back({ NameId, PassID });
			}
		}

		ReloadAccount(false, true, true);
	}
}'''

lw_new2 = '''void CLoginWin::SaveAccount(std::string NameId, std::string PassID)
{
	if (gmProtect->remember_account)
	{
		bool accountFound = false;

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

		if (!accountFound)
		{
			if (stAccountMacro.size() >= MAX_ACCOUNT_REG)
			{
				stAccountMacro.pop_back();
			}
			stAccountMacro.insert(stAccountMacro.begin(), { NameId, PassID });
		}

		ReloadAccount(false, true, true);
	}
}'''

patch_bytes(lw, [(lw_old1, lw_new1)])

# Trying to patch without the Spanish string to avoid encoding
lw_old2_alt = '''		for (auto& account : stAccountMacro)
		{
			if (account.NameId == NameId)
			{
				account.PassID = PassID;
				accountFound = true;
				break;
			}
		}'''

lw_new2_alt = '''		for (auto it = stAccountMacro.begin(); it != stAccountMacro.end(); ++it)
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
		}'''

lw_old3_alt = '''		if (!accountFound)
		{
			if (stAccountMacro.size() < MAX_ACCOUNT_REG)
			{
				stAccountMacro.push_back({ NameId, PassID });
			}
		}'''

lw_new3_alt = '''		if (!accountFound)
		{
			if (stAccountMacro.size() >= MAX_ACCOUNT_REG)
			{
				stAccountMacro.pop_back();
			}
			stAccountMacro.insert(stAccountMacro.begin(), { NameId, PassID });
		}'''

patch_bytes(lw, [(lw_old2_alt, lw_new2_alt), (lw_old3_alt, lw_new3_alt)])


cb = r'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\CB_DangKyInGame.cpp'

cb_old = '''		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.m_LoginWin.GetIDInputBox()->SetText(szID);
		rUIMng.m_LoginWin.GetPassInputBox()->SetText(szPass);
	}
	break;'''

cb_new = '''		CUIMng& rUIMng = CUIMng::Instance();
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
	break;'''

patch_bytes(cb, [(cb_old, cb_new)])

