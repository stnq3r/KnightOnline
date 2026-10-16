import struct, sys, shutil

def dec(data):
    key_r=0x0816; c1=0x6081; c2=0x1608
    out=bytearray(len(data))
    for i,c in enumerate(data):
        out[i]=c ^ ((key_r>>8)&0xff)
        key_r=((c+key_r)*c1+c2)&0xffff
    return bytes(out)

def enc(plain):
    key_r=0x0816; c1=0x6081; c2=0x1608
    out=bytearray(len(plain))
    for i,pl in enumerate(plain):
        c=pl ^ ((key_r>>8)&0xff)
        out[i]=c
        key_r=((c+key_r)*c1+c2)&0xffff
    return bytes(out)

def col_offsets(plain, target_col):
    off=0
    ncol,=struct.unpack_from("<i",plain,off); off+=4
    types=list(struct.unpack_from("<%dI"%ncol,plain,off)); off+=4*ncol
    nrow,=struct.unpack_from("<i",plain,off); off+=4
    offs=[]
    for r in range(nrow):
        coloff=[]
        for t in types:
            coloff.append(off)
            if t==7:
                ln,=struct.unpack_from("<i",plain,off); off+=4+ln
            elif t==9: off+=8
            elif t in (5,6,8): off+=4
            elif t in (3,4): off+=2
            else: off+=1
        offs.append(coloff[target_col])
    return ncol,types,nrow,offs

def merge(path, frm, to, target_col=35):
    d=open(path,'rb').read()
    plain=bytearray(dec(d))
    ncol,types,nrow,offs=col_offsets(plain,target_col)
    assert types[target_col] in (1,2), ("coltype",types[target_col])
    ch=0
    for o in offs:
        if plain[o]==frm:
            plain[o]=to; ch+=1
    assert len(plain)==len(d)
    out=enc(bytes(plain))
    open(path,'wb').write(out)
    return ncol,nrow,ch

if __name__=="__main__":
    for p in sys.argv[1:]:
        print(p, merge(p,250,254))
