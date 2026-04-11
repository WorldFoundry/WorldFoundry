// iffcomp_test.go — end-to-end byte-exact diff against the C++ iffcomp output.
//
// The testdata directory has three files:
//
//   test.iff.txt  — the DSL input (copied verbatim from wftools/iffcomp/)
//   TODO          — a 230-byte file referenced by `[ "TODO" ]` in the input
//   expected.iff  — the reference binary output produced by the modernized
//                   C++ iffcomp against test.iff.txt. Regenerate with:
//                     (cd ../iffcomp && ./iffcomp -binary \
//                         -o=../iffcomp-go/testdata/expected.iff \
//                         ../iffcomp-go/testdata/test.iff.txt)
//
// The test compiles test.iff.txt with the Go port and fails on the first
// byte that differs, reporting offset + hex context on both sides.

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

func TestByteExactAgainstCpp(t *testing.T) {
	tmpDir := t.TempDir()
	outFile := filepath.Join(tmpDir, "out.iff")
	wantPath := "expected.iff" // relative to testdata/

	// `[ "TODO" ]` resolves relative to cwd, so run the compile from inside
	// testdata/. Absolute path for the output lets us stay consistent.
	origWd, err := os.Getwd()
	if err != nil {
		t.Fatalf("getwd: %v", err)
	}
	if err := os.Chdir("testdata"); err != nil {
		t.Fatalf("chdir testdata: %v", err)
	}
	t.Cleanup(func() { _ = os.Chdir(origWd) })

	if _, err := Compile("test.iff.txt", outFile, Options{Mode: ModeBinary}); err != nil {
		t.Fatalf("Compile: %v", err)
	}

	got, err := os.ReadFile(outFile)
	if err != nil {
		t.Fatalf("read Go output: %v", err)
	}
	want, err := os.ReadFile(wantPath)
	if err != nil {
		t.Fatalf("read reference: %v", err)
	}

	diffAt := -1
	n := len(got)
	if len(want) < n {
		n = len(want)
	}
	for i := 0; i < n; i++ {
		if got[i] != want[i] {
			diffAt = i
			break
		}
	}
	if diffAt == -1 && len(got) != len(want) {
		diffAt = n
	}

	if diffAt == -1 {
		t.Logf("byte-exact match: %d bytes", len(got))
		return
	}

	t.Fatalf("byte mismatch at offset 0x%x (%d):\n%s\n",
		diffAt, diffAt, hexDiffContext(got, want, diffAt, 16))
}

// TestTextOutputAgainstCpp diffs the Go -ascii output against the C++
// reference (testdata/expected.iff.txt). This is a byte-exact comparison —
// the formatting helpers are specified tightly enough that matching the
// IffWriterText output exactly is feasible.
func TestTextOutputAgainstCpp(t *testing.T) {
	tmpDir := t.TempDir()
	outFile := filepath.Join(tmpDir, "out.iff.txt")
	wantPath := "expected.iff.txt" // relative to testdata/

	origWd, err := os.Getwd()
	if err != nil {
		t.Fatalf("getwd: %v", err)
	}
	if err := os.Chdir("testdata"); err != nil {
		t.Fatalf("chdir testdata: %v", err)
	}
	t.Cleanup(func() { _ = os.Chdir(origWd) })

	if _, err := Compile("test.iff.txt", outFile, Options{Mode: ModeText}); err != nil {
		t.Fatalf("Compile (text): %v", err)
	}

	got, err := os.ReadFile(outFile)
	if err != nil {
		t.Fatalf("read Go text output: %v", err)
	}
	want, err := os.ReadFile(wantPath)
	if err != nil {
		t.Fatalf("read reference text: %v", err)
	}

	if string(got) == string(want) {
		t.Logf("text byte-exact match: %d bytes", len(got))
		return
	}

	diffAt := -1
	n := len(got)
	if len(want) < n {
		n = len(want)
	}
	for i := 0; i < n; i++ {
		if got[i] != want[i] {
			diffAt = i
			break
		}
	}
	if diffAt == -1 {
		diffAt = n
	}

	t.Fatalf("text mismatch at offset %d (len got=%d want=%d):\n%s",
		diffAt, len(got), len(want), hexDiffContext(got, want, diffAt, 24))
}

// hexDiffContext formats a side-by-side hex snippet of both buffers around
// the first mismatching offset, for readable test failures.
func hexDiffContext(got, want []byte, center, radius int) string {
	lo := center - radius
	if lo < 0 {
		lo = 0
	}
	hi := center + radius
	var b []byte = make([]byte, 0, 256)
	for _, row := range []struct {
		label string
		data  []byte
	}{
		{"got ", got},
		{"want", want},
	} {
		line := fmt.Sprintf("  %s @ 0x%x:", row.label, lo)
		for i := lo; i < hi && i < len(row.data); i++ {
			sep := ""
			if i == center {
				sep = ">"
			} else {
				sep = " "
			}
			line += fmt.Sprintf("%s%02x", sep, row.data[i])
		}
		line += "\n"
		b = append(b, line...)
	}
	return string(b)
}
