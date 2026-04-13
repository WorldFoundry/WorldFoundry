//! oas2oad — compile a World Foundry `.oas` file to a binary `.oad`.
//!
//! Usage:
//!   oas2oad-rs [--prep=<path>] [--types=<path>] [--gpp=<path>] [-o <outfile>] <infile.oas>
//!
//! Pipeline:
//!   1. Shell out to `prep` to expand macros → C source with struct initializers.
//!   2. Fix the C source for gcc compatibility (strip Watcom `huge` keyword,
//!      replace pigtool.h/oad.h with a portable 64-bit-safe equivalent).
//!   3. Compile with `g++` to an object file.
//!   4. Extract the `.data` section with `objcopy` → binary `.oad`.
//!
//! This mirrors what the original Windows pipeline did (wpp → wlink → exe2bin),
//! keeping the compiler responsible for struct layout rather than reimplementing it.

use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::{self, Command};

// gcc-compatible, 64-bit-safe replacement for pigtool.h + oad.h.
// Uses int32_t/int16_t so struct layout is correct on both 32- and 64-bit hosts.
const OAS_COMPAT_H: &str = r#"
// oas_compat.h — portable replacement for pigtool.h + oad.h
#pragma once
#include <stdint.h>

typedef int8_t   int8;
typedef uint8_t  uint8;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;

#define BUTTON_FIXED16              0
#define BUTTON_FIXED32              1
#define BUTTON_INT8                 2
#define BUTTON_INT16                3
#define BUTTON_INT32                4
#define BUTTON_STRING               5
#define BUTTON_OBJECT_REFERENCE     6
#define BUTTON_FILENAME             7
#define BUTTON_PROPERTY_SHEET       8
#define LEVELCONFLAG_NOINSTANCES    9
#define LEVELCONFLAG_NOMESH        10
#define LEVELCONFLAG_SINGLEINSTANCE 11
#define LEVELCONFLAG_TEMPLATE      12
#define LEVELCONFLAG_EXTRACTCAMERA 13
#define BUTTON_CAMERA_REFERENCE    14
#define BUTTON_LIGHT_REFERENCE     15
#define LEVELCONFLAG_ROOM          16
#define LEVELCONFLAG_COMMONBLOCK   17
#define LEVELCONFLAG_ENDCOMMON     18
#define BUTTON_MESHNAME            19
#define BUTTON_XDATA               20
#define BUTTON_EXTRACT_CAMERA      21
#define LEVELCONFLAG_EXTRACTCAMERANEW 22
#define BUTTON_WAVEFORM            23
#define BUTTON_CLASS_REFERENCE     24
#define BUTTON_GROUP_START         25
#define BUTTON_GROUP_STOP          26
#define LEVELCONFLAG_EXTRACTLIGHT  27
#define LEVELCONFLAG_SHORTCUT      28

typedef char buttonType;

#define SHOW_AS_N_A          0
#define SHOW_AS_NUMBER       1
#define SHOW_AS_SLIDER       2
#define SHOW_AS_TOGGLE       3
#define SHOW_AS_DROPMENU     4
#define SHOW_AS_RADIOBUTTONS 5
#define SHOW_AS_HIDDEN       6
#define SHOW_AS_COLOR        7
#define SHOW_AS_CHECKBOX     8
#define SHOW_AS_MAILBOX      9
#define SHOW_AS_COMBOBOX    10
#define SHOW_AS_TEXTEDITOR  11
#define SHOW_AS_FILENAME    12
#define SHOW_AS_VECTOR      0x80

typedef unsigned char visualRepresentation;

#define XDATA_IGNORE                  0
#define XDATA_COPY                    1
#define XDATA_OBJECTLIST              2
#define XDATA_CONTEXTUALANIMATIONLIST 3
#define XDATA_SCRIPT                  4
#define XDATA_CONVERSION_MAX          5

typedef char EConversionAction;

#pragma pack(1)

typedef struct _oadHeader {
    int32_t chunkId;
    int32_t chunkSize;
    char    name[68];
    int32_t version;
} _oadHeader;

typedef struct _typeDescriptor {
    buttonType type;
    char       name[64];
    int32_t    min;
    int32_t    max;
    int32_t    def;
    int16_t    len;
    char       string[512];
    visualRepresentation showAs;
    int16_t    x, y;
    char       helpMessage[128];
    union {
        struct {
            EConversionAction conversionAction;
            int32_t           bRequired;
            char              displayName[64];
            char              szEnableExpression[128];
            int32_t           rollUpLength;
        } xdata;
        struct { char pad[255]; } pad;
    };
    char lpstrFilter[512];
} typeDescriptor;

#pragma pack()

// File-filter strings from xdata.inc; needed when xdata.inc isn't in the include chain.
#define BITMAP_FILESPEC "Windows Bitmap Files (*.bmp)\0*.bmp\0Targa Bitmap Files (*.tga)\0*.tga\0SGI Bitmap Files (*.rgb,*.rgba,*.bw)\0*.rgb;*.rgba;*.bw\0"
#define MAP_FILESPEC    "Scrolling Map Files (*.map)\0*.bmp\0"

// Suppress the original includes if they appear in prep output.
#define OAD_H
#define _PIGTOOL_H
"#;

fn usage() -> ! {
    eprintln!("Usage: oas2oad [--prep=<path>] [--types=<path>] [--gpp=<path>] [-o <outfile>] <infile.oas>");
    process::exit(1);
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut prep_path: Option<PathBuf> = None;
    let mut types_path: Option<PathBuf> = None;
    let mut gpp_path: Option<PathBuf> = None;
    let mut out_path: Option<PathBuf> = None;
    let mut in_path: Option<PathBuf> = None;
    let mut iter = args[1..].iter().peekable();

    while let Some(arg) = iter.next() {
        if let Some(v) = arg.strip_prefix("--prep=") {
            prep_path = Some(PathBuf::from(v));
        } else if let Some(v) = arg.strip_prefix("--types=") {
            types_path = Some(PathBuf::from(v));
        } else if let Some(v) = arg.strip_prefix("--gpp=") {
            gpp_path = Some(PathBuf::from(v));
        } else if arg == "-o" {
            out_path = Some(PathBuf::from(iter.next().unwrap_or_else(|| usage())));
        } else if arg.starts_with('-') {
            eprintln!("oas2oad: unknown flag {arg}");
            usage();
        } else {
            in_path = Some(PathBuf::from(arg));
        }
    }

    let in_path = in_path.unwrap_or_else(|| usage());
    let out_path = out_path.unwrap_or_else(|| in_path.with_extension("oad"));
    let prep_bin = prep_path.unwrap_or_else(find_prep);
    let gpp_bin = gpp_path.unwrap_or_else(|| PathBuf::from("g++"));
    let types_s = types_path.unwrap_or_else(|| find_types3ds(&in_path));

    let stem = in_path
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or_else(|| {
            eprintln!("oas2oad: cannot determine stem from {}", in_path.display());
            process::exit(1);
        });

    // prep must run from the types3ds.s directory so @include types.h resolves.
    let types_dir = types_s.parent().unwrap_or(Path::new(".")).to_path_buf();
    let oas_abs = in_path.canonicalize().unwrap_or_else(|_| in_path.clone());
    let oas_in_types_dir = types_dir.join(format!("{stem}.oas"));
    let need_symlink = !oas_abs.starts_with(&types_dir);
    if need_symlink {
        let _ = fs::remove_file(&oas_in_types_dir);
        std::os::unix::fs::symlink(&oas_abs, &oas_in_types_dir).unwrap_or_else(|e| {
            eprintln!("oas2oad: symlink {}: {e}", oas_in_types_dir.display());
            process::exit(1);
        });
    }

    // Step 1: run prep → .pp
    let pp_path = types_dir.join(format!("{stem}.pp.tmp"));
    let status = Command::new(&prep_bin)
        .arg(format!("-DTYPEFILE_OAS={stem}"))
        .arg(types_s.file_name().unwrap_or(types_s.as_os_str()))
        .arg(&pp_path)
        .current_dir(&types_dir)
        .status()
        .unwrap_or_else(|e| {
            eprintln!("oas2oad: failed to run prep ({}): {e}", prep_bin.display());
            process::exit(1);
        });

    if need_symlink { let _ = fs::remove_file(&oas_in_types_dir); }

    if !status.success() {
        let _ = fs::remove_file(&pp_path);
        eprintln!("oas2oad: prep exited with {status}");
        process::exit(1);
    }

    // Step 2: fix up prep output for gcc
    //   - strip Watcom 'huge' keyword
    //   - strip original pigtool.h / oad.h includes (replaced by our compat header)
    //   - add __attribute__((aligned(1))) to tempstruct so header and entries
    //     are packed contiguously in .data with no inter-variable padding
    let pp_text = fs::read_to_string(&pp_path).unwrap_or_else(|e| {
        eprintln!("oas2oad: reading {}: {e}", pp_path.display());
        process::exit(1);
    });
    let _ = fs::remove_file(&pp_path);

    let fixed = pp_text
        .lines()
        .filter(|l| !l.contains("#include \"pigtool.h\"") && !l.contains("#include \"oad.h\""))
        .map(|l| l.replace(" huge ", " "))
        .map(|l| if l.contains("tempstruct[]") {
            l.replace("tempstruct[]", "tempstruct[] __attribute__((aligned(1)))")
        } else {
            l
        })
        .collect::<Vec<_>>()
        .join("\n");

    let fixed_pp_path = types_dir.join(format!("{stem}.fixed.tmp"));
    fs::write(&fixed_pp_path, &fixed).unwrap_or_else(|e| {
        eprintln!("oas2oad: writing fixed pp: {e}");
        process::exit(1);
    });

    // Write compat header to a temp file
    let compat_h_path = types_dir.join(format!("{stem}.oas_compat.h"));
    fs::write(&compat_h_path, OAS_COMPAT_H).unwrap_or_else(|e| {
        eprintln!("oas2oad: writing compat header: {e}");
        process::exit(1);
    });

    // Step 3: compile with g++
    let obj_path = types_dir.join(format!("{stem}.o.tmp"));
    let status = Command::new(&gpp_bin)
        .args(["-c", "-x", "c++"])
        .arg(format!("-include{}", compat_h_path.display()))
        // name_KIND comes from EActorKind in the generated objects.h (doesn't exist here).
        // DEFAULT_VISIBILITY defaults to 1 in movebloc.inc; mesh.oas omits the define.
        // Both values are cosmetic defaults; 0/1 match the prep fallback behavior.
        .arg("-Dname_KIND=0")
        .arg("-DDEFAULT_VISIBILITY=1")
        .arg("-w")
        .arg("-o").arg(&obj_path)
        .arg(&fixed_pp_path)
        .status()
        .unwrap_or_else(|e| {
            eprintln!("oas2oad: failed to run g++ ({}): {e}", gpp_bin.display());
            process::exit(1);
        });

    let _ = fs::remove_file(&fixed_pp_path);
    let _ = fs::remove_file(&compat_h_path);

    if !status.success() {
        let _ = fs::remove_file(&obj_path);
        eprintln!("oas2oad: g++ exited with {status}");
        process::exit(1);
    }

    // Step 4: extract .data section → .oad
    let status = Command::new("objcopy")
        .args(["--only-section=.data", "-O", "binary"])
        .arg(&obj_path)
        .arg(&out_path)
        .status()
        .unwrap_or_else(|e| {
            eprintln!("oas2oad: failed to run objcopy: {e}");
            process::exit(1);
        });

    let _ = fs::remove_file(&obj_path);

    if !status.success() {
        eprintln!("oas2oad: objcopy exited with {status}");
        process::exit(1);
    }
}

fn find_prep() -> PathBuf {
    if let Ok(exe) = env::current_exe() {
        if let Some(dir) = exe.parent().and_then(|d| d.parent()) {
            let candidate = dir.join("prep/prep");
            if candidate.exists() { return candidate; }
        }
    }
    PathBuf::from("prep")
}

fn find_types3ds(oas_path: &Path) -> PathBuf {
    let sibling = oas_path.with_file_name("types3ds.s");
    if sibling.exists() { return sibling; }

    if let Ok(wf_dir) = env::var("WF_DIR") {
        let candidate = PathBuf::from(wf_dir).join("wfsource/source/oas/types3ds.s");
        if candidate.exists() { return candidate; }
    }

    let mut cur = oas_path.parent();
    while let Some(dir) = cur {
        let candidate = dir.join("wfsource/source/oas/types3ds.s");
        if candidate.exists() { return candidate; }
        cur = dir.parent();
    }

    eprintln!(
        "oas2oad: cannot find types3ds.s (tried siblings, $WF_DIR, parent dirs).\n\
         Use --types=<path> to specify it."
    );
    process::exit(1);
}
