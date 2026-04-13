//! iffdump — IFF file structure dumper.
//!
//! Usage: iffdump [-c<chunks_file>] [-d+|-d-] [-f+|-f-] [-w=<N>] <infile> [outfile]

use iffdump::{Opts, default_wrappers, dump, parse_chunk_list};
use std::fs;
use std::io::{self, BufWriter};
use std::path::PathBuf;
use std::process;

fn usage() -> ! {
    eprintln!("Usage: iffdump [-c<chunks_file>] [-d+|-d-] [-f+|-f-] [-w=<N>] <infile> [outfile]");
    eprintln!("  -c<file>   wrapper chunk whitelist (default: built-in list)");
    eprintln!("  -d+/-d-    enable/disable binary dump of leaf chunks (default: +)");
    eprintln!("  -f+/-f-    pretty HDump / iffcomp $XX format (default: +)");
    eprintln!("  -w=<N>     bytes per line in hex dump (default: 16)");
    process::exit(1);
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let mut chunk_file: Option<String> = None;
    let mut use_hdump = true;
    let mut dump_binary = true;
    let mut chars_per_line: usize = 16;
    let mut positional: Vec<String> = Vec::new();

    let mut i = 1;
    while i < args.len() {
        let arg = &args[i];
        if !arg.starts_with('-') {
            positional.push(arg.clone());
        } else if arg.starts_with("-c") || arg.starts_with("-C") {
            chunk_file = Some(arg[2..].to_string());
        } else if arg == "-f+" || arg == "-F+" {
            use_hdump = true;
        } else if arg == "-f-" || arg == "-F-" {
            use_hdump = false;
        } else if arg == "-d+" || arg == "-D+" {
            dump_binary = true;
        } else if arg == "-d-" || arg == "-D-" {
            dump_binary = false;
        } else if arg.starts_with("-w=") || arg.starts_with("-W=") {
            chars_per_line = match arg[3..].parse::<usize>() {
                Ok(n) if n > 0 => n,
                _ => {
                    eprintln!("iffdump: bad -w value: {}", &arg[3..]);
                    usage();
                }
            };
        } else {
            eprintln!("iffdump: unknown switch: {}", arg);
            usage();
        }
        i += 1;
    }

    if positional.is_empty() {
        usage();
    }

    let in_path = PathBuf::from(&positional[0]);
    let out_path: Option<PathBuf> = positional.get(1).map(PathBuf::from);

    // Read input
    let data = match fs::read(&in_path) {
        Ok(d) => d,
        Err(e) => {
            eprintln!("iffdump: {}: {}", in_path.display(), e);
            process::exit(1);
        }
    };

    // Build wrapper set
    let wrappers = if let Some(ref path) = chunk_file {
        match fs::read_to_string(path) {
            Ok(text) => parse_chunk_list(&text),
            Err(e) => {
                eprintln!("iffdump: {}: {}", path, e);
                process::exit(1);
            }
        }
    } else {
        default_wrappers()
    };

    let opts = Opts { use_hdump, dump_binary, chars_per_line };

    // Open output
    let result = if let Some(ref p) = out_path {
        let f = match fs::File::create(p) {
            Ok(f) => f,
            Err(e) => {
                eprintln!("iffdump: {}: {}", p.display(), e);
                process::exit(1);
            }
        };
        let mut w = BufWriter::new(f);
        dump(&data, &in_path, out_path.as_deref(), &wrappers, &opts, &mut w)
    } else {
        let stdout = io::stdout();
        let mut w = BufWriter::new(stdout.lock());
        dump(&data, &in_path, None, &wrappers, &opts, &mut w)
    };

    if let Err(e) = result {
        eprintln!("iffdump: {}", e);
        process::exit(1);
    }
}
