//! Single error type for the whole crate.
//!
//! A plain enum keeps us off `thiserror` and `anyhow`. Every fallible function
//! returns `Result<T, IffError>` and variants carry enough context (file name,
//! line/col, the raw detail) to format a useful error message without extra
//! layers of wrapping.

use std::fmt;
use std::io;
use std::path::PathBuf;

#[derive(Debug)]
pub enum IffError {
    Io {
        path: PathBuf,
        source: io::Error,
    },
    /// Lexer-level failure — an unexpected character, an unterminated literal,
    /// or a malformed directive. `at` is the source location that triggered it.
    Lex {
        at: Pos,
        msg: String,
    },
    /// Parser-level failure — wrong token kind, grammar rule mismatch.
    Parse {
        at: Pos,
        msg: String,
    },
    /// End-of-parse back-patch failure: one or more `.sizeof` / `.offsetof`
    /// references named a chunk that never appeared in the input.
    UnresolvedSymbols(Vec<String>),
}

/// Source position used in Lex/Parse errors.
#[derive(Debug, Clone, Default)]
pub struct Pos {
    pub filename: String,
    pub line: u32,
    pub col: u32,
}

impl fmt::Display for Pos {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.filename.is_empty() {
            write!(f, "<unknown>:{}:{}", self.line, self.col)
        } else {
            write!(f, "{}:{}:{}", self.filename, self.line, self.col)
        }
    }
}

impl fmt::Display for IffError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IffError::Io { path, source } => write!(f, "{}: {}", path.display(), source),
            IffError::Lex { at, msg } => write!(f, "{}: {}", at, msg),
            IffError::Parse { at, msg } => write!(f, "{}: {}", at, msg),
            IffError::UnresolvedSymbols(refs) => {
                write!(f, "unresolved chunk references: {}", refs.join(", "))
            }
        }
    }
}

impl std::error::Error for IffError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match self {
            IffError::Io { source, .. } => Some(source),
            _ => None,
        }
    }
}

pub type Result<T> = std::result::Result<T, IffError>;
