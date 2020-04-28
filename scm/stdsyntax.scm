(require "macro.scm")


(define-macro (let args . body)
  `((lambda ,(map car args)
      ,@body) ,@(map cadr args)))

(define-macro (cond . clauses)
  (if (null? clauses)
      (unspecified)
      (if (eq? (caar clauses) 'else)
          (cadar clauses)
          `(if ,(caar clauses)
               ,(cadar clauses)
               (cond ,@(cdr clauses))))))
