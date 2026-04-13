#lang racket
;; test/byte-exact.rkt — byte-exact regression tests for iffcomp-scm.
;;
;; Compiles each .iff.txt fixture and compares the output byte-for-byte
;; against the oracle .iff binary produced by the original C++ iffcomp.
;; The TIME chunk payload is masked before comparison because it is
;; time-varying.

(require rackunit
         rackunit/text-ui
         racket/runtime-path
         (only-in racket/file file->bytes)
         "../iffcomp.rkt")

(define-runtime-path TESTDATA "../testdata")

;; Mask the 4-byte timestamp payload after the TIME FOURCC marker.
;; Pattern: 'T' 'I' 'M' 'E' #x04 #x00 #x00 #x00  followed by 4 time bytes.
(define (mask-timestamp bs)
  (define marker #"TIME\x04\x00\x00\x00")
  (define ml (bytes-length marker))
  (define out (bytes-copy bs))
  (let loop ([i 0])
    (when (< i (- (bytes-length out) (+ ml 4)))
      (if (equal? (subbytes out i (+ i ml)) marker)
          (begin
            (bytes-fill! (subbytes out (+ i ml) (+ i ml 4)) 0)
            ;; zero out the 4 timestamp bytes in-place
            (bytes-set! out (+ i ml 0) 0)
            (bytes-set! out (+ i ml 1) 0)
            (bytes-set! out (+ i ml 2) 0)
            (bytes-set! out (+ i ml 3) 0))
          (loop (+ i 1)))))
  out)

(define (compile-and-compare name input-name expected-name)
  (test-case name
    (define input    (path->string (build-path TESTDATA input-name)))
    (define expected (build-path TESTDATA expected-name))
    (define tmp      (make-temporary-file "iffcomp-scm~a.iff"))
    (dynamic-wind
      void
      (λ ()
        (compile input (path->string tmp) 'binary #f)
        (check-equal? (mask-timestamp (file->bytes tmp))
                      (mask-timestamp (file->bytes expected))
                      (format "~a: byte-exact match" name)))
      (λ () (when (file-exists? tmp) (delete-file tmp))))))

(define all-tests
  (test-suite "iffcomp-scm byte-exact"
    (compile-and-compare "test.iff.txt"        "test.iff.txt"        "expected.iff")
    (compile-and-compare "all_features.iff.txt" "all_features.iff.txt" "all_features.iff")))

(run-tests all-tests)
