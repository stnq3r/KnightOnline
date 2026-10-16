import struct
T = "/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode"
SIZE = {0:0,1:1,2:1,3:2,4:2,5:4,6:4,7:0,8:4,9:8}
def dec(data):
    key_r=0x0816; c1=0x6081; c2=0x1608
    out=bytearray(len(data))
    for i,c in enumerate(data):
        out[i]=c ^ ((key_r>>8)&0xff)
        key_r=((c+key_r)*c1+c2)&0xffff
    return bytes(out)
def parse(path, encrypted=True):
    d=open(path,'rb').read()
    if encrypted: d=dec(d)
    off=0
    (ncol,)=struct.unpack_from("<i",d,off); off+=4
    types=list(struct.unpack_from("<%dI"%ncol,d,off)); off+=4*ncol
    (nrow,)=struct.unpack_from("<i",d,off); off+=4
    rows=[]
    for r in range(nrow):
        cols=[]
        for t in types:
            if t==7:
                (ln,)=struct.unpack_from("<i",d,off); off+=4
                s=d[off:off+ln].decode('latin1'); off+=ln
                cols.append(s)
            elif t in (8,):
                v,=struct.unpack_from("<f",d,off); off+=4; cols.append(v)
            elif t in (9,):
                v,=struct.unpack_from("<d",d,off); off+=8; cols.append(v)
            elif t in (5,6):
                v,=struct.unpack_from("<i",d,off); off+=4; cols.append(v)
            elif t in (3,4):
                v,=struct.unpack_from("<h",d,off); off+=2; cols.append(v)
            else:
                sz=SIZE.get(t,1); v=d[off] if sz==1 else d[off:off+sz]; off+=sz; cols.append(v)
        rows.append(cols)
    return ncol,types,rows
def load_int(path,keycol=0):
    nc,ty,rows=parse(path)
    d={}
    for r in rows:
        k=r[keycol]
        if isinstance(k,bytes):
            k=int.from_bytes(k,'little',signed=False) if len(k)>2 else k[0]
        d[k]=r
    return nc,ty,d

if __name__=="__main__":
    import sys
    p=sys.argv[1]
    nc,ty,rows=parse(p)
    print("ncol=%d nrow=%d"%(nc,len(rows)))
    print("types=",ty)
    for r in rows[:3]:
        print([ (v if isinstance(v,str) else (int.from_bytes(v,'little') if isinstance(v,bytes) else v)) for v in r])
