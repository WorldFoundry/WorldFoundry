#lang racket/base
;; iffcomp.rkt — library entry point.
;;
;; (compile in-file out-file mode verbose?) → exact-integer (bytes written)
;; mode: 'binary or 'text

(require racket/path
         "lexer.rkt"
         "parser.rkt"
         "writer.rkt")

(provide compile)

(define (compile in-file out-file mode verbose?)
  ;; File includes in the DSL ("[ file ]") resolve relative to the input file's
  ;; directory, matching the behaviour of the C++ and Go ports.
  (define base-dir (path-only (path->complete-path in-file)))
  (parameterize ([current-directory base-dir])
    (define lx (make-lexer in-file))
    (define w  (if (eq? mode 'text)
                   (make-text-writer)
                   (make-binary-writer)))
    (parse! lx w verbose?)
    (resolve-backpatches! w)
    (define data (writer-bytes w))
    (with-output-to-file out-file
      #:exists 'replace
      (λ () (write-bytes data)))
    (bytes-length data)))
