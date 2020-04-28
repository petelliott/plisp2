;; caar cadr etcetera

(define (caar obj)
  (car (car obj)))

(define (cadr obj)
  (car (cdr obj)))

(define (cdar obj)
  (cdr (car obj)))

(define (cddr obj)
  (cdr (cdr obj)))
