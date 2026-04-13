// iffcomp.go — Compile() is the library entry point: parse an .iff.txt file
// and produce a binary or text IFF tree on disk. main.go wraps this behind
// a CLI.

package main

import (
	"fmt"
	"os"
)

// Mode selects the output writer: binary (default, matches IffWriterBinary)
// or text (matches IffWriterText — indented, human-readable, not round-trippable
// with the binary format).
type Mode int

const (
	ModeBinary Mode = iota
	ModeText
)

// Options tunes Compile's behavior. Verbose enables per-production parse
// traces to stderr, matching the C++ tool's `-v` flag.
type Options struct {
	Mode    Mode
	Verbose bool
}

// Compile reads inFile, parses it as the iffcomp DSL, and writes the output
// (binary or text) to outFile. It returns the number of bytes written.
func Compile(inFile, outFile string, opts Options) (int, error) {
	lex, err := newLexer(inFile)
	if err != nil {
		return 0, err
	}
	var w *writer
	if opts.Mode == ModeText {
		w = newTextWriter()
	} else {
		w = newBinaryWriter()
	}
	p := newParser(lex, w)
	p.verbose = opts.Verbose

	if err := p.Parse(); err != nil {
		return 0, fmt.Errorf("parse %s: %w", inFile, err)
	}
	if err := w.ResolveBackpatches(); err != nil {
		return 0, err
	}
	data := w.Bytes()
	if err := os.WriteFile(outFile, data, 0o644); err != nil {
		return 0, fmt.Errorf("write %s: %w", outFile, err)
	}
	return len(data), nil
}
