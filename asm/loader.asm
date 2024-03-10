
;mode 160an

;we set lr here firstly for each loop
; pop er12 ;8
; hex e4 30 ;10
; setlr   ;
loop:
    ; er12 = 0x30 e4
    pop er0
    hex 30 d2
    read_key ;18
    ; pop xr0
    ; hex 30 30 30 d2
    ; l er0,[er2]
    ;v
    ; ^ 
    ;这整一段(包括上面两行)都是用于设置lr 到 130e4h (pop pc指令处)
    pop er0
    hex 32 d2 
    ;34 bytes above

    ; r0 &= 0xf
    ; r2 = r0
    read_key ;18
    pop er12 ;8
    hex e4 30 ;10
    setlr
    pop xr0
    hex 30 30 31 d2
    l er0,[er2]
    ; v
    
    ; ^ 
    ;这整一段(包括上面两行)都是用于设置lr 到 130e4h (pop pc指令处)
    sll r0,4
    or r0,r1
    mov r2,r0,pop er0
    hex 30 30 
    ;现在r2的值是我们目标写入的字节
    ; r0 &= 0xf
    ; r2 = r0
    pop er0
;60 above
y:
    ;写入地址
    hex e0 e9
    ;计算一下偏移
    ; hex e0 e9 offset = 60 bytes
    ; 0xd180 + 60 = d1bc
    st r2,[er0]
    ;自增
    mov er2,1
    pop er8
    hex bc d1
    r0=0,[er8]+=er2,pop xr8
    hex 30 30 30 30 ;for xr8

    pop qr0
    hex 22 d5 80 d1 30 30 30 30 
    smart_strcpy
    pop ER14
    adr loop -8
    jpop qr8   ;jump and pop qr8,pop qr0
