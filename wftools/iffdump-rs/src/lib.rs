//! iffdump-rs — IFF file structure dumper.
//!
//! Port of `wftools/iffdump/iffdump.cc`.

pub mod dump;
pub mod error;
pub mod reader;

pub use error::{IffDumpError, Result};
pub use reader::{Opts, parse_chunk_list};

use std::collections::HashSet;
use std::io::Write;
use std::path::Path;

/// Default wrapper chunk FOURCC list (space-separated, matching `iffdump.cc`).
pub const DEFAULT_CHUNKS: &str = "OADL TYPE LVAS L0   L1   L2   L3   PERM RM0  RM1  RM2  RM3  RM4  UDM  IFFC";

/// Parse the default wrapper list into a HashSet.
pub fn default_wrappers() -> HashSet<u32> {
    parse_chunk_list(DEFAULT_CHUNKS)
}

/// Dump an IFF file to `out`.
///
/// `in_path` is used only for error messages.
/// `wrappers` is the set of FOURCCs to treat as container chunks.
pub fn dump(
    data: &[u8],
    _in_path: &Path,
    out_path: Option<&Path>,
    wrappers: &HashSet<u32>,
    opts: &Opts,
    out: &mut impl Write,
) -> Result<()> {
    // Header comment
    let out_name = out_path
        .map(|p| p.display().to_string())
        .unwrap_or_else(|| "<stdout>".to_string());
    writeln!(out, "//=============================================================================")
        .map_err(|e| IffDumpError::Io { path: out_name.as_str().into(), source: e })?;
    writeln!(out, "// {} Created by iffdump v{}", out_name, env!("CARGO_PKG_VERSION"))
        .map_err(|e| IffDumpError::Io { path: out_name.as_str().into(), source: e })?;
    writeln!(out, "//=============================================================================")
        .map_err(|e| IffDumpError::Io { path: out_name.as_str().into(), source: e })?;

    reader::dump_chunks(data, 0, data.len(), 0, wrappers, opts, out)?;
    Ok(())
}
