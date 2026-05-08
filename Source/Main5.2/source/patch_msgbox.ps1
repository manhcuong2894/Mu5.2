$path = 'e:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Source\Main5.2\source\NewUIInvenExpansion.cpp'
$content = [IO.File]::ReadAllText($path, [Text.Encoding]::Default)
$old = 'unicode::_sprintf(buytext, GlobalText[3185], m_PurchaseCoin);'
$new = 'unicode::_sprintf(buytext, "Bạn Có Chắc Chắn \nMuốn Mở Rộng Thêm Rương Với Giá %d WC Không?", m_PurchaseCoin);'
$content = $content.Replace($old, $new)

$oldTitle = '&OnClickBuyExpansionOk, "Thng Bo", buytext'
$newTitle = '&OnClickBuyExpansionOk, "Thông Báo", buytext'
$content = $content.Replace($oldTitle, $newTitle)

[IO.File]::WriteAllText($path, $content, [Text.Encoding]::Default)
