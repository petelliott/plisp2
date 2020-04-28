(define a (+ 1 3))
(println a)

; this comment should break parsing ( (((((( dfsdfdsf

(println (lambda () 4))
(println ((lambda (a b) (+ a b)) 5 6))

(println ((((lambda (a) (lambda () (lambda () (+ a 1)))) 4))))
(println ((lambda (a b . rest) (list a b rest)) 1 2 3 4 5)
         ((lambda (a b . rest) (list a b rest)) 1 2))

(define (f x y)
  (write 'a)
  (+ x y))

(println (f 5 6))

(define (f2 x y)
  (define z (+ x y))
  (define q (+ z 1))
  (+ q 1))

(println (f2 5 6))

(define (c+ x)
  (define (c++ y)
    (+ x y))
  c++)

(define curry (c+ 8))

(println (curry 1) (curry 2) (curry 3))

(println (if #f 1 2) (if #t 1 2) (if #f (newline) 3)
         (if 7 1 2) (if '() 1 2))

(define x 5)
(set! x 10)
(println x)
(println ((lambda () x)) ((lambda (x) x) 4))
((lambda () ((lambda () (set! x 6)))))
(println x)

(define (f3 y)
  (define z 4)
  (set! z 5)
  (set! y 5)
  (+ z y))

(println (f3 1) (f3 2))

(println `(1 2 3)
         `(1 ,(+ 1 2) 3)
         `(1 ((,2)) 3)
         `,(+ 1 2)
         `(1 2 ,@'(3 4) 5 6)
         `(1 `(2 ,3) 4))
