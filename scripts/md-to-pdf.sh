#!/usr/bin/env bash
# Convert a Markdown file to a self-contained HTML (with embedded images)
# that can be printed to PDF from a browser.
#
# Usage: ./scripts/md-to-pdf.sh [FILE...]
#   Defaults to README.md if no files given

set -euo pipefail

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  echo "Usage: $0 [FILE...]"
  echo "  Convert Markdown files to self-contained HTML for PDF printing"
  echo "  Defaults to README.md if no files given"
  exit 0
fi

if [ $# -eq 0 ]; then
  FILES=(README.md)
else
  FILES=("$@")
fi

for FILE in "${FILES[@]}"; do

if [[ ! -f "$FILE" ]]; then
  echo "Error: $FILE not found" >&2
  continue
fi

# Resolve directory so relative image paths work
FILE_DIR="$(cd "$(dirname "$FILE")" && pwd)"
FILE_NAME="$(basename "$FILE")"
BASE_NAME="${FILE_NAME%.*}"
OUT="/tmp/${BASE_NAME}.html"

python3 -c "
import base64, re, os, sys, html as html_mod, urllib.request, urllib.parse, subprocess, shutil, io

file_dir = sys.argv[1]
file_path = os.path.join(file_dir, sys.argv[2])
out_path = sys.argv[3]

with open(file_path) as f:
    md = f.read()

# Resize raster images wider than max_width using gm (GraphicsMagick).
# Returns the (possibly resized) image bytes and mime type.
MAX_IMG_WIDTH = 1200

def _read_and_resize(img_path, ext):
    mime = {'png': 'image/png', 'jpg': 'image/jpeg', 'jpeg': 'image/jpeg',
            'gif': 'image/gif', 'svg': 'image/svg+xml', 'webp': 'image/webp'}.get(ext, 'image/png')
    with open(img_path, 'rb') as f:
        data = f.read()
    if ext in ('svg',) or not shutil.which('gm'):
        return data, mime
    try:
        result = subprocess.run(
            ['gm', 'identify', '-format', '%w', img_path],
            capture_output=True, text=True, timeout=5)
        width = int(result.stdout.strip())
        if width > MAX_IMG_WIDTH:
            # Resize and convert to JPEG (PNG recompression often inflates)
            result = subprocess.run(
                ['gm', 'convert', img_path, '-resize', f'{MAX_IMG_WIDTH}x>',
                 '-quality', '80', 'jpeg:-'],
                capture_output=True, timeout=15)
            if result.returncode == 0 and result.stdout:
                data = result.stdout
                mime = 'image/jpeg'
    except Exception:
        pass  # fall back to original
    return data, mime

# Replace <img src=...> tags with base64-embedded versions
def replace_img(m):
    full = m.group(0)
    src_match = re.search(r'src=\"([^\"]+)\"', full)
    if not src_match:
        return full
    src = src_match.group(1)
    img_path = os.path.join(file_dir, src)
    if not os.path.exists(img_path):
        return full
    ext = os.path.splitext(src)[1].lstrip('.') or 'png'
    data, mime = _read_and_resize(img_path, ext)
    b64 = base64.b64encode(data).decode()
    return full.replace(src, f'data:{mime};base64,{b64}')

md = re.sub(r'<img\s[^>]+>', replace_img, md)

# Replace ![alt](src) markdown images with base64-embedded <img> tags
def replace_md_img(m):
    alt = m.group(1)
    src = m.group(2)
    img_path = os.path.join(file_dir, src)
    if not os.path.exists(img_path):
        return m.group(0)
    ext = os.path.splitext(src)[1].lstrip('.') or 'png'
    data, mime = _read_and_resize(img_path, ext)
    b64 = base64.b64encode(data).decode()
    return f'<img src=\"data:{mime};base64,{b64}\" alt=\"{alt}\">'

md = re.sub(r'!\[([^\]]*)\]\(([^)]+)\)', replace_md_img, md)

# Convert markdown to HTML
lines = md.split('\n')
html_lines = []
in_code = False
in_table = False
table_aligns = []
in_blockquote = False
code_lines = []
code_lang = ''
# Stack of open list tags, e.g. [('ul', 0), ('ol', 2)]
list_stack = []
li_open = False  # True when a <li> is open and can accept nested content
# Accumulate consecutive text lines into a single paragraph
para_lines = []

def close_li():
    global li_open
    if li_open:
        html_lines.append('</li>')
        li_open = False

def close_lists():
    global list_stack
    close_li()
    while list_stack:
        tag, _ = list_stack.pop()
        html_lines.append(f'</{tag}>')

def close_lists_to_depth(indent):
    global list_stack
    while list_stack and list_stack[-1][1] >= indent:
        close_li()
        tag, _ = list_stack.pop()
        html_lines.append(f'</{tag}>')

def close_table():
    global in_table, table_aligns
    if in_table: html_lines.append('</tbody></table>'); in_table = False; table_aligns = []

def close_blockquote():
    global in_blockquote
    if in_blockquote: html_lines.append('</blockquote>'); in_blockquote = False

def flush_para():
    global para_lines
    if para_lines:
        html_lines.append('<p>' + ' '.join(para_lines) + '</p>')
        para_lines = []

def parse_table_row(line):
    # Split on | but respect backtick code spans (| inside them is literal)
    # Handles multi-backtick spans like \`\` \` \`\` (double-backtick wrapping a literal backtick)
    s = line.strip().strip('|')
    cells = []
    current = []
    i = 0
    while i < len(s):
        ch = s[i]
        if ch == '\`':
            # Count consecutive backticks to determine code span delimiter
            tick_start = i
            while i < len(s) and s[i] == '\`':
                i += 1
            tick_len = i - tick_start
            ticks = '\`' * tick_len
            current.append(ticks)
            # Find matching closing backticks (same count)
            close_idx = s.find(ticks, i)
            if close_idx != -1:
                current.append(s[i:close_idx])
                current.append(ticks)
                i = close_idx + tick_len
            # else: no closing ticks found, just continue
        elif ch == '|':
            cells.append(''.join(current).strip())
            current = []
            i += 1
        else:
            current.append(ch)
            i += 1
    cells.append(''.join(current).strip())
    return cells

for line in lines:
    stripped = line.strip()
    # Fenced code blocks
    if stripped.startswith('\`\`\`'):
        flush_para()
        if in_code:
            if code_lang == 'mermaid':
                # Render via mermaid.ink API
                mermaid_src = '\n'.join(code_lines)
                try:
                    encoded = base64.urlsafe_b64encode(mermaid_src.encode()).decode()
                    url = f'https://mermaid.ink/svg/{encoded}'
                    req = urllib.request.Request(url, headers={'User-Agent': 'md-to-pdf'})
                    with urllib.request.urlopen(req, timeout=15) as resp:
                        svg = resp.read().decode()
                    html_lines.append(f'<div class=\"mermaid-diagram\">{svg}</div>')
                except Exception as e:
                    print(f'Warning: mermaid.ink render failed ({e}), falling back to <pre>', file=sys.stderr)
                    html_lines.append('<pre><code>' + '\n'.join(code_lines) + '</code></pre>')
            else:
                html_lines.append('<pre><code>' + '\n'.join(code_lines) + '</code></pre>')
            code_lines = []
            code_lang = ''
            in_code = False
        else:
            line_indent = len(line) - len(line.lstrip())
            # Only close lists if the code fence is not indented under a list item
            if not list_stack or line_indent <= list_stack[-1][1]:
                close_lists()
            close_table(); close_blockquote()
            code_lang = stripped.removeprefix('\`\`\`').strip()
            in_code = True
        continue
    if in_code:
        code_lines.append(line if code_lang == 'mermaid' else html_mod.escape(line))
        continue

    # Tables
    if stripped.startswith('|') and stripped.endswith('|'):
        flush_para()
        # Separator row (e.g. |---|---|) — parse alignment
        if re.match(r'^\|[\s\-:|]+\|$', stripped):
            table_aligns = []
            for cell in stripped.strip('|').split('|'):
                cell = cell.strip()
                if cell.startswith(':') and cell.endswith(':'):
                    table_aligns.append('center')
                elif cell.endswith(':'):
                    table_aligns.append('right')
                else:
                    table_aligns.append('left')
            continue
        cells = parse_table_row(line)
        # Process inline code in cells first, escaping HTML inside code spans,
        # then escape remaining HTML in the rest of the cell content
        def escape_cell(c):
            # Handle double-backtick spans (can contain single backticks) before single-backtick spans
            parts = re.split(r'(\`\`.+?\`\`|\`[^\`]+\`)', c)
            result = []
            for part in parts:
                if part.startswith('\`\`') and part.endswith('\`\`'):
                    inner = html_mod.escape(part[2:-2].strip())
                    # Escape special chars so body-level inline processing doesn't re-match
                    inner = inner.replace('\`', '&#96;').replace('[', '&#91;').replace(']', '&#93;').replace('|', '&#124;')
                    result.append('<code>' + inner + '</code>')
                elif part.startswith('\`') and part.endswith('\`'):
                    inner = html_mod.escape(part[1:-1])
                    inner = inner.replace('\`', '&#96;').replace('[', '&#91;').replace(']', '&#93;').replace('|', '&#124;')
                    result.append('<code>' + inner + '</code>')
                else:
                    result.append(html_mod.escape(part))
            return ''.join(result)
        cells = [escape_cell(c) for c in cells]
        if not in_table:
            close_lists(); close_blockquote()
            html_lines.append('<table><thead><tr>')
            for ci, c in enumerate(cells):
                align = table_aligns[ci] if ci < len(table_aligns) else 'left'
                style = f' style="text-align:{align}"' if align != 'left' else ''
                html_lines.append(f'<th{style}>{c}</th>')
            html_lines.append('</tr></thead><tbody>')
            in_table = True
        else:
            html_lines.append('<tr>')
            for ci, c in enumerate(cells):
                align = table_aligns[ci] if ci < len(table_aligns) else 'left'
                style = f' style="text-align:{align}"' if align != 'left' else ''
                html_lines.append(f'<td{style}>{c}</td>')
            html_lines.append('</tr>')
        continue

    if in_table and not stripped.startswith('|'):
        close_table()

    # Blockquotes
    if stripped.startswith('> '):
        flush_para(); close_lists(); close_table()
        if not in_blockquote:
            html_lines.append('<blockquote>')
            in_blockquote = True
        html_lines.append(f'<p>{stripped[2:]}</p>')
        continue
    elif in_blockquote and stripped != '':
        close_blockquote()

    # Horizontal rules
    if stripped in ('---', '***', '___'):
        flush_para(); close_lists(); close_table(); close_blockquote()
        html_lines.append('<hr>')
        continue

    # Headers (check longest prefix first)
    header_match = re.match(r'^(#{1,6})\s+(.*)', line)
    if header_match:
        flush_para(); close_lists(); close_table(); close_blockquote()
        level = len(header_match.group(1))
        text = header_match.group(2)
        html_lines.append(f'<h{level}>{text}</h{level}>')
        continue

    # List items (ordered, unordered, task lists) with nesting
    list_match = re.match(r'^(\s*)([-*])\s+(.*)$', line)
    ol_match = re.match(r'^(\s*)\d+\.\s+(.*)$', line) if not list_match else None

    if list_match or ol_match:
        flush_para(); close_table(); close_blockquote()
        if list_match:
            indent = len(list_match.group(1))
            text = list_match.group(3)
            tag = 'ul'
        else:
            indent = len(ol_match.group(1))
            text = ol_match.group(2)
            tag = 'ol'

        # Close lists deeper than current indent
        while list_stack and list_stack[-1][1] > indent:
            t, _ = list_stack.pop()
            html_lines.append(f'</{t}>')

        # Open new list if needed (different depth or different type)
        if not list_stack or list_stack[-1][1] < indent:
            html_lines.append(f'<{tag}>')
            list_stack.append((tag, indent))

        # Close previous <li> before opening a new one
        close_li()

        # Task list items
        task_match = re.match(r'^\[([ xX])\]\s+(.*)', text)
        if task_match:
            checked = 'checked ' if task_match.group(1) in ('x', 'X') else ''
            html_lines.append(f'<li class=\"task\"><input type=\"checkbox\" {checked}disabled> {task_match.group(2)}')
        else:
            html_lines.append(f'<li>{text}')
        li_open = True
        continue

    # Non-list line closes lists — but indented content (code blocks,
    # paragraphs) under a list item stays inside the list.
    if list_stack and stripped != '':
        line_indent = len(line) - len(line.lstrip())
        list_indent = list_stack[-1][1]
        if line_indent <= list_indent:
            close_lists()

    # HTML passthrough
    if stripped.startswith('<'):
        flush_para()
        html_lines.append(line)
    elif stripped == '':
        flush_para()
        if list_stack:
            pass  # Keep lists open across single blank lines
        else:
            close_blockquote()
            html_lines.append('')
    else:
        # Accumulate text lines into a paragraph (joined on flush)
        para_lines.append(line)

flush_para()
close_lists()
close_table()
close_blockquote()

body = '\n'.join(html_lines)

# Inline markdown: extract <pre> blocks and code spans FIRST so their content
# is protected from bold/italic/escape processing.

# Protect <pre><code>...</code></pre> blocks
pre_blocks = []
def save_pre_block(m):
    pre_blocks.append(m.group(0))
    return f'\x00PRE{len(pre_blocks)-1}\x00'
body = re.sub(r'<pre><code>.*?</code></pre>', save_pre_block, body, flags=re.DOTALL)

# Handle backtick code spans with matched delimiters (CommonMark rules):
# opening backticks of any length are matched by the same-length closing backticks.
code_spans = []
def save_code_span(m):
    code_spans.append(m.group(2).strip())
    return f'\x00CODE{len(code_spans)-1}\x00'
body = re.sub(r'(?<!\`)(\`{1,3})(?!\`)(.+?)(?<!\`)\1(?!\`)', save_code_span, body)

# Handle backslash escapes (e.g. \* → placeholder, then restore after inline processing)
# NOTE: backtick escapes are already handled by code span extraction above
ESCAPE_MAP = {r'\*': '\x00STAR\x00', r'\_': '\x00UNDER\x00', r'\~': '\x00TILDE\x00',
              r'\[': '\x00LBRACK\x00', r'\]': '\x00RBRACK\x00'}
for esc, placeholder in ESCAPE_MAP.items():
    body = body.replace(esc, placeholder)

# Bold, italic, strikethrough, links (code content is now placeholder-protected)
# NOTE: emphasis must NOT cross newlines — \n is in the negated class so a stray
# literal '*' (e.g. 'bg-*.sh') can't pair with another '*' on a later line and
# wrap unrelated content in <em>/<strong>. Paragraphs are already collapsed to
# a single line by flush_para(), so legitimate emphasis fits on one line.
body = re.sub(r'\*\*\*([^*\n]+)\*\*\*', r'<strong><em>\1</em></strong>', body)
body = re.sub(r'\*\*([^*\n]+)\*\*', r'<strong>\1</strong>', body)
body = re.sub(r'(?<!\*)\*([^*\n]+)\*(?!\*)', r'<em>\1</em>', body)
body = re.sub(r'(?<![_\w])_([^_\n]+)_(?![_\w])', r'<em>\1</em>', body)
body = re.sub(r'~~(.+?)~~', r'<del>\1</del>', body)
body = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', r'<a href=\"\2\">\1</a>', body)

# Restore code spans
for i, content in enumerate(code_spans):
    body = body.replace(f'\x00CODE{i}\x00', f'<code>{html_mod.escape(content)}</code>')

# Restore <pre> blocks
for i, block in enumerate(pre_blocks):
    body = body.replace(f'\x00PRE{i}\x00', block)

# Restore backslash escapes as literal characters
RESTORE_MAP = {'\x00STAR\x00': '*', '\x00UNDER\x00': '_', '\x00TILDE\x00': '~',
               '\x00LBRACK\x00': '[', '\x00RBRACK\x00': ']'}
for placeholder, char in RESTORE_MAP.items():
    body = body.replace(placeholder, char)

title = os.path.splitext(sys.argv[2])[0]

html = f'''<!DOCTYPE html>
<html>
<head>
<meta charset=\"utf-8\">
<title>{title}</title>
<style>
  body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif; max-width: 900px; margin: 40px auto; padding: 0 20px; color: #1f2328; line-height: 1.5; }}
  h1 {{ border-bottom: 1px solid #d1d9e0; padding-bottom: 8px; }}
  h2 {{ border-bottom: 1px solid #d1d9e0; padding-bottom: 6px; margin-top: 24px; }}
  h3 {{ margin-top: 20px; }}
  h4 {{ margin-top: 16px; }}
  img {{ border-radius: 8px; margin: 4px; vertical-align: top; max-width: 100%; height: auto; }}
  ul, ol {{ padding-left: 2em; }}
  li {{ margin-bottom: 4px; }}
  li.task {{ list-style: none; margin-left: -1.5em; }}
  li.task input {{ margin-right: 6px; }}
  a {{ color: #0969da; text-decoration: none; }}
  code {{ background: #f6f8fa; padding: 2px 6px; border-radius: 4px; font-size: 0.9em; }}
  pre {{ background: #f6f8fa; padding: 16px; border-radius: 6px; overflow-x: auto; }}
  pre code {{ background: none; padding: 0; }}
  blockquote {{ border-left: 4px solid #d1d9e0; margin: 16px 0; padding: 4px 16px; color: #656d76; }}
  blockquote p {{ margin: 4px 0; }}
  del {{ color: #656d76; }}
  table {{ border-collapse: collapse; width: 100%; margin: 16px 0; }}
  th, td {{ border: 1px solid #d1d9e0; padding: 8px 12px; text-align: left; }}
  th {{ background: #f6f8fa; font-weight: 600; }}
  tr:nth-child(even) {{ background: #f6f8fa; }}
  hr {{ border: none; border-top: 1px solid #d1d9e0; margin: 24px 0; }}
  .mermaid-diagram {{ margin: 24px -20px; }}
  .mermaid-diagram svg {{ width: 100%; height: auto; }}
  @media print {{ body {{ max-width: 100%; margin: 0; }} img {{ break-inside: avoid; }} table {{ break-inside: avoid; }} .mermaid-diagram {{ margin: 24px 0; page-break-inside: avoid; }} }}
</style>
</head>
<body>
{body}
</body>
</html>'''

with open(out_path, 'w') as f:
    f.write(html)
size_kb = os.path.getsize(out_path) // 1024
print(f'{out_path} ({size_kb} KB)')
" "$FILE_DIR" "$FILE_NAME" "$OUT"

# Try to open in browser
if command -v xdg-open &>/dev/null; then
  xdg-open "$OUT" 2>/dev/null &
elif command -v open &>/dev/null; then
  open "$OUT"
fi

done
