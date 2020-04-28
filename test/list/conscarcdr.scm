(println (car '(1 2)) (car '(1 . 2)))
(println (cdr '(1 2)) (cdr '(1 2 3)) (cdr '(1 . 2)))

(println (cons 1 2) (cons 1 '()))
(println (cons 1 (cons 2 (cons 3 '()))))
