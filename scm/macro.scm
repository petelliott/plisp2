(require "alist.scm")
(require "listtools.scm")

;;; begin

;; the following functions and values will be replaced later in the
;; bootstrapping process. probably with a hashtable or something

(define macros '())

(define (macro-find sym)
  (assq-ref macros sym))

(define (macro-set! sym fn)
  (set! macros (acons sym fn macros))
  (unspecified))
;;; end

;; expand the unquoted part of quasiquotes
(define (macroexpand-qq expr)
  (if (not (list? expr))
      expr
      (if (eq? (car expr) 'quasiquote)
          expr
          (if (eq? (car expr) 'unquote)
              (list 'unquote (macroexpand (cadr expr)))
              (map macroexpand-qq expr)))))

(define (macroexpand expr)
  ;; no cond yet :-(
  (if (not (list? expr))
      expr
      (if (eq? (car expr) 'quote) ; TODO quasiquote
          expr
          (if (eq? (car expr) 'quasiquote)
              (list 'quasiquote (macroexpand-qq (cadr expr)))
              (if (macro-find (car expr))
                  (macroexpand (apply (macro-find (car expr))
                                      (cdr expr)))
                  (map macroexpand expr))))))

(macro-set! 'define-macro
            (lambda (args . body)
              `(macro-set! ',(car args)
                           (lambda ,(cdr args)
                             ,@body))))
