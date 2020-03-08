#lang br/quicklang

; progress: Program­ming our expander: output
; https://beautifulracket.com/stacker/the-expander.html

; Reader, takes every line and converts it to (handle ${EXPRESSION})
(define (read-syntax path port)
  (define src-lines (port->lines port))
  (define src-datums (format-datums '(handle ~a) src-lines))
  (define module-datum `(module stacker-mod "stacker.rkt"
                          ,@src-datums))
  (datum->syntax #f module-datum))

; export reader
(provide read-syntax)

; Expander
(define-macro (stacker-module-begin HANDLE-EXPR ...)
  ; #' is equivalent to datum->syntax
  ; (and lexical context, giving it access to all variables)
  #'(#%module-begin 
     HANDLE-EXPR ...
     (display (first stack))))

; export expander
(provide (rename-out [stacker-module-begin #%module-begin]))

; Implement basic stack with pop & push
(define stack empty)
(define (pop-stack!)
  (define arg (first stack))
  (set! stack (rest stack))
  arg)
(define (push-stack! arg)
  (set! stack (cons arg stack)))

; Implement handle function
(define (handle [arg #f]) ; [arg #f] makes the arg parameter optional
  (cond
    ; Adds numbers to the stack
    [(number? arg) (push-stack! arg)]
    ; Take last two numbers and execute operation on them,
    ; adding the result to the stack
    [(or (equal? + arg) (equal? * arg))
         (define op-result (arg (pop-stack!) (pop-stack!)))
         (push-stack! op-result)]))
(provide handle)

; export operation functions so they can be used after (handle) call
(provide + *)
  
