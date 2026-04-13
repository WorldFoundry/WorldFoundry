use std::fmt;
use std::io;
use std::path::PathBuf;

pub type Result<T> = std::result::Result<T, IffDumpError>;

#[derive(Debug)]
pub enum IffDumpError {
    Io { path: PathBuf, source: io::Error },
    Parse { msg: String },
}

impl fmt::Display for IffDumpError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IffDumpError::Io { path, source } => {
                write!(f, "{}: {}", path.display(), source)
            }
            IffDumpError::Parse { msg } => write!(f, "parse error: {}", msg),
        }
    }
}

impl std::error::Error for IffDumpError {}
