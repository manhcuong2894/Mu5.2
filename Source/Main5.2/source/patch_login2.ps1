$path = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\LoginWin.cpp'
$text = [IO.File]::ReadAllText($path, [System.Text.Encoding]::Default)

$inject1 = "
	if (gmProtect->remember_account && !stAccountMacro.empty())
	{
		m_pIDInputBox->SetText(stAccountMacro[0].NameId.c_str());
		m_pPassInputBox->SetText(stAccountMacro[0].PassID.c_str());
	}
}
"
$text = $text -replace 'FirstLoad = TRUE;\s*}', "FirstLoad = TRUE;`r`n$inject1"

$old2 = '(?s)for \(auto& account : stAccountMacro\).*?accountFound = true;\s+break;\s+}\s+}'
$new2 = 'for (auto it = stAccountMacro.begin(); it != stAccountMacro.end(); ++it)
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
		}'
$text = [regex]::Replace($text, $old2, $new2)

$old3 = '(?s)if \(\!accountFound\)\s*{\s*if \(stAccountMacro\.size\(\) < MAX_ACCOUNT_REG\)\s*{\s*stAccountMacro\.push_back\(\{ NameId, PassID \}\);\s*}\s*}'
$new3 = 'if (!accountFound)
		{
			if (stAccountMacro.size() >= MAX_ACCOUNT_REG)
			{
				stAccountMacro.pop_back();
			}
			stAccountMacro.insert(stAccountMacro.begin(), { NameId, PassID });
		}'
$text = [regex]::Replace($text, $old3, $new3)

[IO.File]::WriteAllText($path, $text, [System.Text.Encoding]::Default)


$pathCB = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\CB_DangKyInGame.cpp'
$textCB = [IO.File]::ReadAllText($pathCB, [System.Text.Encoding]::Default)

$oldCB = '(?s)rUIMng\.m_LoginWin\.GetPassInputBox\(\)->SetText\(szPass\);\s*}\s*break;'
$newCB = 'rUIMng.m_LoginWin.GetPassInputBox()->SetText(szPass);
		if (gInterface->Data[eWindow_DangKyInGame].OnShow)
		{
			gInterface->Data[eWindow_DangKyInGame].OnShow = 0;
		}
		if (gmProtect->remember_account)
		{
			rUIMng.m_LoginWin.SaveAccount(szID, szPass);
		}
	}
	break;'
$textCB = [regex]::Replace($textCB, $oldCB, $newCB)

[IO.File]::WriteAllText($pathCB, $textCB, [System.Text.Encoding]::Default)
