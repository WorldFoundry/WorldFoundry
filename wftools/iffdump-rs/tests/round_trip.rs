//! Round-trip integration test: `iffdump -f-` output should recompile to
//! byte-identical IFF (modulo the TIME chunk timestamp).

use iffdump::{Opts, default_wrappers, dump};
use std::path::Path;

/// Zero out the 4 timestamp bytes that follow a `TIME\x04\x00\x00\x00` header.
fn mask_timestamp(data: &mut Vec<u8>) {
    let pattern = b"TIME\x04\x00\x00\x00";
    let mut i = 0;
    while i + pattern.len() + 4 <= data.len() {
        if &data[i..i + pattern.len()] == pattern {
            let ts_start = i + pattern.len();
            data[ts_start..ts_start + 4].fill(0);
            i += pattern.len() + 4;
        } else {
            i += 1;
        }
    }
}

fn round_trip(iff_path: &Path) {
    // 1. Load original IFF and mask its timestamp.
    let mut original = std::fs::read(iff_path).expect("read original");
    mask_timestamp(&mut original);

    // 2. Dump to iffcomp source using -f- mode.
    let wrappers = default_wrappers();
    let opts = Opts { use_hdump: false, dump_binary: true, chars_per_line: 16 };
    let mut source = Vec::new();
    dump(&original, iff_path, None, &wrappers, &opts, &mut source).expect("dump");

    // 3. Recompile with iffcomp.
    let source_str = String::from_utf8(source).expect("utf8");
    let tmp_src = tempfile::NamedTempFile::new().expect("tmp src");
    let tmp_iff = tempfile::NamedTempFile::new().expect("tmp iff");
    std::fs::write(tmp_src.path(), &source_str).expect("write src");

    let status = std::process::Command::new(
        std::env::var("IFFCOMP_BIN")
            .unwrap_or_else(|_| "../iffcomp-rs/target/debug/iffcomp".to_string()),
    )
    .args([
        "-binary",
        &format!("-o={}", tmp_iff.path().display()),
        tmp_src.path().to_str().unwrap(),
    ])
    .status()
    .expect("run iffcomp");
    assert!(status.success(), "iffcomp failed");

    // 4. Compare bytes.
    let mut recompiled = std::fs::read(tmp_iff.path()).expect("read recompiled");
    mask_timestamp(&mut recompiled);

    assert_eq!(
        original, recompiled,
        "round-trip mismatch for {}",
        iff_path.display()
    );
}

#[test]
fn round_trip_expected() {
    round_trip(Path::new("testdata/expected.iff"));
}

#[test]
fn round_trip_all_features() {
    round_trip(Path::new("testdata/all_features.iff"));
}
