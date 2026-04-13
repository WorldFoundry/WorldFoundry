#lang racket/base
;; parser.rkt — recursive-descent parser for the iffcomp DSL.
;;
;; Mirrors iffcomp-go/parser.go one-for-one. Productions:
;;
;;   statement-list  → chunk+
;;   chunk           → '{' CHAR-LIT chunk-statement* '}'
;;   chunk-statement → chunk | .align | .fillchar | expr
;;   expr            → item ( ('+' | '-') item )*
;;   item            → state-push | REAL | INTEGER | string-list
;;                   | '[' STRING extract-spec* ']'
;;                   | CHAR-LIT | .timestamp
;;                   | .offsetof '(' chunk-spec (',' INTEGER)? ')'
;;                   | .sizeof   '(' chunk-spec ')'
;;   state-push      → '{' (Y|W|L | .precision '(' PREC-SPEC ')') expr-list '}'
;;   string-list     → STRING+
;;   chunk-spec      → ('::' CHAR-LIT)+

(require racket/match
         "lexer.rkt"
         "writer.rkt")

(provide parse!)

(define DEFAULT-PREC (sizespec 1 15 16))
(define DEFAULT-SIZE 1)

;; State stack entry: (cons size-override sizespec)
(define (make-state size prec) (cons size prec))
(define (state-size s) (car s))
(define (state-prec s) (cdr s))

;; ---------------------------------------------------------------------------
;; Public entry

(define (parse! lx w verbose?)
  (define states (list (make-state DEFAULT-SIZE DEFAULT-PREC)))
  (let loop ()
    (unless (tok:eof? (lex-peek lx 0))
      (parse-chunk! lx w states verbose?)
      (loop))))

;; ---------------------------------------------------------------------------
;; Helpers

(define (pos-str tok)
  (define p (cond [(tok:eof?        tok) (tok:eof-pos tok)]
                  [(tok:lbrace?     tok) (tok:lbrace-pos tok)]
                  [(tok:rbrace?     tok) (tok:rbrace-pos tok)]
                  [(tok:lparen?     tok) (tok:lparen-pos tok)]
                  [(tok:rparen?     tok) (tok:rparen-pos tok)]
                  [(tok:lbrack?     tok) (tok:lbrack-pos tok)]
                  [(tok:rbrack?     tok) (tok:rbrack-pos tok)]
                  [(tok:comma?      tok) (tok:comma-pos tok)]
                  [(tok:double-colon? tok) (tok:double-colon-pos tok)]
                  [(tok:plus?       tok) (tok:plus-pos tok)]
                  [(tok:minus?      tok) (tok:minus-pos tok)]
                  [(tok:size-y?     tok) (tok:size-y-pos tok)]
                  [(tok:size-w?     tok) (tok:size-w-pos tok)]
                  [(tok:size-l?     tok) (tok:size-l-pos tok)]
                  [(tok:integer?    tok) (tok:integer-pos tok)]
                  [(tok:real?       tok) (tok:real-pos tok)]
                  [(tok:string?     tok) (tok:string-pos tok)]
                  [(tok:char-lit?   tok) (tok:char-lit-pos tok)]
                  [(tok:prec-spec?  tok) (tok:prec-spec-pos tok)]
                  [(tok:timestamp?  tok) (tok:timestamp-pos tok)]
                  [(tok:align?      tok) (tok:align-pos tok)]
                  [(tok:offsetof?   tok) (tok:offsetof-pos tok)]
                  [(tok:sizeof?     tok) (tok:sizeof-pos tok)]
                  [(tok:fillchar?   tok) (tok:fillchar-pos tok)]
                  [(tok:precision?  tok) (tok:precision-pos tok)]
                  [(tok:start?      tok) (tok:start-pos tok)]
                  [(tok:length?     tok) (tok:length-pos tok)]
                  [else #f]))
  (if p (format "~a:~a:~a" (srcpos-file p) (srcpos-line p) (srcpos-col p)) "?"))

(define (tok-name tok)
  (cond [(tok:eof?         tok) "EOF"]
        [(tok:lbrace?      tok) "'{'"]
        [(tok:rbrace?      tok) "'}'"]
        [(tok:lparen?      tok) "'('"]
        [(tok:rparen?      tok) "')'"]
        [(tok:lbrack?      tok) "'['"]
        [(tok:rbrack?      tok) "']'"]
        [(tok:comma?       tok) "','"]
        [(tok:double-colon? tok) "'::'"]
        [(tok:plus?        tok) "'+'"]
        [(tok:minus?       tok) "'-'"]
        [(tok:size-y?      tok) "'Y'"]
        [(tok:size-w?      tok) "'W'"]
        [(tok:size-l?      tok) "'L'"]
        [(tok:integer?     tok) "INTEGER"]
        [(tok:real?        tok) "REAL"]
        [(tok:string?      tok) "STRING"]
        [(tok:char-lit?    tok) "CHAR_LIT"]
        [(tok:prec-spec?   tok) "PREC_SPEC"]
        [(tok:timestamp?   tok) ".timestamp"]
        [(tok:align?       tok) ".align"]
        [(tok:offsetof?    tok) ".offsetof"]
        [(tok:sizeof?      tok) ".sizeof"]
        [(tok:fillchar?    tok) ".fillchar"]
        [(tok:precision?   tok) ".precision"]
        [(tok:start?       tok) ".start"]
        [(tok:length?      tok) ".length"]
        [else "?"]))

(define (trace! verbose? rule lx)
  (when verbose?
    (define tok (lex-peek lx 0))
    (eprintf "parse: ~a @ ~a (lookahead=~a)\n" rule (pos-str tok) (tok-name tok))))

(define (expect! lx pred what)
  (define tok (lex-peek lx 0))
  (unless (pred tok)
    (error 'iffcomp "~a: expected ~a, got ~a" (pos-str tok) what (tok-name tok)))
  (lex-next! lx))

;; ---------------------------------------------------------------------------
;; chunk

(define (parse-chunk! lx w states verbose?)
  (trace! verbose? "chunk" lx)
  (expect! lx tok:lbrace? "'{' starting chunk")
  (define id-tok (expect! lx tok:char-lit? "chunk ID"))
  (enter-chunk! w (tok:char-lit-fourcc id-tok))
  (let loop ()
    (define t (lex-peek lx 0))
    (unless (or (tok:rbrace? t) (tok:eof? t))
      (parse-chunk-statement! lx w states verbose?)
      (loop)))
  (expect! lx tok:rbrace? "'}' closing chunk")
  (exit-chunk! w))

;; ---------------------------------------------------------------------------
;; chunk-statement

(define (parse-chunk-statement! lx w states verbose?)
  (define t (lex-peek lx 0))
  (cond
    [(tok:lbrace? t)
     (if (tok:char-lit? (lex-peek lx 1))
         (parse-chunk! lx w states verbose?)
         (parse-item! lx w states verbose?))]
    [(tok:align? t)   (parse-alignment! lx w)]
    [(tok:fillchar? t)(parse-fillchar! lx w)]
    [else             (parse-expr! lx w states verbose?)]))

(define (parse-alignment! lx w)
  (lex-next! lx) ; .align
  (expect! lx tok:lparen? "'(' after .align")
  (define n (expect! lx tok:integer? "integer in .align"))
  (expect! lx tok:rparen? "')' closing .align")
  (when (= (tok:integer-val n) 0)
    (error 'iffcomp "~a: .align(0) doesn't make sense" (pos-str n)))
  (align-fn! w (tok:integer-val n)))

(define (parse-fillchar! lx w)
  (lex-next! lx) ; .fillchar
  (expect! lx tok:lparen? "'(' after .fillchar")
  (define n (expect! lx tok:integer? "integer in .fillchar"))
  (expect! lx tok:rparen? "')' closing .fillchar")
  (set-fill-char! w (tok:integer-val n)))

;; ---------------------------------------------------------------------------
;; expr

(define (parse-expr! lx w states verbose?)
  (define lhs (parse-item! lx w states verbose?))
  (let loop ([lhs lhs])
    (define t (lex-peek lx 0))
    (cond
      [(tok:plus?  t) (lex-next! lx) (loop (+ lhs (parse-item! lx w states verbose?)))]
      [(tok:minus? t) (lex-next! lx) (loop (- lhs (parse-item! lx w states verbose?)))]
      [else lhs])))

(define (parse-expr-list! lx w states verbose?)
  (let loop ()
    (define t (lex-peek lx 0))
    (unless (or (tok:rbrace? t) (tok:eof? t))
      (parse-item! lx w states verbose?)
      (loop))))

;; ---------------------------------------------------------------------------
;; item

(define (parse-item! lx w states verbose?)
  (trace! verbose? "item" lx)
  (define t (lex-peek lx 0))
  (cond
    [(tok:lbrace? t)
     (parse-state-push! lx w states verbose?)
     0]

    [(tok:real? t)
     (lex-next! lx)
     (define prec (or (tok:real-prec t) (state-prec (car states))))
     (out-fixed! w (tok:real-val t) prec)
     0]

    [(tok:integer? t)
     (lex-next! lx)
     (define width (let ([w* (tok:integer-width t)])
                     (if (= w* 0) (state-size (car states)) w*)))
     (define v (tok:integer-val t))
     (case width
       [(1) (when (> v #xFF)
              (error 'iffcomp "~a: value doesn't fit in int8" (pos-str t)))
            (out-int8! w v)]
       [(2) (when (> v #xFFFF)
              (error 'iffcomp "~a: value doesn't fit in int16" (pos-str t)))
            (out-int16! w v)]
       [(4) (when (> v #x7FFFFFFF)
              (error 'iffcomp "~a: value doesn't fit in int32" (pos-str t)))
            (out-int32! w v)]
       [else (error 'iffcomp "~a: bad width ~a" (pos-str t) width)])
     v]

    [(tok:string? t)
     (parse-string-list! lx w)
     0]

    [(tok:lbrack? t)
     (parse-file-include! lx w)
     0]

    [(tok:char-lit? t)
     (lex-next! lx)
     (out-id! w (tok:char-lit-fourcc t))
     0]

    [(tok:timestamp? t)
     (lex-next! lx)
     (out-timestamp! w (inexact->exact (floor (/ (current-inexact-milliseconds) 1000))))
     0]

    [(tok:offsetof? t)
     (parse-offsetof! lx w)
     0]

    [(tok:sizeof? t)
     (parse-sizeof! lx w)
     0]

    [else
     (error 'iffcomp "~a: unexpected ~a in item" (pos-str t) (tok-name t))]))

;; ---------------------------------------------------------------------------
;; state-push

(define (parse-state-push! lx w states verbose?)
  (lex-next! lx) ; '{'
  (define cur (car states))
  (define new-state
    (let ([t (lex-peek lx 0)])
      (cond
        [(tok:size-y? t) (lex-next! lx) (make-state 1 (state-prec cur))]
        [(tok:size-w? t) (lex-next! lx) (make-state 2 (state-prec cur))]
        [(tok:size-l? t) (lex-next! lx) (make-state 4 (state-prec cur))]
        [(tok:precision? t)
         (lex-next! lx)
         (expect! lx tok:lparen? "'(' after .precision")
         (define ps (expect! lx tok:prec-spec? "precision specifier"))
         (expect! lx tok:rparen? "')' closing .precision")
         (make-state (state-size cur)
                     (sizespec (tok:prec-spec-sign ps)
                               (tok:prec-spec-whole ps)
                               (tok:prec-spec-frac ps)))]
        [else
         (error 'iffcomp "~a: expected Y/W/L or .precision inside '{ … }' block"
                (pos-str (lex-peek lx 0)))])))
  (parse-expr-list! lx w (cons new-state (cdr states)) verbose?)
  (expect! lx tok:rbrace? "'}' closing state push"))

;; ---------------------------------------------------------------------------
;; string-list

(define (parse-string-list! lx w)
  (define first (lex-next! lx))
  (if (> (tok:string-size-override first) 0)
      (out-string-pad! w (tok:string-body first) (tok:string-size-override first))
      (out-string! w (tok:string-body first)))
  (let loop ()
    (when (tok:string? (lex-peek lx 0))
      (define next (lex-next! lx))
      (out-string-continue! w (tok:string-body next))
      (loop))))

;; ---------------------------------------------------------------------------
;; file include

(define (parse-file-include! lx w)
  (lex-next! lx) ; '['
  (define path-tok (expect! lx tok:string? "filename string in '[ … ]'"))
  (define start 0)
  (define len   +inf.0)
  (let loop ()
    (define t (lex-peek lx 0))
    (cond
      [(tok:start? t)
       (lex-next! lx)
       (expect! lx tok:lparen? "'(' after .start")
       (define n (expect! lx tok:integer? "integer in .start"))
       (expect! lx tok:rparen? "')' closing .start")
       (set! start (tok:integer-val n))
       (loop)]
      [(tok:length? t)
       (lex-next! lx)
       (expect! lx tok:lparen? "'(' after .length")
       (define n (expect! lx tok:integer? "integer in .length"))
       (expect! lx tok:rparen? "')' closing .length")
       (set! len (tok:integer-val n))
       (loop)]))
  (expect! lx tok:rbrack? "']' closing file include")
  (out-file! w (tok:string-body path-tok) start len))

;; ---------------------------------------------------------------------------
;; chunk specifier  ::CHAR_LIT ('::' CHAR_LIT)*

(define (parse-chunk-specifier! lx)
  (define parts
    (let loop ([acc '()])
      (if (tok:double-colon? (lex-peek lx 0))
          (let ()
            (lex-next! lx)
            (define id (expect! lx tok:char-lit? "CHAR_LIT after '::'"))
            (loop (cons (string-append "::'" (id-name (tok:char-lit-fourcc id)) "'") acc)))
          (reverse acc))))
  (when (null? parts)
    (error 'iffcomp "~a: expected chunkSpecifier starting with '::'"
           (pos-str (lex-peek lx 0))))
  (apply string-append parts))

;; ---------------------------------------------------------------------------
;; .offsetof / .sizeof

(define (parse-offsetof! lx w)
  (lex-next! lx) ; .offsetof
  (expect! lx tok:lparen? "'(' after .offsetof")
  (define path (parse-chunk-specifier! lx))
  (define addend
    (if (tok:comma? (lex-peek lx 0))
        (begin
          (lex-next! lx)
          (tok:integer-val (expect! lx tok:integer? "addend integer in .offsetof")))
        0))
  (expect! lx tok:rparen? "')' closing .offsetof")
  (emit-offsetof! w path addend))

(define (parse-sizeof! lx w)
  (lex-next! lx) ; .sizeof
  (expect! lx tok:lparen? "'(' after .sizeof")
  (define path (parse-chunk-specifier! lx))
  (expect! lx tok:rparen? "')' closing .sizeof")
  (emit-sizeof! w path))
