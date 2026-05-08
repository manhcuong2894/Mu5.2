import re

file_path = r"E:\MU 5.2\Takumi base\SRCMainGSShader\SRCMainGS\Item.xml"

with open(file_path, "r", encoding="utf-8") as f:
    lines = f.readlines()

max_lens = {}
item_lines = []

for i, line in enumerate(lines):
    stripped = line.strip()
    if stripped.startswith("<Item ") and stripped.endswith("/>"):
        matches = re.findall(r'([a-zA-Z0-9_]+)="([^"]*)"', line)
        item_lines.append((i, matches, line[:len(line) - len(line.lstrip('\t '))]))
        for k, v in matches:
            kv_str = f'{k}="{v}"'
            max_lens[k] = max(max_lens.get(k, 0), len(kv_str))

if not item_lines:
    print("No items found or different format")
else:
    for idx, matches, indent in item_lines:
        new_line = indent + "<Item "
        row_strs = []
        for k, v in matches:
            kv_str = f'{k}="{v}"'
            row_strs.append(kv_str.ljust(max_lens[k]))
        new_line += "  ".join(row_strs) + " />\n"
        lines[idx] = new_line

    with open(file_path, "w", encoding="utf-8") as f:
        f.writelines(lines)
    print("Formatted ok")
