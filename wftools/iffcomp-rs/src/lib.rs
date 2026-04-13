//! Rust port of World Foundry `wftools/iffcomp`.
//!
//! The library surface is intentionally tiny — one function, one options
//! struct, two writer modes. The CLI (`src/main.rs`) is a thin wrapper.
//! Everything structural mirrors the Go port (`wftools/iffcomp-go/`) so the
//! two are easy to cross-read side-by-side, and both are validated against
//! the modernized C++ reference (`wftools/iffcomp/iffcomp`) as a byte-exact
//! oracle.

pub mod error;
pub mod lexer;
pub mod parser;
pub mod writer;

use std::path::Path;

pub use error::{IffError, Result};
pub use writer::Mode;

/// Runtime options for [`compile`]. Defaults to binary output, non-verbose.
#[derive(Debug, Clone, Copy, Default)]
pub struct Options {
    pub mode: Mode,
    pub verbose: bool,
}

/// Read `in_file`, parse it as the iffcomp DSL, and write the output (binary
/// or text) to `out_file`. Returns the number of bytes written.
pub fn compile(
    in_file: impl AsRef<Path>,
    out_file: impl AsRef<Path>,
    opts: Options,
) -> Result<usize> {
    let in_path = in_file.as_ref();
    let out_path = out_file.as_ref();

    let lex = lexer::Lexer::open(in_path)?;
    let mut writer = writer::Writer::new(opts.mode);
    let mut p = parser::Parser::new(lex, &mut writer);
    p.verbose = opts.verbose;
    p.parse()?;
    drop(p);
    writer.resolve_backpatches()?;

    let data = writer.bytes();
    std::fs::write(out_path, data).map_err(|e| IffError::Io {
        path: out_path.to_path_buf(),
        source: e,
    })?;
    Ok(data.len())
}
