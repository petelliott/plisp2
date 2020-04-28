(println (let ((a (+ 1 2))
               (b 5))
           (newline)
           (+ a b)))

(println (let* ((a 5)
                (b (+ 5 4)))
           (newline)
           (+ b 1)))

(println (begin) (begin 5) (begin (write 'a) 6))

(println (cond)
         (cond
          (#f 1)
          (#f 2))
         (cond
          (#f (newline))
          (#t 4)
          (#f (newline)))
         (cond
          (#f 5)
          (else 6)))

(println (and)
         (and 1 2)
         (and 1 #f)
         (and 1 #f (newline) (newline)))

(println (or)
         (or 1 2)
         (or 1 (newline) (newline))
         (or #f #f 2))
