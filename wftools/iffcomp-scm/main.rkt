#!/usr/bin/env racket
#lang racket/base
;; main.rkt — CLI entry point for iffcomp-scm.
;;
;; Usage:
;;   iffcomp-scm [-o <file>] [-binary] [-ascii] [-v] [-q] <input.iff.txt>

(require racket/cmdline
         "iffcomp.rkt")

(define out-file (make-parameter "test.wf"))
(define mode     (make-parameter 'binary))
(define verbose? (make-parameter #f))
(define quiet?   (make-parameter #f))

(define input-file
  (command-line
   #:program "iffcomp-scm"
   #:once-each
   [("-o") f
    "Output file path (default: test.wf)"
    (out-file f)]
   [("-binary")
    "Write binary IFF output (default)"
    (mode 'binary)]
   [("-ascii")
    "Write human-readable text IFF output"
    (mode 'text)]
   [("-v")
    "Verbose: trace parse productions to stderr"
    (verbose? #t)]
   [("-q")
    "Quiet: suppress informational output"
    (quiet? #t)]
   #:args (input)
   input))

(define (main)
  (with-handlers
    ([exn:fail?
      (λ (e)
        (eprintf "iffcomp: ~a\n" (exn-message e))
        (exit 10))])
    (define nbytes (compile input-file (out-file) (mode) (verbose?)))
    (unless (quiet?)
      (eprintf "wrote ~a bytes to ~a\n" nbytes (out-file)))))

(main)
