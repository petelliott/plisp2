(require "alist.scm")
(require "listtools.scm")

;;; begin

;; the following functions and values will be replaced later in the
;; bootstrapping process. probably with a hashtable or something

(define macros '())

(define (macro-find sym)
  (assq-ref macros sym))

(define (macro-set! sym fn)
  (set! macros (acons sym fn macros)))
;;; end

(define (macroexpand expr)
  ;; no cond yet :-(
  (if (pair? expr)
      (if (macro-find (car expr))
          (macroexpand (apply (macro-find (car expr))
                               (cdr expr)))
          (map macroexpand expr))
      expr))
