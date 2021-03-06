; EXPECT: sat
; EXPECT: ((foo true) (bar false) (baz true))
; EXIT: 10
(set-logic QF_LIA)
(set-option :produce-assignments true)
(declare-fun x () Int)
(declare-fun y () Int)
(assert (! (or (! (= x (+ y 5)) :named foo) (! (= x (- y 5)) :named bar)) :named baz))
(assert (and (> x 0) (<= y 5)))
(check-sat)
(get-assignment)
