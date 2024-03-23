
;mode 160an
loop:
    ;
    ;v
    pop er0
    hex 30 d2
    read_key ;18
    ;^
    ;读ki ko 到 d230的位置

    ;
    ;v
    pop er0
    hex 32 d2 
    read_key ;18
    ;^
    ;读ki ko 到d232的位置

    ;
    ;v
    pop er12 ;8
    hex e4 30 ;10
    setlr
    ;^
    ;setlr 设置er14 因为后面有些rop地址是RT返回不是pop pc返回

    ;
    ;v
    pop xr0
    hex 30 30 31 d2
    l er0,[er2]
    ;^
    ; [xx xx] [xx xx]  <- 两次read key 内存排布
    ;  xx [     ] xx   <- 取中间这两个字节到er0中

    sll r0,4
    or r0,r1
    mov r2,r0,pop er0
    hex 30 30 
    ;现在r2的值是我们目标写入的字节
    
    pop er0
;60 above
y:
    ;写入地址 选取e9e0是因为这一块地址在重启后不会清除
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

; 程序主入口:
start:
    pop qr0
    hex 22 d5 80 d1 30 30 30 30 
    smart_strcpy
    pop ER14
    adr loop -8
    jpop qr8   ;jump and pop qr8,pop qr0
