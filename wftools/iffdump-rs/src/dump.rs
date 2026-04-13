//! Output formatters: pretty HDump mode and iffcomp `$HH` mode.

use std::io::{self, Write};
use wf_hdump::hdump;

/// Emit pretty hex+ASCII dump (the `-f+` default).
/// Delegates to `wf_hdump::hdump`.
pub fn format_hdump(
    data: &[u8],
    indent: usize,
    chars_per_line: usize,
    out: &mut impl Write,
) -> io::Result<()> {
    if data.is_empty() {
        return Ok(());
    }
    hdump(data, indent, chars_per_line, out)
}

/// Emit iffcomp-compatible `$HH $HH ...` hex listing (the `-f-` mode).
///
/// Bytes are written as `$XX` tokens separated by spaces, wrapped so that
/// each line of tokens is at most `chars_per_line` *characters* wide
/// (matching the C++ `lineCounter % 64` logic but keyed off actual char
/// count).  Each line is prefixed with `indent` tabs.
pub fn format_iffcomp(
    data: &[u8],
    indent: usize,
    chars_per_line: usize,
    out: &mut impl Write,
) -> io::Result<()> {
    if data.is_empty() {
        return Ok(());
    }
    // Each token is "$XX " = 4 chars; fit as many as possible per line.
    let tokens_per_line = (chars_per_line / 4).max(1);
    let tabs: Vec<u8> = std::iter::repeat(b'\t').take(indent).collect();

    for (i, &byte) in data.iter().enumerate() {
        if i % tokens_per_line == 0 {
            out.write_all(&tabs)?;
        }
        write!(out, "${:02X}", byte)?;
        if i + 1 < data.len() {
            if (i + 1) % tokens_per_line == 0 {
                out.write_all(b"\n")?;
            } else {
                out.write_all(b" ")?;
            }
        }
    }
    out.write_all(b"\n")?;
    Ok(())
}
