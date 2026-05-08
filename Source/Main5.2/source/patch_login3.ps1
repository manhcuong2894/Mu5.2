$path = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\LoginWin.cpp'
$lines = [System.IO.File]::ReadAllLines($path, [System.Text.Encoding]::Default)
for ($i=0; $i -lt $lines.Length; $i++) {
    if ($lines[$i] -match 'RequestLogin\(false\);') {
        $lines[$i] = $lines[$i].Replace('RequestLogin(false);', 'RequestLogin(true);')
    }
}
[System.IO.File]::WriteAllLines($path, $lines, [System.Text.Encoding]::Default)


$pathCB = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\CB_DangKyInGame.cpp'
$linesCB = [System.IO.File]::ReadAllLines($pathCB, [System.Text.Encoding]::Default)

$newLinesCB = New-Object System.Collections.Generic.List[string]
$insertedCallback = $false

for ($i=0; $i -lt $linesCB.Length; $i++) {
    # Insert the callback right before Instance() definition
    if (!$insertedCallback -and $linesCB[$i] -match 'CB_DangKyInGame\* CB_DangKyInGame::Instance\(\)') {
        $newLinesCB.Add("void OnClickRegisterSuccess(LPVOID pClass)")
        $newLinesCB.Add("{")
        $newLinesCB.Add("	if (gInterface->Data[eWindow_DangKyInGame].OnShow)")
        $newLinesCB.Add("	{")
        $newLinesCB.Add("		gInterface->Data[eWindow_DangKyInGame].OnShow = 0;")
        $newLinesCB.Add("	}")
        $newLinesCB.Add("}")
        $newLinesCB.Add("")
        $insertedCallback = $true
    }

    if ($linesCB[$i] -match 'case CB_DangKyInGame::eDangKyThanhCong:') {
        # Modify the OpenMessageBox line which is $linesCB[$i+2]
        $linesCB[$i+2] = $linesCB[$i+2].Replace('OpenMessageBox(', 'OpenMessageBoxOkCancel(&OnClickRegisterSuccess, ')
        
        # We also need to remove the inline OnShow = 0 that we previously added!
        $linesCB[$i+6] = "// removed inline close"
        $linesCB[$i+7] = "//"
        $linesCB[$i+8] = "//"
        $linesCB[$i+9] = "//"
    }

    $newLinesCB.Add($linesCB[$i])
}

[System.IO.File]::WriteAllLines($pathCB, $newLinesCB.ToArray(), [System.Text.Encoding]::Default)
