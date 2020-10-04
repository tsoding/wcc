(module
 (func $fib (export "fib")
       (param $n i32)
       (result i32)
       (local $a i32)
       (local $b i32)
       (local $c i32)
       (set_local $a (i32.const 0))
       (set_local $b (i32.const 1))
       (block
        $exit
        (loop
         $repeat
         (br_if $exit (i32.eqz (get_local $n)))
         (set_local $c (i32.add (get_local $a) (get_local $b)))
         (set_local $a (get_local $b))
         (set_local $b (get_local $c))
         (set_local $n (i32.sub (get_local $n) (i32.const 1)))
         (br $repeat)))
       (get_local $a)))
