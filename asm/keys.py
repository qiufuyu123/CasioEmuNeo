key_codes=[
    #
    ['[nul]','[space]',*['@']*14],
    [*['@']*9,'[]',*['@']*6],
    ['i','e','兀',':','$','?',*['@']*6,',','x10','.','@'],
    ['0','1','2','3','4','5','6','7','8','9','[A]','[B]','[C]','[D]','[E]','[F]'],
    ['M','ANS','A','B','C','D','E','F','x','y','PreAns',*['@']*5],
    [*['@']*16],
    ['(',*['@']*5,'Conjg','Arg','Abs','Rnd(',*['@']*2,'sinh','cosh','tanh','sinh-1'],
    ['cosh-1','tanh-1','e^','10^','√','ln','∛','sin','cos','tan','sin-1','cos-1','tan-1','log(',*['@']*2],
    [*['@']*15,'Rep'],
    [*['@']*16],
    [*['@']*5,'=','+','-','*','∻','∻R','·','∠','|P','|C','@'],
    [*['@']*16],
    [*['@']*8,'∟','^(','x√x',*['@']*5],
    [')','@','>a+bi','>r∠0','-1','square','cube','%','!',*['@']*3,'∘','E','P','T'],
    ['G','M','k','m','u','n','p','f',*['@']*8],
    [*['@']*16]
]

tmp_uglyc=[]
def byte2keys(b):
    if len(b) != 2:
        return '',False
    y,x = 0,0
    if b[0] >= 'a':
        y = ord(b[0]) -ord('a')+10
    elif '0' <= b[0] <= '9':
        y = ord(b[0]) -ord('0')
    else:
        return '',False
    if b[1] >= 'a':
        x = ord(b[1]) -ord('a')+10
    elif '0' <= b[1] <= '9':
        x = ord(b[1]) -ord('0')
    else:
        return '',False
    try:
        s = key_codes[y][x]
    except:
        tmp_uglyc.append(str(b))
        return '<'+b+'>',True
    if s == '@':
        tmp_uglyc.append(str(b))
        return '<'+b+'>',True
    return key_codes[y][x],True