#lang racket/base
;; lexer.rkt — hand-rolled tokenizer for the iffcomp DSL.
;;
;; Mirrors iffcomp-go/lexer.go one-for-one. The include stack is a list of
;; frame structs; each frame holds the raw file bytes plus a mutable offset
;; and position. Using a byte-string rather than a port lets us roll back
;; speculatively during number/precision-triple parsing with a simple
;; set-frame-offset! rather than file-position.

(require racket/string
         racket/match
         racket/file)

(provide
 ;; token structs
 tok:eof tok:eof? tok:eof-pos
 tok:lbrace tok:lbrace? tok:lbrace-pos
 tok:rbrace tok:rbrace? tok:rbrace-pos
 tok:lparen tok:lparen? tok:lparen-pos
 tok:rparen tok:rparen? tok:rparen-pos
 tok:lbrack tok:lbrack? tok:lbrack-pos
 tok:rbrack tok:rbrack? tok:rbrack-pos
 tok:comma tok:comma? tok:comma-pos
 tok:double-colon tok:double-colon? tok:double-colon-pos
 tok:plus tok:plus? tok:plus-pos
 tok:minus tok:minus? tok:minus-pos
 tok:size-y tok:size-y? tok:size-y-pos
 tok:size-w tok:size-w? tok:size-w-pos
 tok:size-l tok:size-l? tok:size-l-pos
 tok:integer tok:integer? tok:integer-pos tok:integer-val tok:integer-width
 tok:real tok:real? tok:real-pos tok:real-val tok:real-prec
 tok:string tok:string? tok:string-pos tok:string-body tok:string-size-override
 tok:char-lit tok:char-lit? tok:char-lit-pos tok:char-lit-fourcc
 tok:prec-spec tok:prec-spec? tok:prec-spec-pos tok:prec-spec-sign tok:prec-spec-whole tok:prec-spec-frac
 tok:timestamp tok:timestamp? tok:timestamp-pos
 tok:align tok:align? tok:align-pos
 tok:offsetof tok:offsetof? tok:offsetof-pos
 tok:sizeof tok:sizeof? tok:sizeof-pos
 tok:fillchar tok:fillchar? tok:fillchar-pos
 tok:precision tok:precision? tok:precision-pos
 tok:start tok:start? tok:start-pos
 tok:length tok:length? tok:length-pos
 ;; sizespec
 (struct-out sizespec)
 default-precision
 ;; srcpos
 (struct-out srcpos)
 ;; lexer API
 make-lexer
 lex-peek
 lex-next!)

;; ---------------------------------------------------------------------------
;; Source position

(struct srcpos (file line col) #:transparent)

(define (srcpos->string p)
  (format "~a:~a:~a" (srcpos-file p) (srcpos-line p) (srcpos-col p)))

;; ---------------------------------------------------------------------------
;; Precision spec

(struct sizespec (sign whole frac) #:transparent)

(define default-precision (sizespec 1 15 16))

;; ---------------------------------------------------------------------------
;; Token structs — one per family, transparent for match

(struct tok:eof        (pos)                       #:transparent)
(struct tok:lbrace     (pos)                       #:transparent)
(struct tok:rbrace     (pos)                       #:transparent)
(struct tok:lparen     (pos)                       #:transparent)
(struct tok:rparen     (pos)                       #:transparent)
(struct tok:lbrack     (pos)                       #:transparent)
(struct tok:rbrack     (pos)                       #:transparent)
(struct tok:comma      (pos)                       #:transparent)
(struct tok:double-colon (pos)                     #:transparent)
(struct tok:plus       (pos)                       #:transparent)
(struct tok:minus      (pos)                       #:transparent)
(struct tok:size-y     (pos)                       #:transparent)
(struct tok:size-w     (pos)                       #:transparent)
(struct tok:size-l     (pos)                       #:transparent)
(struct tok:integer    (pos val width)             #:transparent)
(struct tok:real       (pos val prec)              #:transparent) ; prec = #f or sizespec
(struct tok:string     (pos body size-override)    #:transparent) ; size-override = 0 means none
(struct tok:char-lit   (pos fourcc)                #:transparent) ; packed u32 MSB-first
(struct tok:prec-spec  (pos sign whole frac)       #:transparent)
(struct tok:timestamp  (pos)                       #:transparent)
(struct tok:align      (pos)                       #:transparent)
(struct tok:offsetof   (pos)                       #:transparent)
(struct tok:sizeof     (pos)                       #:transparent)
(struct tok:fillchar   (pos)                       #:transparent)
(struct tok:precision  (pos)                       #:transparent)
(struct tok:start      (pos)                       #:transparent)
(struct tok:length     (pos)                       #:transparent)

;; ---------------------------------------------------------------------------
;; Frame — one file on the include stack

(struct frame
  (src        ; bytes — full file contents
   [offset #:mutable]
   [line   #:mutable]
   [col    #:mutable]
   filename)
  #:transparent)

(define (make-frame filename)
  (define src
    (with-handlers ([exn:fail:filesystem?
                     (λ (e) (error 'iffcomp "open ~a: ~a" filename (exn-message e)))])
      (file->bytes filename)))
  (frame src 0 1 1 filename))

;; ---------------------------------------------------------------------------
;; Lexer

(struct lexer
  ([stack     #:mutable]   ; list of frame, head = current
   [lookahead #:mutable])  ; list of buffered tokens
  #:transparent)

(define (make-lexer filename)
  (lexer (list (make-frame filename)) '()))

;; --- include stack ----------------------------------------------------------

(define (lex-push-file! lx filename)
  (set-lexer-stack! lx (cons (make-frame filename) (lexer-stack lx))))

(define (lex-push-system-file! lx name)
  (define dir (getenv "WF_DIR"))
  (unless dir (error 'iffcomp "include <~a>: WF_DIR not set" name))
  (lex-push-file! lx (build-path dir name)))

(define (cur lx)
  (and (not (null? (lexer-stack lx))) (car (lexer-stack lx))))

;; --- raw byte access --------------------------------------------------------

(define (peek-byte* lx)
  (let loop ()
    (define f (cur lx))
    (cond
      [(not f) -1]
      [(< (frame-offset f) (bytes-length (frame-src f)))
       (bytes-ref (frame-src f) (frame-offset f))]
      [(null? (cdr (lexer-stack lx))) -1]
      [else
       (set-lexer-stack! lx (cdr (lexer-stack lx)))
       (loop)])))

(define (read-byte* lx)
  (define b (peek-byte* lx))
  (when (>= b 0)
    (define f (cur lx))
    (set-frame-offset! f (+ (frame-offset f) 1))
    (if (= b 10) ; \n
        (begin (set-frame-line! f (+ (frame-line f) 1))
               (set-frame-col!  f 1))
        (set-frame-col! f (+ (frame-col f) 1))))
  b)

(define (current-pos lx)
  (define f (cur lx))
  (if f
      (srcpos (frame-filename f) (frame-line f) (frame-col f))
      (srcpos "" 0 0)))

;; Peek at offset k from current position within the current frame only.
(define (peek-at lx k)
  (define f (cur lx))
  (if (and f (< (+ (frame-offset f) k) (bytes-length (frame-src f))))
      (bytes-ref (frame-src f) (+ (frame-offset f) k))
      0))

;; ---------------------------------------------------------------------------
;; Public API

(define (lex-peek lx k)
  (let loop ()
    (when (<= (length (lexer-lookahead lx)) k)
      (set-lexer-lookahead! lx (append (lexer-lookahead lx) (list (scan! lx))))
      (loop)))
  (list-ref (lexer-lookahead lx) k))

(define (lex-next! lx)
  (if (not (null? (lexer-lookahead lx)))
      (let ([t (car (lexer-lookahead lx))])
        (set-lexer-lookahead! lx (cdr (lexer-lookahead lx)))
        t)
      (scan! lx)))

;; ---------------------------------------------------------------------------
;; Scan

(define (digit? b)    (and (>= b 48) (<= b 57)))
(define (hexdig? b)   (or (digit? b) (and (>= b 65) (<= b 70)) (and (>= b 97) (<= b 102))))
(define (identchar? b)(or (and (>= b 97) (<= b 122))
                           (and (>= b 65) (<= b 90))
                           (digit? b) (= b 95)))

(define (scan! lx)
  ;; Skip whitespace and // comments.
  (let loop ()
    (define b (peek-byte* lx))
    (cond
      [(memv b '(32 9 13 10)) (read-byte* lx) (loop)]
      [(= b 47) ; '/'
       (when (= (peek-at lx 1) 47)
         (let cloop ()
           (define c (read-byte* lx))
           (unless (or (= c -1) (= c 10)) (cloop)))
         (loop))]
      [else (void)]))

  (define sp (current-pos lx))
  (define b  (peek-byte* lx))

  (cond
    [(= b -1) (tok:eof sp)]

    ;; '.' — real or keyword
    [(= b 46)
     (if (digit? (peek-at lx 1))
         (scan-number! lx sp)
         (scan-dot-keyword! lx sp))]

    ;; 'include' — handle inline
    [(and (= b 105) ; 'i'
          (has-prefix? lx "include")
          (not (identchar? (peek-at lx 7))))
     (handle-include! lx)
     (scan! lx)]

    ;; Single-char tokens
    [(= b 123) (read-byte* lx) (tok:lbrace sp)]   ; {
    [(= b 125) (read-byte* lx) (tok:rbrace sp)]   ; }
    [(= b 40)  (read-byte* lx) (tok:lparen sp)]   ; (
    [(= b 41)  (read-byte* lx) (tok:rparen sp)]   ; )
    [(= b 91)  (read-byte* lx) (tok:lbrack sp)]   ; [
    [(= b 93)  (read-byte* lx) (tok:rbrack sp)]   ; ]
    [(= b 44)  (read-byte* lx) (tok:comma sp)]    ; ,
    [(= b 43)  (read-byte* lx) (tok:plus sp)]     ; +
    [(= b 89)  (read-byte* lx) (tok:size-y sp)]   ; Y
    [(= b 87)  (read-byte* lx) (tok:size-w sp)]   ; W
    [(= b 76)  (read-byte* lx) (tok:size-l sp)]   ; L
    [(= b 39)  (scan-char-lit! lx sp)]            ; '
    [(= b 34)  (scan-string! lx sp)]              ; "
    [(= b 58)                                      ; :
     (read-byte* lx)
     (if (= (peek-byte* lx) 58)
         (begin (read-byte* lx) (tok:double-colon sp))
         (error 'iffcomp "~a: expected '::' after ':'" (srcpos->string sp)))]

    ;; '-' — negative literal or binary minus
    [(= b 45)
     (define nxt (peek-at lx 1))
     (if (or (digit? nxt) (= nxt 46))
         (scan-number! lx sp)
         (begin (read-byte* lx) (tok:minus sp)))]

    ;; '$' — hex integer
    [(= b 36) (scan-hex-integer! lx sp)]

    ;; digit
    [(digit? b) (scan-number! lx sp)]

    [else
     (read-byte* lx)
     (error 'iffcomp "~a: unexpected character ~s" (srcpos->string sp) (integer->char b))]))

;; ---------------------------------------------------------------------------
;; has-prefix?

(define (has-prefix? lx p)
  (define f (cur lx))
  (and f
       (let ([src (frame-src f)] [off (frame-offset f)] [plen (string-length p)])
         (and (<= (+ off plen) (bytes-length src))
              (for/and ([i (in-range plen)])
                (= (bytes-ref src (+ off i)) (char->integer (string-ref p i))))))))

;; ---------------------------------------------------------------------------
;; include

(define (handle-include! lx)
  (for ([_ 7]) (read-byte* lx)) ; consume 'include'
  (let loop () (define b (peek-byte* lx)) (when (memv b '(32 9)) (read-byte* lx) (loop)))
  (define b (peek-byte* lx))
  (define-values (closer system?)
    (cond [(= b 34) (values 34 #f)]
          [(= b 60) (values 62 #t)]
          [else (error 'iffcomp "include: expected '\"' or '<', got ~s" (integer->char b))]))
  (read-byte* lx) ; opening quote
  (define name
    (let loop ([acc '()])
      (define c (read-byte* lx))
      (when (= c -1) (error 'iffcomp "include: unexpected EOF inside filename"))
      (if (= c closer)
          (list->string (map integer->char (reverse acc)))
          (loop (cons c acc)))))
  (if system?
      (lex-push-system-file! lx name)
      (lex-push-file! lx name)))

;; ---------------------------------------------------------------------------
;; dot keywords

(define (scan-dot-keyword! lx sp)
  (read-byte* lx) ; consume '.'
  (define ident
    (let loop ([acc '()])
      (define b (peek-byte* lx))
      (if (and (>= b 0) (identchar? b))
          (begin (read-byte* lx) (loop (cons b acc)))
          (list->string (map integer->char (reverse acc))))))
  (match ident
    ["timestamp" (tok:timestamp sp)]
    ["align"     (tok:align     sp)]
    ["offsetof"  (tok:offsetof  sp)]
    ["sizeof"    (tok:sizeof    sp)]
    ["fillchar"  (tok:fillchar  sp)]
    ["start"     (tok:start     sp)]
    ["length"    (tok:length    sp)]
    ["precision" (tok:precision sp)]
    [_ (error 'iffcomp "~a: unknown directive .~a" (srcpos->string sp) ident)]))

;; ---------------------------------------------------------------------------
;; char literal 'ABCD'

(define (scan-char-lit! lx sp)
  (read-byte* lx) ; opening '
  (define chars
    (let loop ([acc '()])
      (define b (peek-byte* lx))
      (cond
        [(= b -1) (error 'iffcomp "~a: unterminated char literal" (srcpos->string sp))]
        [(= b 39) (read-byte* lx) (reverse acc)]
        [(= (length acc) 4) (error 'iffcomp "~a: char literal too long" (srcpos->string sp))]
        [else (read-byte* lx) (loop (cons b acc))])))
  (when (null? chars) (error 'iffcomp "~a: empty char literal" (srcpos->string sp)))
  ;; Pack MSB-first into u32, right-pad with NUL.
  (define v
    (for/fold ([acc 0]) ([i (in-range 4)])
      (define c (if (< i (length chars)) (list-ref chars i) 0))
      (bitwise-ior acc (arithmetic-shift c (* 8 (- 3 i))))))
  (tok:char-lit sp v))

;; ---------------------------------------------------------------------------
;; string "..."

(define (scan-string! lx sp)
  (read-byte* lx) ; opening "
  (define body
    (let loop ([acc '()])
      (define b (peek-byte* lx))
      (cond
        [(or (= b -1) (= b 10)) (reverse acc)]   ; unterminated — permissive
        [(= b 34) (read-byte* lx) (reverse acc)]  ; closing "
        [(= b 92)                                  ; backslash
         (read-byte* lx)
         (define c (read-byte* lx))
         (when (= c -1) (error 'iffcomp "~a: unterminated escape" (srcpos->string sp)))
         (loop (cons c (cons 92 acc)))]            ; preserve \X literally
        [else (read-byte* lx) (loop (cons b acc))])))
  ;; Optional (N) size override immediately after closing quote.
  (define size-override
    (if (= (peek-byte* lx) 40) ; '('
        (let ([f (cur lx)])
          (define saved-off (frame-offset f))
          (define saved-line (frame-line f))
          (define saved-col (frame-col f))
          (set-frame-offset! f (+ saved-off 1))
          (set-frame-col! f (+ saved-col 1))
          (define num
            (let loop ([acc '()])
              (define c (peek-byte* lx))
              (if (and (>= c 0) (digit? c))
                  (begin (read-byte* lx) (loop (cons c acc)))
                  (reverse acc))))
          (if (and (not (null? num)) (= (peek-byte* lx) 41)) ; ')'
              (begin (read-byte* lx)
                     (string->number (list->string (map integer->char num))))
              (begin
                (set-frame-offset! f saved-off)
                (set-frame-line! f saved-line)
                (set-frame-col! f saved-col)
                0)))
        0))
  (tok:string sp (bytes->string/latin-1 (list->bytes body)) size-override))

;; ---------------------------------------------------------------------------
;; numbers

(define (scan-width-suffix! lx)
  (define b (peek-byte* lx))
  (cond [(= b 121) (read-byte* lx) 1]   ; y
        [(= b 119) (read-byte* lx) 2]   ; w
        [(= b 108) (read-byte* lx) 4]   ; l
        [else 0]))

(define (scan-hex-integer! lx sp)
  (read-byte* lx) ; consume '$'
  (define digits
    (let loop ([acc '()])
      (define b (peek-byte* lx))
      (if (and (>= b 0) (hexdig? b))
          (begin (read-byte* lx) (loop (cons b acc)))
          (reverse acc))))
  (when (null? digits) (error 'iffcomp "~a: empty hex literal" (srcpos->string sp)))
  (define v (string->number (list->string (map integer->char digits)) 16))
  (tok:integer sp v (scan-width-suffix! lx)))

(define (scan-number! lx sp)
  ;; Use an escape continuation so we can return early from two special cases:
  ;;   1. Bare N.N.N precision triple → tok:prec-spec
  ;;   2. Integer N followed by (M.M.M) → tok:real with explicit precision
  (call-with-current-continuation
   (λ (return)
     (define raw '())
     (define (push! b) (set! raw (cons b raw)))

     ;; Optional leading '-'
     (define negative?
       (if (= (peek-byte* lx) 45)
           (begin (push! (read-byte* lx)) #t)
           #f))

     ;; Leading digits (may be empty for ".5")
     (let loop ()
       (define b (peek-byte* lx))
       (when (and (>= b 0) (digit? b))
         (push! (read-byte* lx)) (loop)))

     ;; Speculative: bare N.N.N precision triple (no sign, two dots).
     ;; Pattern: digits '.' digits '.' digits  (no exponent, not negative).
     (when (and (not negative?) (= (peek-byte* lx) 46))
       (define f (cur lx))
       (define saved (list (frame-offset f) (frame-line f) (frame-col f)))
       (push! (read-byte* lx))              ; first '.'
       (define saw-digit? #f)
       (let loop ()
         (define b (peek-byte* lx))
         (when (and (>= b 0) (digit? b))
           (push! (read-byte* lx)) (set! saw-digit? #t) (loop)))
       (if (and saw-digit? (= (peek-byte* lx) 46))
           (let ()
             (push! (read-byte* lx))        ; second '.'
             (let loop ()
               (define b (peek-byte* lx))
               (when (and (>= b 0) (digit? b)) (push! (read-byte* lx)) (loop)))
             ;; Return precision triple immediately.
             (define raw-str (list->string (map integer->char (reverse raw))))
             (define parts (string-split raw-str "."))
             (define a  (string->number (car parts)))
             (define b  (string->number (cadr parts)))
             (define c  (string->number (caddr parts)))
             (define a* (if (or (< a 0) (> a 1)) 1 a))
             (return (tok:prec-spec sp a* b c)))
           ;; Not a triple.
           (begin
             ;; If no digit after first '.', rollback the dot entirely.
             (when (not saw-digit?)
               (set-frame-offset! f (car saved))
               (set-frame-line!   f (cadr saved))
               (set-frame-col!    f (caddr saved))
               (set! raw (cdr raw))))))        ; remove the '.' we pushed

     ;; Leading-dot real ".5" or fractional part after digits.
     (when (= (peek-byte* lx) 46)
       (push! (read-byte* lx))
       (let loop ()
         (define b (peek-byte* lx))
         (when (and (>= b 0) (digit? b)) (push! (read-byte* lx)) (loop))))

     ;; Optional exponent.
     (let ([b (peek-byte* lx)])
       (when (or (= b 101) (= b 69)) ; e E
         (push! (read-byte* lx))
         (let ([s (peek-byte* lx)])
           (when (or (= s 43) (= s 45)) (push! (read-byte* lx))))
         (let loop ()
           (define d (peek-byte* lx))
           (when (and (>= d 0) (digit? d)) (push! (read-byte* lx)) (loop)))))

     (define raw-str (list->string (map integer->char (reverse raw))))
     (define is-real? (regexp-match? #rx"[.eE]" raw-str))

     ;; Integer followed by (N.N.N) — reclassify as real with explicit precision.
     (when (and (not is-real?) (= (peek-byte* lx) 40))
       (define f (cur lx))
       (define saved (list (frame-offset f) (frame-line f) (frame-col f)))
       (read-byte* lx) ; '('
       (define triple (try-scan-prec-triple! lx))
       (if (and triple (= (peek-byte* lx) 41))
           (begin
             (read-byte* lx) ; ')'
             (return (tok:real sp (string->number raw-str) triple)))
           (begin
             (set-frame-offset! f (car saved))
             (set-frame-line!   f (cadr saved))
             (set-frame-col!    f (caddr saved)))))

     ;; Normal: real (with optional precision override) or integer.
     (if is-real?
         (let ([v (string->number raw-str)])
           ;; Optional explicit precision override (N.N.N).
           (define prec
             (if (= (peek-byte* lx) 40)
                 (let ([f     (cur lx)]
                       [saved (list (frame-offset (cur lx))
                                    (frame-line   (cur lx))
                                    (frame-col    (cur lx)))])
                   (read-byte* lx) ; '('
                   (define triple (try-scan-prec-triple! lx))
                   (if (and triple (= (peek-byte* lx) 41))
                       (begin (read-byte* lx) triple)
                       (begin
                         (set-frame-offset! f (car saved))
                         (set-frame-line!   f (cadr saved))
                         (set-frame-col!    f (caddr saved))
                         #f)))
                 #f))
           (tok:real sp v prec))
         (let* ([width (scan-width-suffix! lx)]
                [v     (string->number raw-str)])
           (tok:integer sp v width))))))

(define (try-scan-prec-triple! lx)
  (define (read-num!)
    (define digits
      (let loop ([acc '()])
        (define b (peek-byte* lx))
        (if (and (>= b 0) (digit? b))
            (begin (read-byte* lx) (loop (cons b acc)))
            (reverse acc))))
    (if (null? digits) #f (string->number (list->string (map integer->char digits)))))
  (define a (read-num!))
  (and a
       (= (peek-byte* lx) 46)
       (let ()
         (read-byte* lx) ; '.'
         (let ([b (read-num!)])
           (and b
                (= (peek-byte* lx) 46)
                (let ()
                  (read-byte* lx) ; '.'
                  (let ([c (read-num!)])
                    (and c
                         (let ([a* (if (or (< a 0) (> a 1)) 1 a)])
                           (sizespec a* b c))))))))))
