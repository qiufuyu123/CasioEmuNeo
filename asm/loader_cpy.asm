;132 an

;
; 0xd180 处内存结构
; [xx] [xx] [yy ... yy] [jump t0 e9 e0...]
;  ^^--目标地址
;            ^^--写入数据

; 数据 - 0x6c --> 值
; 所以我们把
; sinh( cosh(  tanh( ... tan-1( 映射到
;  01    02     03   ... 0f


;mode 162

#org e9e0

loop:
    ;
    ;v
    pop er12 ;8
    hex e4 30 ;10
    setlr
    ;^
    ;setlr 设置er14 因为后面有些rop地址是RT返回不是pop pc返回
    pop qr0
    hex 30 30 
    adr y
    l er0,[er2]
    l r0,[er0]   ;rt
    and r0,15
    mov r2,r0,pop er0
y:
    hex ea e0
    st r2,[er0]
    ;自增
    mov er2,1
    pop er8
    adr y
    r0=0,[er8]+=er2,pop xr8
    hex 30 30 30 30 ;for xr8

    pop qr0 ;;EA34
    hex  22 d5 e0 e9 30 30 30 30
    smart_strcpy
    pop ER14
    hex 1a d5
    jpop qr8   ;jump and pop qr8,pop qr0

;86 bytes

    pop qr0
    hex  e0 e9 80 d1 30 30 30 30
    smart_strcpy
    