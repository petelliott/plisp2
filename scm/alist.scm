(require "alist.scm")

(define (acons key value alist)
  (cons (cons key value) alist))

(define (assq key alist)
  (if (null? alist)
      #f
      (if (eq? key (caar alist))
          (car alist)
          (assq key (cdr alist)))))

(define (assq-ref alist key)
  (define res (assq key alist))
  (if res
      (cdr res)
      #f))
