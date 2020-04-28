(require "macro.scm")


(define-macro (let args . body)
  `((lambda ,(map car args)
      ,@body) ,@(map cadr args)))

(define-macro (begin . body)
  (if (null? body)
      (unspecified)
      (if (null? (cdr body))
          (car body)
          `((lambda () ,@body)))))

(define-macro (cond . clauses)
  (if (null? clauses)
      (unspecified)
      (if (eq? (caar clauses) 'else)
          `(begin
             ,@(cdar clauses))
          `(if ,(caar clauses)
               (begin
                 ,@(cdar clauses))
               (cond ,@(cdr clauses))))))


(define-macro (and . terms)
  (cond
   ((null? terms) #t)
   ((null? (cdr terms)) (car terms))
   (else `(if ,(car terms)
              (and ,@(cdr terms))
              #f))))

(define-macro (or . terms)
  (cond
   ((null? terms) #f)
   ((null? (cdr terms)) (car terms))
   (else `(let ((res ,(car terms)))
             (if res
                 res
                 (or ,@(cdr terms)))))))
