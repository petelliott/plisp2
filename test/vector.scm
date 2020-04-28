(println #(1 2 3 (+ 1 2)))
(println (vector 1 2 3 (+ 1 2)))

(define v #(1 2 3 4 5 6 7 8))

(println (vector-length v))
(println (vector-ref v 0) (vector-ref v 1) (vector-ref v 7))

(vector-set! v 3 99)
(println (vector-ref v 3))

(println (vector-append #() #()) (vector-append #(1 2) #(3 4)))
(println (make-vector 4 7))

(println (list->vector '())
         (list->vector '(1 2 3)))
