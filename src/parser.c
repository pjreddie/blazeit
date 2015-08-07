/*

lx.x
lx . ly. x y
(x y -> xy)

(x -> (y -> y))

ind nat : Type0 = 
| 0 : nat
| S : nat -> nat
.

def plus : (_:nat) -> (_:nat) -> nat =
    fun x => fun y =>
        nat.ind (fun _:nat => nat) y (fun _:nat => fun sum:nat => S sum) x.
            

*/


term *parse_term(char *s)
{
    
}


