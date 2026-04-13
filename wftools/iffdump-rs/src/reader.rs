//! IFF chunk reader and recursive dumper.
//!
//! IFF on-disk layout (same as iffcomp-rs writes):
//!   [4 bytes] Chunk ID  — big-endian u32 (FOURCC, e.g. `TEST`)
//!   [4 bytes] Payload size — little-endian u32
//!   [N bytes] Payload
//!   [0–3 bytes] Alignment padding to 4-byte boundary (not counted in size)
//!
//! Wrappers (chunk IDs in the whitelist) recurse; leaves are hex-dumped.

use crate::dump::{format_hdump, format_iffcomp};
use crate::error::{IffDumpError, Result};
use std::collections::HashSet;
use std::io::Write;

/// Options controlling dump output.
pub struct Opts {
    /// Pretty HDump (`true`) vs iffcomp `$XX` (`false`).
    pub use_hdump: bool,
    /// Emit binary content of leaf chunks.
    pub dump_binary: bool,
    /// Bytes per line in the hex dump.
    pub chars_per_line: usize,
}

/// Convert a big-endian u32 FOURCC to a 4-char display string.
pub fn id_name(id: u32) -> String {
    let bytes = id.to_be_bytes();
    bytes
        .iter()
        .map(|&b| if b.is_ascii_graphic() || b == b' ' { b as char } else { '.' })
        .collect()
}

/// Parse the set of wrapper FOURCCs from a whitespace-separated text stream.
/// Lines beginning with `//` are treated as comments and skipped.
pub fn parse_chunk_list(text: &str) -> HashSet<u32> {
    let mut set = HashSet::new();
    for line in text.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with("//") {
            continue;
        }
        // Take only the first whitespace-delimited token on the line.
        for token in line.split_whitespace() {
            let bytes = token.as_bytes();
            if bytes.len() > 4 {
                continue; // malformed, skip
            }
            let mut arr = [b' '; 4];
            arr[..bytes.len()].copy_from_slice(bytes);
            let id = u32::from_be_bytes(arr);
            set.insert(id);
        }
    }
    set
}

/// Read one chunk from `buf` starting at `*pos`.
/// Returns `(id, payload_slice_range, aligned_end)`.
/// `*pos` is advanced past the aligned end of the chunk.
fn read_chunk_header(buf: &[u8], pos: &mut usize) -> Result<(u32, usize, usize)> {
    if *pos + 8 > buf.len() {
        return Err(IffDumpError::Parse {
            msg: format!(
                "truncated chunk header at offset {:#x} (buf len {:#x})",
                pos,
                buf.len()
            ),
        });
    }
    let id = u32::from_be_bytes(buf[*pos..*pos + 4].try_into().unwrap());
    let size = u32::from_le_bytes(buf[*pos + 4..*pos + 8].try_into().unwrap()) as usize;
    let payload_start = *pos + 8;
    let payload_end = payload_start + size;
    let aligned_end = (payload_end + 3) & !3;

    if aligned_end > buf.len() {
        return Err(IffDumpError::Parse {
            msg: format!(
                "chunk '{}' at {:#x}: size {} extends past end of buffer",
                id_name(id),
                pos,
                size
            ),
        });
    }
    *pos = aligned_end;
    Ok((id, size, payload_start))
}

/// Recursively dump all chunks in `buf[start..end]` at the given `depth`.
pub fn dump_chunks(
    buf: &[u8],
    start: usize,
    end: usize,
    depth: usize,
    wrappers: &HashSet<u32>,
    opts: &Opts,
    out: &mut impl Write,
) -> Result<()> {
    let mut pos = start;
    while pos < end {
        let before = pos;
        let (id, size, payload_start) = read_chunk_header(buf, &mut pos)?;
        let payload = &buf[payload_start..payload_start + size];
        let indent = depth;

        // Opening line: `{ 'FOUR'\t\t// Size = N`
        let tabs = "\t".repeat(indent);
        writeln!(out, "{tabs}{{ '{}'  \t// Size = {size}", id_name(id))
            .map_err(|e| IffDumpError::Io { path: "<output>".into(), source: e })?;

        if wrappers.contains(&id) {
            // Wrapper: recurse into payload
            dump_chunks(buf, payload_start, payload_start + size, depth + 1, wrappers, opts, out)?;
        } else {
            // Leaf: hex dump
            if opts.dump_binary && !payload.is_empty() {
                if opts.use_hdump {
                    format_hdump(payload, indent + 1, opts.chars_per_line, out)
                        .map_err(|e| IffDumpError::Io { path: "<output>".into(), source: e })?;
                } else {
                    format_iffcomp(payload, indent + 1, opts.chars_per_line, out)
                        .map_err(|e| IffDumpError::Io { path: "<output>".into(), source: e })?;
                }
            }
        }

        writeln!(out, "{tabs}}}")
            .map_err(|e| IffDumpError::Io { path: "<output>".into(), source: e })?;

        let _ = before; // used implicitly via pos
    }
    Ok(())
}
