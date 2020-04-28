;; defines a mininal require, and requires the standard library

(define required '())

(define (require-cannonicalize path)
  ; we don't have cond or or yet
  (realpath (if (eq? (vector-ref path 0) #\/)
                path
                (if %file
                    (string-append (dirname %file) "/" path)
                    path))))

(define (require-find lst obj)
  (if (null? lst)
      #f
      (if (equal? (car lst) obj)
          lst
          (require-find (cdr lst) obj))))

(define (require-load file)
  (set! required (cons file required))
  (load file))

(define (require path)
  (define rpath (require-cannonicalize path))
  (if (not (require-find required rpath))
      (require-load rpath)
      #f))

(require "stdlib.scm")
