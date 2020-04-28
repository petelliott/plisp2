(println (not 1) (not #t) (not #f))

(println (null? '()) (null? '(1 2)) (null? 5))

(println (eq? 1 1) (eq? 1 2)
         (eq? '() '()) (eq? '(1) '(1))
         (eq? 'a 'a) (eq? 'a 'b)
         (eq? "a" "a"))

(println (equal? 'a 'a)
         (equal? '(1 . 2) '(1 . 2))
         (equal? '(1 2 3 (4 5) 6) '(1 2 3 (4 5) 6))
         (equal? #(1 2 3 4) #(1 2 3 4))
         (equal? "abcdef" "abcdef"))

(println (< 1 2) (< 2 1) (< 2 2))

(println (pair? '(1 . 2))
         (pair? (cons 1 2))
         (pair? '(1 2 3 4))
         (pair? 1)
         (pair? '()))

(println (list? '(1 2 3 . 4))
         (list? '())
         (list? '(1 2 3)))
