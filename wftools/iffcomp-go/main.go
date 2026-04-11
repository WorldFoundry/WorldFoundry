// main.go — CLI wrapper matching the C++ iffcomp switches closely enough for
// drop-in substitution in existing makefiles. The -ascii output mode is not
// implemented; binary is the only sink.

package main

import (
	"flag"
	"fmt"
	"os"
	"strings"
)

func main() {
	// The C++ tool uses `-o=<file>` (with equals sign) rather than `-o <file>`.
	// Go's flag package accepts both forms for `-o=X` and `-o X`, but it
	// expects the flag name to be exactly `o`. Pre-process argv so both
	// `-o=x`, `-o x`, and `--o=x` all work.
	args := preprocessArgs(os.Args[1:])

	fs := flag.NewFlagSet("iffcomp-go", flag.ExitOnError)
	outFile := fs.String("o", "test.wf", "output file")
	bin := fs.Bool("binary", true, "binary output (default)")
	asc := fs.Bool("ascii", false, "ascii / text output (IffWriterText-compatible)")
	verbose := fs.Bool("v", false, "verbose (trace parse productions to stderr)")
	quiet := fs.Bool("q", false, "quiet (suppress informational output)")
	_ = bin
	_ = quiet

	if err := fs.Parse(args); err != nil {
		os.Exit(2)
	}

	if fs.NArg() != 1 {
		fmt.Fprintln(os.Stderr, "usage: iffcomp-go [-o=file] [-binary|-ascii] [-v] <input>")
		os.Exit(1)
	}

	mode := ModeBinary
	if *asc {
		mode = ModeText
	}

	n, err := Compile(fs.Arg(0), *outFile, Options{Mode: mode, Verbose: *verbose})
	if err != nil {
		fmt.Fprintln(os.Stderr, "iffcomp-go:", err)
		os.Exit(10)
	}
	if *verbose {
		fmt.Fprintf(os.Stderr, "wrote %d bytes to %s\n", n, *outFile)
	}
}

// preprocessArgs normalizes `-o=x` into what flag expects. Go's flag already
// accepts both `-o x` and `-o=x`, so in practice this is a no-op today — but
// having it in one place keeps the CLI surface obvious.
func preprocessArgs(in []string) []string {
	out := make([]string, 0, len(in))
	for _, a := range in {
		if strings.HasPrefix(a, "--") {
			a = a[1:] // normalize --flag to -flag
		}
		out = append(out, a)
	}
	return out
}
