(println (reverse '())
         (reverse '(1))
         (reverse '(1 2))
         (reverse '(1 2 3))
         (reverse '(1 2 3 4)))

(println (list (+ 1 2) 3 8)
         (list))

(println (length '())
         (length '(1))
         (length '(1 2 3 4 5 6)))

(println (append '() '())
         (append '(1) '())
         (append '() '(1))
         (append '(1 2) '(3 4)))
