//! CLI wrapper around [`iffcomp::compile`] — flags chosen to stay drop-in
//! compatible with the C++ and Go iffcomp binaries so existing build scripts
//! and test targets work unchanged.
//!
//!     iffcomp [-o=file] [-binary|-ascii] [-v] [-q] <input>
//!
//! `-ascii` selects [`iffcomp::Mode::Text`] (matches `IffWriterText`).
//! `-binary` is the default and selects [`iffcomp::Mode::Binary`].
//! `-v` enables parse-rule tracing on stderr.

use iffcomp::{compile, Mode, Options};
use std::process::ExitCode;

fn usage() -> ! {
    eprintln!("usage: iffcomp [-o=file] [-binary|-ascii] [-v] [-q] <input>");
    std::process::exit(1);
}

fn main() -> ExitCode {
    let mut out_file = String::from("test.wf");
    let mut mode = Mode::Binary;
    let mut verbose = false;
    let mut _quiet = false;
    let mut positional: Option<String> = None;

    for raw in std::env::args().skip(1) {
        // Normalize `--flag` → `-flag`.
        let a: &str = if let Some(rest) = raw.strip_prefix("--") {
            // rebind to a temporary str; we need it to live long enough
            // — do the match on the stripped form below.
            // The trick: shadow `raw` via a second loop body.
            let _ = rest;
            // fallthrough: treat the same as a single-dash flag
            raw.trim_start_matches('-')
        } else if let Some(rest) = raw.strip_prefix('-') {
            rest
        } else {
            // Positional.
            if positional.is_some() {
                eprintln!("iffcomp: multiple input files not supported");
                usage();
            }
            positional = Some(raw.clone());
            continue;
        };

        match a {
            "binary" => mode = Mode::Binary,
            "ascii" => mode = Mode::Text,
            "v" => verbose = true,
            "q" => _quiet = true,
            _ if a.starts_with("o=") => {
                out_file = a["o=".len()..].to_string();
            }
            _ => {
                eprintln!("iffcomp: unrecognized option -{}", a);
                usage();
            }
        }
    }

    let in_file = match positional {
        Some(p) => p,
        None => usage(),
    };

    match compile(&in_file, &out_file, Options { mode, verbose }) {
        Ok(n) => {
            if verbose {
                eprintln!("wrote {} bytes to {}", n, out_file);
            }
            ExitCode::SUCCESS
        }
        Err(e) => {
            eprintln!("iffcomp: {}", e);
            ExitCode::from(10)
        }
    }
}
