//! oaddump — OAD (Object Attribute Data) file structure dumper.
//!
//! Usage: oaddump <infile> [outfile]

use std::fs;
use std::io::{self, BufWriter};
use std::path::PathBuf;
use std::process;
use wf_oad::OadFile;

fn usage() -> ! {
    eprintln!("Usage: oaddump <infile> [outfile]");
    process::exit(1);
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let mut positional: Vec<String> = Vec::new();

    for arg in &args[1..] {
        if arg.starts_with('-') {
            // -p stream redirection not implemented (no-op)
        } else {
            positional.push(arg.clone());
        }
    }

    if positional.is_empty() {
        usage();
    }

    let in_path = PathBuf::from(&positional[0]);
    let out_path: Option<PathBuf> = positional.get(1).map(PathBuf::from);

    let data = match fs::read(&in_path) {
        Ok(d) => d,
        Err(e) => {
            eprintln!("oaddump: {}: {}", in_path.display(), e);
            process::exit(1);
        }
    };

    let oad = match OadFile::read(&mut std::io::Cursor::new(&data)) {
        Ok(o) => o,
        Err(e) => {
            eprintln!("oaddump: {}: {}", in_path.display(), e);
            process::exit(1);
        }
    };

    let result = if let Some(ref p) = out_path {
        let f = match fs::File::create(p) {
            Ok(f) => f,
            Err(e) => {
                eprintln!("oaddump: {}: {}", p.display(), e);
                process::exit(1);
            }
        };
        let mut w = BufWriter::new(f);
        use std::io::Write;
        write!(w, "{}", oad)
    } else {
        let stdout = io::stdout();
        let mut w = BufWriter::new(stdout.lock());
        use std::io::Write;
        write!(w, "{}", oad)
    };

    if let Err(e) = result {
        eprintln!("oaddump: write error: {}", e);
        process::exit(1);
    }
}
