//! Hex + ASCII dumper.
//!
//! Port of `wfsource/source/recolib/hdump.cc` (Cave Logic Studios / World Foundry).
//!
//! Each line:  `OOOO: HHHHHHHH HHHHHHHH HHHHHHHH HHHHHHHH    AAAA....`
//!
//! - Offset is 4 hex digits.
//! - Hex bytes are grouped in fours with a space after each group (including the last).
//! - Four spaces separate the hex section from the ASCII column.
//! - Short final lines are NOT padded — the ASCII column shifts left (matches C++ original).

use std::io::{self, Write};

/// The printable-character table from `hdump.cc`.
/// Bytes 0x20–0x7E are themselves; everything else is `.`.
fn to_printable(b: u8) -> u8 {
    if b >= 0x20 && b <= 0x7e {
        b
    } else {
        b'.'
    }
}

/// Emit one line of hex+ASCII output for `data` (up to `chars_per_line` bytes).
///
/// `indent` is the number of tab stops to prefix each line.
/// The hex column is padded to `chars_per_line` bytes wide so the ASCII
/// column is always aligned even on a short final line.
fn hdump_line(
    data: &[u8],
    offset: usize,
    indent: usize,
    out: &mut impl Write,
) -> io::Result<()> {
    // indent
    for _ in 0..indent {
        out.write_all(b"\t")?;
    }
    // offset
    write!(out, "{:04X}: ", offset)?;
    // hex bytes — groups of 4, space after each group (including the last)
    // No padding for short lines; the ASCII column shifts left (matches C++ HDumpLine).
    for (i, &b) in data.iter().enumerate() {
        write!(out, "{:02X}", b)?;
        if (i + 1) % 4 == 0 {
            write!(out, " ")?;
        }
    }
    // 4-space separator between hex section and ASCII column
    write!(out, "    ")?;
    // ASCII column
    for &b in data {
        out.write_all(&[to_printable(b)])?;
    }
    out.write_all(b"\n")?;
    Ok(())
}

/// Dump `data` in hex+ASCII format, prefixing each line with `indent` tabs.
///
/// `chars_per_line` controls how many bytes appear per line (default 16
/// matches the C++ original).
pub fn hdump(
    data: &[u8],
    indent: usize,
    chars_per_line: usize,
    out: &mut impl Write,
) -> io::Result<()> {
    assert!(chars_per_line > 0, "chars_per_line must be > 0");
    let mut offset = 0usize;
    for chunk in data.chunks(chars_per_line) {
        hdump_line(chunk, offset, indent, out)?;
        offset += chunk.len();
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn empty_data_produces_no_output() {
        let mut out = Vec::new();
        hdump(&[], 0, 16, &mut out).unwrap();
        assert!(out.is_empty());
    }

    #[test]
    fn single_line() {
        let mut out = Vec::new();
        hdump(b"Hello", 0, 16, &mut out).unwrap();
        let s = String::from_utf8(out).unwrap();
        assert!(s.starts_with("0000: "));
        assert!(s.contains("Hello"));
    }

    #[test]
    fn non_printable_becomes_dot() {
        let mut out = Vec::new();
        hdump(&[0x00, 0x01, 0x41, 0xFF], 0, 16, &mut out).unwrap();
        let s = String::from_utf8(out).unwrap();
        // ASCII column: two dots, 'A', dot
        assert!(s.contains("..A."));
    }
}
