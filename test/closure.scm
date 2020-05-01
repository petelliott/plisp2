(define (clos x)
  (lambda () x))

(define a (clos 5))
(define b (clos 6))

(println (a) (b) (a) (b))

(define (clos x)
  (lambda ()
    ((lambda ()
       (lambda () x)))))

(define a ((clos 5)))
(define b ((clos 6)))

(println (a) (b) (a) (b))

(define (clos x)
  (define z x)
  (lambda () z))

(define a (clos 5))
(define b (clos 6))

(println (a) (b) (a) (b))

;; closure mutability

(define (inc)
  (define x 0)
  (lambda ()
    (set! x (+ x 1))
    x))

(define f (inc))

(println (f))
(println (f))
(println (f))
(println (f))

(define (inc n)
  (lambda ()
    (set! n (+ n 1))
    n))

(define f (inc 3))

(println (f))
(println (f))
(println (f))
(println (f))
