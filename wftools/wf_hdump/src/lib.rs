//! Hex + ASCII dumper.
//!
//! Port of `wfsource/source/recolib/hdump.cc` (Cave Logic Studios / World Foundry).
//!
//! Each line:  `OOOO: HHHHHHHH HHHHHHHH HHHHHHHH HHHHHHHH    AAAA....`
//!
//! - Offset is 4 hex digits.
//! - Hex bytes are grouped in fours with a space between groups.
//! - ASCII column maps non-printable bytes (and 0x80–0xFF) to `.`.
//! - Short final lines are padded so the ASCII column is aligned.

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
    chars_per_line: usize,
    indent: usize,
    out: &mut impl Write,
) -> io::Result<()> {
    // indent
    for _ in 0..indent {
        out.write_all(b"\t")?;
    }
    // offset
    write!(out, "{:04x}: ", offset)?;
    // hex bytes — groups of 4
    for i in 0..chars_per_line {
        if i < data.len() {
            write!(out, "{:02X}", data[i])?;
        } else {
            write!(out, "  ")?; // padding for short final line
        }
        if (i + 1) % 4 == 0 {
            write!(out, " ")?;
        }
    }
    // separator
    write!(out, "   ")?;
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
        hdump_line(chunk, offset, chars_per_line, indent, out)?;
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
