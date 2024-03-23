hex 31 32 33 34 35 36 37 38
space 26
    pop qr0 ;;EA34
    hex  e0 ea 80 d1 30 30 30 30
    smart_strcpy
    pop ER14
    hex 2c ea
    jpop qr8   ;jump and pop qr8,pop qr0