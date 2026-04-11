// parser.go — recursive-descent parser for the iffcomp DSL.
//
// Mirrors the productions in wftools/iffcomp/lang.y one-for-one:
//
//     statement_list   -> chunk+
//     chunk            -> '{' CHAR_LITERAL chunk_statement* '}'
//     chunk_statement  -> chunk | alignment | fillchar | expr | (empty)
//     fillchar         -> '.fillchar' '(' INTEGER ')'
//     alignment        -> '.align'    '(' INTEGER ')'
//     expr             -> item ( ('+' | '-') item )*
//     item             -> state_push | REAL | INTEGER | string_list
//                      |  '[' STRING extractSpec* ']'
//                      |  CHAR_LITERAL | '.timestamp'
//                      |  '.offsetof' '(' chunkSpec (',' INTEGER)? ')'
//                      |  '.sizeof'   '(' chunkSpec ')'
//     state_push       -> '{' (precision_spec | size_spec) expr_list '}'
//     string_list      -> STRING (STRING)*
//     chunkSpecifier   -> '::' CHAR_LITERAL (':: CHAR_LITERAL)*
//     extractSpec      -> '.start' '(' INTEGER ')' | '.length' '(' INTEGER ')'
//
// The "chunk vs. state_push" disambiguation uses 2-token lookahead on LBRACE:
// if the next token after '{' is CHAR_LITERAL, it's a chunk; otherwise it's a
// state push. This is the same lookahead the bison grammar relies on.

package main

import (
	"fmt"
	"math"
	"os"
	"time"
)

type parseState struct {
	sizeOverride int
	precision    sizeSpec
}

type parser struct {
	lex *lexer
	w   *writer

	states []parseState

	// Per-chunk scratch state used by file-include and chunkSpec paths.
	startPosOverride uint64
	lengthOverride   uint64

	// verbose enables per-production trace logging to stderr, matching the
	// C++ tool's `-v` flag (which flipped bison's yydebug). The Go trace is
	// rule-entry-based rather than shift/reduce-based, but serves the same
	// debugging purpose.
	verbose bool
}

// trace logs a parser-rule entry to stderr when verbose is set.
func (p *parser) trace(rule string) {
	if !p.verbose {
		return
	}
	tok := p.peek()
	fmt.Fprintf(os.Stderr, "parse: %s @ %s (lookahead=%s)\n", rule, tok.pos, tokName(tok.kind))
}

func newParser(l *lexer, w *writer) *parser {
	return &parser{
		lex: l,
		w:   w,
		states: []parseState{
			{sizeOverride: 1, precision: sizeSpec{sign: 1, whole: 15, fraction: 16}},
		},
	}
}

func (p *parser) topState() parseState { return p.states[len(p.states)-1] }

func (p *parser) pushState(s parseState) { p.states = append(p.states, s) }

func (p *parser) popState() {
	if len(p.states) <= 1 {
		panic("popState on default stack")
	}
	p.states = p.states[:len(p.states)-1]
}

// --- token helpers ----------------------------------------------------------

func (p *parser) peek() token { return p.lex.peek(0) }

func (p *parser) peek1() token { return p.lex.peek(1) }

func (p *parser) consume() token {
	tok, _ := p.lex.next()
	return tok
}

func (p *parser) expect(k tokKind, what string) (token, error) {
	tok := p.peek()
	if tok.kind != k {
		return token{}, fmt.Errorf("%s: expected %s, got %s", tok.pos, what, tokName(tok.kind))
	}
	return p.consume(), nil
}

func tokName(k tokKind) string {
	switch k {
	case tokEOF:
		return "EOF"
	case tokLBrace:
		return "'{'"
	case tokRBrace:
		return "'}'"
	case tokLParen:
		return "'('"
	case tokRParen:
		return "')'"
	case tokLBrack:
		return "'['"
	case tokRBrack:
		return "']'"
	case tokComma:
		return "','"
	case tokDoubleColon:
		return "'::'"
	case tokPlus:
		return "'+'"
	case tokMinus:
		return "'-'"
	case tokSizeY:
		return "'Y'"
	case tokSizeW:
		return "'W'"
	case tokSizeL:
		return "'L'"
	case tokInteger:
		return "INTEGER"
	case tokReal:
		return "REAL"
	case tokString:
		return "STRING"
	case tokCharLit:
		return "CHAR_LITERAL"
	case tokPrecSpec:
		return "PRECISION_SPECIFIER"
	case tokTimestamp:
		return ".timestamp"
	case tokAlign:
		return ".align"
	case tokOffsetof:
		return ".offsetof"
	case tokSizeof:
		return ".sizeof"
	case tokFillChar:
		return ".fillchar"
	case tokStart:
		return ".start"
	case tokLength:
		return ".length"
	case tokPrecision:
		return ".precision"
	}
	return fmt.Sprintf("tok(%d)", k)
}

// --- top level --------------------------------------------------------------

// Parse walks the input until EOF, expecting one or more chunks.
func (p *parser) Parse() error {
	for p.peek().kind != tokEOF {
		if err := p.parseChunk(); err != nil {
			return err
		}
	}
	return nil
}

// --- chunk ------------------------------------------------------------------

func (p *parser) parseChunk() error {
	p.trace("chunk")
	if _, err := p.expect(tokLBrace, "'{' starting chunk"); err != nil {
		return err
	}
	idTok, err := p.expect(tokCharLit, "chunk ID")
	if err != nil {
		return err
	}
	p.w.enterChunk(idTok.charLit)
	for p.peek().kind != tokRBrace && p.peek().kind != tokEOF {
		if err := p.parseChunkStatement(); err != nil {
			return err
		}
	}
	if _, err := p.expect(tokRBrace, "'}' closing chunk"); err != nil {
		return err
	}
	p.w.exitChunk()
	return nil
}

// --- chunk_statement --------------------------------------------------------

func (p *parser) parseChunkStatement() error {
	tok := p.peek()
	switch tok.kind {
	case tokLBrace:
		// chunk vs. state_push: disambiguate on token[1].
		if p.peek1().kind == tokCharLit {
			return p.parseChunk()
		}
		// Fall through to expr, which handles state_push under item.
	case tokAlign:
		return p.parseAlignment()
	case tokFillChar:
		return p.parseFillChar()
	}
	// Everything else is an expression (which evaluates items for side effect).
	_, err := p.parseExpr()
	return err
}

func (p *parser) parseAlignment() error {
	p.consume() // .align
	if _, err := p.expect(tokLParen, "'(' after .align"); err != nil {
		return err
	}
	n, err := p.expect(tokInteger, "integer in .align")
	if err != nil {
		return err
	}
	if _, err := p.expect(tokRParen, "')' closing .align"); err != nil {
		return err
	}
	if n.intVal == 0 {
		return fmt.Errorf("%s: .align(0) doesn't make sense", n.pos)
	}
	p.w.alignFunction(int(n.intVal))
	return nil
}

func (p *parser) parseFillChar() error {
	p.consume() // .fillchar
	if _, err := p.expect(tokLParen, "'(' after .fillchar"); err != nil {
		return err
	}
	n, err := p.expect(tokInteger, "integer in .fillchar")
	if err != nil {
		return err
	}
	if _, err := p.expect(tokRParen, "')' closing .fillchar"); err != nil {
		return err
	}
	p.w.setFillChar(byte(n.intVal))
	return nil
}

// --- expr -------------------------------------------------------------------

// parseExpr parses `item ( ('+' | '-') item )*` and returns the accumulated
// value. The numeric result is discarded by the grammar at chunk-statement
// level — byte emission happens inside parseItem's cases.
func (p *parser) parseExpr() (uint64, error) {
	lhs, err := p.parseItem()
	if err != nil {
		return 0, err
	}
	for {
		op := p.peek().kind
		if op != tokPlus && op != tokMinus {
			break
		}
		p.consume()
		rhs, err := p.parseItem()
		if err != nil {
			return 0, err
		}
		if op == tokPlus {
			lhs += rhs
		} else {
			lhs -= rhs
		}
	}
	return lhs, nil
}

// parseExprList parses zero or more items inside a state_push block (i.e.
// `{ Y 1 2 3 }`). The original grammar's `expr_list : expr_list expr | item`
// is effectively "one or more items" and that's what we implement here; the
// loop terminates on the closing brace.
func (p *parser) parseExprList() error {
	for p.peek().kind != tokRBrace && p.peek().kind != tokEOF {
		if _, err := p.parseItem(); err != nil {
			return err
		}
	}
	return nil
}

// --- item -------------------------------------------------------------------

// parseItem returns the numeric value of the item for use in arithmetic
// expressions. Most items also have a side effect (writing bytes); the
// returned value is the current integer value for INTEGER items, 0 for others.
func (p *parser) parseItem() (uint64, error) {
	p.trace("item")
	tok := p.peek()
	switch tok.kind {
	case tokLBrace:
		return 0, p.parseStatePush()
	case tokReal:
		p.consume()
		prec := p.topState().precision
		if tok.realPrec != (sizeSpec{}) {
			prec = tok.realPrec
		}
		p.w.outFixed(tok.realVal, prec)
		return 0, nil
	case tokInteger:
		p.consume()
		width := tok.intWidth
		if width == 0 {
			width = p.topState().sizeOverride
		}
		v := tok.intVal
		switch width {
		case 1:
			p.w.outInt8(uint8(v))
			if v > 0xFF {
				return 0, fmt.Errorf("%s: value doesn't fit in int8", tok.pos)
			}
		case 2:
			if v > 0xFFFF {
				return 0, fmt.Errorf("%s: value doesn't fit in int16", tok.pos)
			}
			p.w.outInt16(uint16(v))
		case 4:
			if v > 0x7FFFFFFF {
				return 0, fmt.Errorf("%s: value doesn't fit in int32", tok.pos)
			}
			p.w.outInt32(uint32(v))
		default:
			return 0, fmt.Errorf("%s: bad width %d", tok.pos, width)
		}
		return v, nil
	case tokString:
		return 0, p.parseStringList()
	case tokLBrack:
		return 0, p.parseFileInclude()
	case tokCharLit:
		p.consume()
		p.w.outID(tok.charLit)
		return 0, nil
	case tokTimestamp:
		p.consume()
		p.w.outTimestamp(time.Now().Unix())
		return 0, nil
	case tokOffsetof:
		return 0, p.parseOffsetof()
	case tokSizeof:
		return 0, p.parseSizeof()
	}
	return 0, fmt.Errorf("%s: unexpected %s in item", tok.pos, tokName(tok.kind))
}

// parseStatePush handles the `{ (Y|W|L | .precision(S.W.F)) expr_list }`
// form. The lookahead has already decided this is a state push, not a chunk.
func (p *parser) parseStatePush() error {
	p.consume() // '{'
	newState := p.topState()
	switch p.peek().kind {
	case tokSizeY:
		p.consume()
		newState.sizeOverride = 1
	case tokSizeW:
		p.consume()
		newState.sizeOverride = 2
	case tokSizeL:
		p.consume()
		newState.sizeOverride = 4
	case tokPrecision:
		p.consume() // .precision
		if _, err := p.expect(tokLParen, "'(' after .precision"); err != nil {
			return err
		}
		ps, err := p.expect(tokPrecSpec, "precision specifier")
		if err != nil {
			return err
		}
		if _, err := p.expect(tokRParen, "')' closing .precision"); err != nil {
			return err
		}
		newState.precision = ps.precSpec
	default:
		return fmt.Errorf("%s: expected Y/W/L or .precision inside '{ … }' block", p.peek().pos)
	}
	p.pushState(newState)
	if err := p.parseExprList(); err != nil {
		return err
	}
	if _, err := p.expect(tokRBrace, "'}' closing state push"); err != nil {
		return err
	}
	p.popState()
	return nil
}

// parseStringList handles `"abc" "def" "ghi"` — adjacent string literals
// concatenate into one C string via out_string_continue. The `(N)` size
// override on the *first* string is honored; on continuation strings it is
// silently dropped, matching the original grammar's behavior.
func (p *parser) parseStringList() error {
	first := p.consume() // STRING
	if first.strSize > 0 {
		p.w.outStringPad(first.str, first.strSize)
	} else {
		p.w.outString(first.str)
	}
	for p.peek().kind == tokString {
		next := p.consume()
		p.w.outStringContinue(next.str)
	}
	return nil
}

// parseFileInclude handles `[ "path" (.start(N))? (.length(M))? ]`.
// The extract specifiers are optional and order-independent.
func (p *parser) parseFileInclude() error {
	p.consume() // '['
	// Reset scratch.
	p.startPosOverride = 0
	p.lengthOverride = math.MaxUint64

	pathTok, err := p.expect(tokString, "filename string in '[ … ]'")
	if err != nil {
		return err
	}
	for {
		k := p.peek().kind
		if k == tokStart || k == tokLength {
			if err := p.parseExtractSpec(); err != nil {
				return err
			}
			continue
		}
		break
	}
	if _, err := p.expect(tokRBrack, "']' closing file include"); err != nil {
		return err
	}
	return p.w.outFile(pathTok.str, p.startPosOverride, p.lengthOverride)
}

func (p *parser) parseExtractSpec() error {
	tok := p.consume() // .start or .length
	if _, err := p.expect(tokLParen, "'(' after extract spec"); err != nil {
		return err
	}
	n, err := p.expect(tokInteger, "integer in extract spec")
	if err != nil {
		return err
	}
	if _, err := p.expect(tokRParen, "')' closing extract spec"); err != nil {
		return err
	}
	switch tok.kind {
	case tokStart:
		p.startPosOverride = n.intVal
	case tokLength:
		p.lengthOverride = n.intVal
	}
	return nil
}

// parseChunkSpecifier accumulates `::'A'::'B'` into a single path key.
// The grammar production requires at least one `::` followed by a
// CHAR_LITERAL, and can chain.
func (p *parser) parseChunkSpecifier() (string, error) {
	var path string
	for p.peek().kind == tokDoubleColon {
		p.consume()
		id, err := p.expect(tokCharLit, "CHAR_LITERAL after '::'")
		if err != nil {
			return "", err
		}
		path += "::'" + idName(id.charLit) + "'"
	}
	if path == "" {
		return "", fmt.Errorf("%s: expected chunkSpecifier starting with '::'", p.peek().pos)
	}
	return path, nil
}

// parseOffsetof handles `.offsetof(::'A'::'B')` and `.offsetof(::'A'::'B', N)`.
func (p *parser) parseOffsetof() error {
	p.consume() // .offsetof
	if _, err := p.expect(tokLParen, "'(' after .offsetof"); err != nil {
		return err
	}
	path, err := p.parseChunkSpecifier()
	if err != nil {
		return err
	}
	addend := 0
	if p.peek().kind == tokComma {
		p.consume()
		n, err := p.expect(tokInteger, "addend integer in .offsetof")
		if err != nil {
			return err
		}
		addend = int(n.intVal)
	}
	if _, err := p.expect(tokRParen, "')' closing .offsetof"); err != nil {
		return err
	}
	p.w.emitOffsetof(path, addend)
	return nil
}

// parseSizeof handles `.sizeof(::'A'::'B')`.
func (p *parser) parseSizeof() error {
	p.consume() // .sizeof
	if _, err := p.expect(tokLParen, "'(' after .sizeof"); err != nil {
		return err
	}
	path, err := p.parseChunkSpecifier()
	if err != nil {
		return err
	}
	if _, err := p.expect(tokRParen, "')' closing .sizeof"); err != nil {
		return err
	}
	p.w.emitSizeof(path)
	return nil
}
