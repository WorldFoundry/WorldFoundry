#lang racket/base
;; writer.rkt — IFF binary/text writer.
;;
;; Mirrors iffcomp-go/writer.go. Uses a mutable bytes buffer (grows by
;; doubling) for binary mode and an output-string port for text mode.
;; Back-patches are resolved after the full parse with bytes-set!.
;;
;; Key invariants (from Go):
;;   - offsetof immediate: sym.payload-pos - 4  (= size_field_pos)
;;   - offsetof deferred:  sym.payload-pos - 8  (= id_pos)
;;   - sizeof always writes sym.payload-size

(require racket/match
         racket/string
         racket/format
         racket/list
         racket/file
         "lexer.rkt")

(provide
 make-binary-writer
 make-text-writer
 writer-bytes
 out-int8!  out-int16!  out-int32!
 align-internal!  align-fn!  set-fill-char!
 out-id!
 enter-chunk!  exit-chunk!
 emit-sizeof!  emit-offsetof!
 out-string!   out-string-continue!  out-string-pad!
 out-timestamp!  out-file!
 out-fixed!
 resolve-backpatches!
 id-name  build-path-key)

;; ---------------------------------------------------------------------------
;; Internal structs

(struct chunk-frame
  (id start-pos size-field-pos path-key)
  #:transparent)

(struct backpatch-rec
  (kind path addend write-pos)      ; kind: 'sizeof or 'offsetof
  #:transparent)

(struct wstate
  ([text?        #:mutable]
   [buf          #:mutable]   ; mutable bytes, binary mode
   [pos          #:mutable]   ; exact-integer
   [fill-char    #:mutable]   ; 0–255
   [stack        #:mutable]   ; list of chunk-frame (head = innermost)
   [path-ids     #:mutable]   ; list of u32, parallels stack
   [symbols      #:mutable]   ; hash: path-key → (cons payload-pos payload-size)
   [pending      #:mutable]   ; list of backpatch-rec
   [text-port    #:mutable]   ; output-string port (text mode)
   [text-on-line #:mutable])  ; byte count for 100-byte wrap in out-file
  #:transparent)

(define (make-binary-writer)
  (wstate #f (make-bytes 4096 0) 0 0 '() '() (make-hash) '() #f 0))

(define (make-text-writer)
  (wstate #t #f 0 0 '() '() (make-hash) '() (open-output-string) 0))

(define (writer-bytes w)
  (if (wstate-text? w)
      (string->bytes/latin-1 (get-output-string (wstate-text-port w)))
      (subbytes (wstate-buf w) 0 (wstate-pos w))))

;; ---------------------------------------------------------------------------
;; Buffer helpers (binary only)

(define (grow! w needed)
  (define buf (wstate-buf w))
  (when (> needed (bytes-length buf))
    (define new-cap
      (let loop ([c (* 2 (bytes-length buf))])
        (if (>= c needed) c (loop (* 2 c)))))
    (define new-buf (make-bytes new-cap 0))
    (bytes-copy! new-buf 0 buf 0 (wstate-pos w))
    (set-wstate-buf! w new-buf)))

(define (patch-bytes! w src off)
  (grow! w (+ off (bytes-length src)))
  (bytes-copy! (wstate-buf w) off src))

(define (wb! w b)
  (define pos (wstate-pos w))
  (grow! w (+ pos 1))
  (bytes-set! (wstate-buf w) pos b)
  (set-wstate-pos! w (+ pos 1)))

(define (wbs! w src)
  (define pos (wstate-pos w))
  (define len (bytes-length src))
  (grow! w (+ pos len))
  (bytes-copy! (wstate-buf w) pos src)
  (set-wstate-pos! w (+ pos len)))

;; ---------------------------------------------------------------------------
;; Primitive emitters

(define (out-int8! w v)
  (define vb (bitwise-and v #xFF))
  (if (wstate-text? w)
      (text-emit! w (format "~ay " (if (> vb 127) (- vb 256) vb)))
      (wb! w vb)))

(define (out-int16! w v)
  (define vw (bitwise-and v #xFFFF))
  (if (wstate-text? w)
      (text-emit! w (format "~aw " (if (> vw 32767) (- vw 65536) vw)))
      (wbs! w (integer->integer-bytes vw 2 #f #f))))

(define (out-int32! w v)
  (define vl (bitwise-and v #xFFFFFFFF))
  (if (wstate-text? w)
      (text-emit! w (format "~al " (if (> vl 2147483647) (- vl 4294967296) vl)))
      (wbs! w (integer->integer-bytes vl 4 #f #f))))

;; Internal align — pads with zero bytes; used between chunks.
(define (align-internal! w n)
  (when (and (not (wstate-text? w)) (> n 1))
    (define rem (modulo (wstate-pos w) n))
    (unless (= rem 0)
      (define pad (- n rem))
      (define pos (wstate-pos w))
      (grow! w (+ pos pad))
      (for ([i (in-range pad)])
        (bytes-set! (wstate-buf w) (+ pos i) 0))
      (set-wstate-pos! w (+ pos pad)))))

;; User-visible .align — pads with fill-char.
(define (align-fn! w n)
  (when (and (not (wstate-text? w)) (> n 1))
    (define rem (modulo (wstate-pos w) n))
    (unless (= rem 0)
      (define pad (- n rem))
      (define pos (wstate-pos w))
      (define fc  (wstate-fill-char w))
      (grow! w (+ pos pad))
      (for ([i (in-range pad)])
        (bytes-set! (wstate-buf w) (+ pos i) fc))
      (set-wstate-pos! w (+ pos pad)))))

(define (set-fill-char! w b) (set-wstate-fill-char! w (bitwise-and b #xFF)))

;; outID: align(4) then write 4-byte FOURCC big-endian.
(define (out-id! w id)
  (if (wstate-text? w)
      (text-emit! w (format "'~a' " (id-name id)))
      (begin
        (align-internal! w 4)
        (wbs! w (integer->integer-bytes id 4 #f #t)))))

;; ---------------------------------------------------------------------------
;; Chunks

(define (enter-chunk! w id)
  (define depth (length (wstate-stack w)))
  (set-wstate-path-ids! w (append (wstate-path-ids w) (list id)))
  (define path-key (build-path-key (wstate-path-ids w)))
  (if (wstate-text? w)
      (begin
        (text-emit! w "\n")
        (for ([_ (in-range depth)]) (text-emit! w "\t"))
        (text-emit! w (format "{ '~a' " (id-name id)))
        (set-wstate-stack! w (cons (chunk-frame id 0 0 path-key) (wstate-stack w))))
      (let ()
        (align-internal! w 4)
        (define start-pos      (wstate-pos w))
        (wbs! w (integer->integer-bytes id 4 #f #t))  ; ID big-endian
        (define size-field-pos (wstate-pos w))
        (out-int32! w #xFFFFFFFF)                     ; placeholder
        (set-wstate-stack! w
          (cons (chunk-frame id start-pos size-field-pos path-key)
                (wstate-stack w))))))

(define (exit-chunk! w)
  (define top (car (wstate-stack w)))
  (set-wstate-stack!    w (cdr (wstate-stack w)))
  (set-wstate-path-ids! w (drop-right (wstate-path-ids w) 1))
  (define depth (length (wstate-stack w)))
  (if (wstate-text? w)
      (begin
        (text-emit! w "\n")
        (for ([_ (in-range depth)]) (text-emit! w "\t"))
        (text-emit! w "}"))
      (let ()
        (define payload-start (+ (chunk-frame-size-field-pos top) 4))
        (define size          (- (wstate-pos w) payload-start))
        (patch-bytes! w (integer->integer-bytes size 4 #f #f)
                      (chunk-frame-size-field-pos top))
        (hash-set! (wstate-symbols w) (chunk-frame-path-key top)
                   (cons payload-start size))
        (align-internal! w 4))))

;; ---------------------------------------------------------------------------
;; sizeof / offsetof

(define (emit-sizeof! w path)
  (define sym (hash-ref (wstate-symbols w) path #f))
  (cond
    [sym                  (out-int32! w (cdr sym))]
    [(wstate-text? w)     (out-int32! w 0)]
    [else
     (set-wstate-pending! w
       (cons (backpatch-rec 'sizeof path 0 (wstate-pos w)) (wstate-pending w)))
     (out-int32! w 0)]))

(define (emit-offsetof! w path addend)
  (define sym (hash-ref (wstate-symbols w) path #f))
  (cond
    [sym (out-int32! w (+ (- (car sym) 4) addend))]  ; immediate: payload-pos - 4
    [(wstate-text? w) (out-int32! w 0)]
    [else
     (set-wstate-pending! w
       (cons (backpatch-rec 'offsetof path addend (wstate-pos w)) (wstate-pending w)))
     (out-int32! w #xFFFFFFFF)]))

(define (resolve-backpatches! w)
  (unless (wstate-text? w)
    (define missing '())
    (for ([bp (in-list (reverse (wstate-pending w)))])
      (define sym (hash-ref (wstate-symbols w) (backpatch-rec-path bp) #f))
      (if (not sym)
          (set! missing (cons (backpatch-rec-path bp) missing))
          (let* ([raw (match (backpatch-rec-kind bp)
                        ['sizeof   (cdr sym)]
                        ['offsetof (+ (- (car sym) 8)    ; deferred: payload-pos - 8
                                      (backpatch-rec-addend bp))])]
                 [val (bitwise-and raw #xFFFFFFFF)])
            (patch-bytes! w (integer->integer-bytes val 4 #f #f)
                          (backpatch-rec-write-pos bp)))))
    (unless (null? missing)
      (error 'iffcomp "unresolved chunk references: ~a"
             (string-join missing ", ")))))

;; ---------------------------------------------------------------------------
;; Strings

(define (translate-escapes s)
  ;; Mirrors translateEscapeCodes in writer.go.
  (define out (open-output-bytes))
  (define len (string-length s))
  (let loop ([i 0])
    (when (< i len)
      (define c (string-ref s i))
      (if (and (char=? c #\\) (< (+ i 1) len))
          (let ([c2 (string-ref s (+ i 1))])
            (cond
              [(char=? c2 #\t)  (write-byte 9  out) (loop (+ i 2))]
              [(char=? c2 #\n)  (write-byte 10 out) (loop (+ i 2))]
              [(char=? c2 #\\)  (write-byte 92 out) (loop (+ i 2))]
              [(char=? c2 #\")  (write-byte 34 out) (loop (+ i 2))]
              [(char<=? #\0 c2 #\9)
               (define j
                 (let find ([j (+ i 1)])
                   (if (and (< j len) (char<=? #\0 (string-ref s j) #\9))
                       (find (+ j 1)) j)))
               (write-byte (modulo (string->number (substring s (+ i 1) j)) 256) out)
               (loop j)]
              [else (write-byte 92 out) (write-char c2 out) (loop (+ i 2))]))
          (begin (write-char c out) (loop (+ i 1))))))
  (get-output-bytes out))

(define (out-string! w s)
  (if (wstate-text? w)
      (text-emit-string! w s)
      (begin (wbs! w (translate-escapes s)) (wb! w 0))))

(define (out-string-continue! w s)
  (if (wstate-text? w)
      (out-string! w s)
      (begin
        (when (> (wstate-pos w) 0)
          (set-wstate-pos! w (- (wstate-pos w) 1)))
        (out-string! w s))))

(define (out-string-pad! w s total-bytes)
  (if (wstate-text? w)
      (out-string! w s)
      (let ()
        (out-string! w s)
        (define required (+ (bytes-length (translate-escapes s)) 1))
        (when (< required total-bytes)
          (define pad (- total-bytes required))
          (define pos (wstate-pos w))
          (grow! w (+ pos pad))
          (for ([i (in-range pad)])
            (bytes-set! (wstate-buf w) (+ pos i) 0))
          (set-wstate-pos! w (+ pos pad))))))

;; ---------------------------------------------------------------------------
;; Timestamp / file

(define (out-timestamp! w unix-seconds)
  (out-int32! w unix-seconds))

(define (out-file! w file-path start length)
  (define data
    (with-handlers ([exn:fail:filesystem?
                     (λ (e) (error 'iffcomp "read ~a: ~a" file-path (exn-message e)))])
      (file->bytes file-path)))
  (when (>= start (bytes-length data))
    (error 'iffcomp "~a: start ~a past EOF (~a bytes)" file-path start (bytes-length data)))
  (define end (if (= length +inf.0)
                  (bytes-length data)
                  (min (bytes-length data) (+ start length))))
  (define slice (subbytes data start end))
  (if (wstate-text? w)
      (begin
        (for ([b (in-bytes slice)])
          (text-emit! w (format "~ay " (if (> b 127) (- b 256) b)))
          (set-wstate-text-on-line! w (+ (wstate-text-on-line w) 1))
          (when (= (wstate-text-on-line w) 100)
            (text-emit-comment! w)
            (set-wstate-text-on-line! w 0)))
        (text-emit-comment! w)
        (set-wstate-text-on-line! w 0))
      (wbs! w slice)))

;; ---------------------------------------------------------------------------
;; Fixed-point

(define (out-fixed! w val prec)
  (if (wstate-text? w)
      (text-emit! w (format "~a(~a.~a.~a) "
                            (format-g-alt val 16)
                            (sizespec-sign prec)
                            (sizespec-whole prec)
                            (sizespec-frac prec)))
      (let ()
        (define scale  (arithmetic-shift 1 (sizespec-frac prec)))
        (define scaled (inexact->exact (truncate (* val scale))))
        (define bits   (+ (sizespec-sign prec) (sizespec-whole prec) (sizespec-frac prec)))
        (cond
          [(> bits 16) (out-int32! w (bitwise-and scaled #xFFFFFFFF))]
          [(> bits  8) (out-int16! w (bitwise-and scaled #xFFFF))]
          [else        (out-int8!  w (bitwise-and scaled #xFF))]))))

;; ---------------------------------------------------------------------------
;; Text helpers

(define (text-emit! w s)
  (display s (wstate-text-port w)))

(define (text-emit-string! w s)
  (display "\"" (wstate-text-port w))
  (for ([c (in-string s)])
    (cond
      [(char=? c #\newline) (display "\\n\"\n\"" (wstate-text-port w))]
      [(char=? c #\\)       (display "\\\\" (wstate-text-port w))]
      [(char=? c #\")       (display "\\\"" (wstate-text-port w))]
      [else                  (display c (wstate-text-port w))]))
  (display "\" " (wstate-text-port w)))

(define (text-emit-comment! w)
  (display " //\n" (wstate-text-port w))
  (define tabs (+ (length (wstate-stack w)) 1))
  (for ([_ (in-range tabs)]) (display "\t" (wstate-text-port w))))

;; ---------------------------------------------------------------------------
;; %#.16g equivalent

(define (format-g-alt val prec)
  (cond
    [(not (real? val)) (~a val)]
    [(= val 0.0)
     (string-append "0." (make-string (- prec 1) #\0))]
    [else
     (define sign (if (< val 0) "-" ""))
     (define abs  (abs val))
     (define exp  (inexact->exact (floor (/ (log abs) (log 10)))))
     (if (or (>= exp prec) (< exp -4))
         ;; Scientific notation
         (let ([s (~r abs #:precision `(= ,(- prec 1)) #:notation 'exponential)])
           ;; Normalize e+5 → e+05 (C printf style)
           (string-append sign (regexp-replace #rx"e([+-])([0-9])$" s "e\\10\\2")))
         ;; Fixed notation
         (let* ([dp (max 0 (- prec 1 exp))]
                [s  (~r abs #:precision `(= ,dp))]
                [s  (if (regexp-match? #rx"\\." s) s (string-append s "."))])
           (string-append sign s)))]))

;; ---------------------------------------------------------------------------
;; Path key helpers

(define (id-name id)
  (define raw (list (bitwise-and (arithmetic-shift id -24) #xFF)
                    (bitwise-and (arithmetic-shift id -16) #xFF)
                    (bitwise-and (arithmetic-shift id  -8) #xFF)
                    (bitwise-and id #xFF)))
  (define n (let loop ([n 4])
               (if (and (> n 0) (= (list-ref raw (- n 1)) 0))
                   (loop (- n 1)) n)))
  (list->string (map integer->char (take raw n))))

(define (build-path-key ids)
  (apply string-append
         (map (λ (id) (string-append "::'" (id-name id) "'")) ids)))
