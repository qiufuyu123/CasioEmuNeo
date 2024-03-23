; loop test
;

; Effect:
; 
; 1. print `love you`
; 2. wait for key press
; 3. print `forever`
; 4. wait for key press
; 5. goto 1. and repeat forever

;mode 100+9+13an

#org e740
hex 6c 6f 76 65 20 79 6f 75 ; `love you`
space 8
;space 16
home:
    pop qr0
    hex 20 20 e0 e9 30 30 30 30 
    ;remember, we do not use label here to represent `love you`
    ;since, stacks will be filled and broke by other functions
    ;we use 0xd180 (input area rather than stack area)
    printline
    waitkey
    pop qr0
    ; 74 d5
    hex 20 20 
    adr label2
    hex 30 30 30 30
    printline
    waitkey
    pop qr0
    hex 40 e7 e9 e0 30 30 30 30 
    smart_strcpy
    pop ER14
    adr home -16
    jpop qr80   ;jump and pop qr8,pop qr0
label2:
    hex 46 6f 72 20 65 76 65 72  ; `Forever`
    ; here, since it locates at the end of stack
    ; we do not care 

copy:
    pop qr0 ;;EA34
    hex  e0 e9 80 d1 30 30 30 30
    smart_strcpy