//! End-to-end byte-exact differential tests against the C++ iffcomp oracle.
//!
//! Both tests `chdir` into `testdata/` so that `[ "TODO" ]` in the fixture
//! resolves to the local file. A `ChdirGuard` with `Drop` restores the
//! original working directory even on panic. Output is written to an ad-hoc
//! tempfile in `std::env::temp_dir()` so we stay dependency-free (no
//! `tempfile` crate).

use iffcomp::{compile, Mode, Options};
use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::Mutex;

// Both tests modify process-wide CWD state; run them serialized so they
// don't race under `cargo test`'s default multi-thread runner. Use
// `lock_or_recover` below instead of `.lock().unwrap()` so a panic in one
// test doesn't cascade-fail subsequent tests via mutex poisoning.
static CWD_LOCK: Mutex<()> = Mutex::new(());

fn lock_cwd() -> std::sync::MutexGuard<'static, ()> {
    match CWD_LOCK.lock() {
        Ok(g) => g,
        Err(poisoned) => poisoned.into_inner(),
    }
}

/// Returns the wftools/iffcomp-rs/testdata directory (via CARGO_MANIFEST_DIR).
fn testdata_dir() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("testdata")
}

/// Scope guard: restores the original working directory on Drop.
struct ChdirGuard {
    original: PathBuf,
}

impl ChdirGuard {
    fn new(to: &Path) -> Self {
        let original = env::current_dir().expect("current_dir");
        env::set_current_dir(to).expect("set_current_dir");
        ChdirGuard { original }
    }
}

impl Drop for ChdirGuard {
    fn drop(&mut self) {
        let _ = env::set_current_dir(&self.original);
    }
}

/// Make a unique temp path for the test's output file. Removed on scope exit
/// via a second tiny RAII helper.
struct TempPath(PathBuf);

impl TempPath {
    fn new(label: &str) -> Self {
        let pid = std::process::id();
        let nanos = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        let p = env::temp_dir().join(format!("iffcomp-rs-{}-{}-{}.bin", label, pid, nanos));
        TempPath(p)
    }
}

impl Drop for TempPath {
    fn drop(&mut self) {
        let _ = fs::remove_file(&self.0);
    }
}

#[test]
fn byte_exact_binary_vs_cpp() {
    let _lock = lock_cwd();
    let _cwd = ChdirGuard::new(&testdata_dir());
    let out = TempPath::new("binary");

    compile(
        "test.iff.txt",
        &out.0,
        Options {
            mode: Mode::Binary,
            verbose: false,
        },
    )
    .expect("compile binary");

    let got = fs::read(&out.0).expect("read Rust output");
    let want = fs::read("expected.iff").expect("read reference");
    assert_bytes_eq(&got, &want, "binary");
}

#[test]
fn byte_exact_text_vs_cpp() {
    let _lock = lock_cwd();
    let _cwd = ChdirGuard::new(&testdata_dir());
    let out = TempPath::new("text");

    compile(
        "test.iff.txt",
        &out.0,
        Options {
            mode: Mode::Text,
            verbose: false,
        },
    )
    .expect("compile text");

    let got = fs::read(&out.0).expect("read Rust output");
    let want = fs::read("expected.iff.txt").expect("read reference");
    assert_bytes_eq(&got, &want, "text");
}

/// Comprehensive torture test against `all_features.iff.txt` — exercises
/// nested chunks, sizeof/offsetof (forward and backward), hex literals,
/// state-push blocks, string escapes, padded strings, precision overrides,
/// `.align` / `.fillchar`, file inclusion with slicing, and `.timestamp`.
///
/// The `.timestamp` output is masked in both the got and want buffers
/// because the value is time-varying: the C++ oracle was generated at
/// build time with one unix epoch, and the Rust port runs at test time
/// with another. See [`mask_timestamp`] for the zeroing logic.
#[test]
fn byte_exact_all_features_vs_cpp() {
    let _lock = lock_cwd();
    let _cwd = ChdirGuard::new(&testdata_dir());
    let out = TempPath::new("all_features");

    compile(
        "all_features.iff.txt",
        &out.0,
        Options {
            mode: Mode::Binary,
            verbose: false,
        },
    )
    .expect("compile all_features");

    let mut got = fs::read(&out.0).expect("read Rust output");
    let mut want = fs::read("all_features.iff").expect("read reference");

    mask_timestamp(&mut got);
    mask_timestamp(&mut want);

    assert_bytes_eq(&got, &want, "all_features");
}

/// Find the `TIME\x04\x00\x00\x00` chunk header at a 4-aligned offset and
/// zero the next 4 bytes (the time_t payload). Only the first match is
/// masked; the sequence is specific enough that accidental matches
/// elsewhere in the payload are unlikely, but scanning on 4-byte
/// boundaries further reduces the chance.
fn mask_timestamp(data: &mut [u8]) {
    let pattern = b"TIME\x04\x00\x00\x00";
    let mut i = 0;
    while i + pattern.len() <= data.len() {
        if &data[i..i + pattern.len()] == pattern {
            let ts_start = i + pattern.len();
            let ts_end = (ts_start + 4).min(data.len());
            for j in ts_start..ts_end {
                data[j] = 0;
            }
            return;
        }
        i += 4;
    }
}

fn assert_bytes_eq(got: &[u8], want: &[u8], label: &str) {
    if got == want {
        eprintln!("{} byte-exact match: {} bytes", label, got.len());
        return;
    }
    let n = got.len().min(want.len());
    let mut diff_at = None;
    for i in 0..n {
        if got[i] != want[i] {
            diff_at = Some(i);
            break;
        }
    }
    if diff_at.is_none() && got.len() != want.len() {
        diff_at = Some(n);
    }
    let off = diff_at.unwrap_or(0);
    panic!(
        "{} mismatch at offset 0x{:x} ({}): got.len={} want.len={}\n{}",
        label,
        off,
        off,
        got.len(),
        want.len(),
        hex_diff_context(got, want, off, 16)
    );
}

fn hex_diff_context(got: &[u8], want: &[u8], center: usize, radius: usize) -> String {
    let lo = center.saturating_sub(radius);
    let hi = center + radius;
    let mut out = String::new();
    for (label, data) in [("got ", got), ("want", want)] {
        out.push_str(&format!("  {} @ 0x{:x}:", label, lo));
        let end = hi.min(data.len());
        for i in lo..end {
            let sep = if i == center { '>' } else { ' ' };
            out.push_str(&format!("{}{:02x}", sep, data[i]));
        }
        out.push('\n');
    }
    out
}
