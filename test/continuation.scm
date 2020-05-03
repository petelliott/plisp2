(println (call/cc (lambda (cont)
                    5
                    7)))

(println (call/cc (lambda (cont)
                    5
                    (cont 6)
                    7)))

;(define c #f)
;
;(println (call/cc (lambda (cont) (set! c cont))))
;
;(when c
;  (c 66)
;  (set! c #f))
