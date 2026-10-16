import struct,glob,os
T="/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode"
def dec(d):
    kr,c1,c2=0x0816,0x6081,0x1608;o=bytearray(len(d))
    for i,c in enumerate(d):
        o[i]=c^((kr>>8)&0xff);kr=((c+kr)*c1+c2)&0xffff
    return bytes(o)
SIZE={1:1,2:1,3:2,4:2,5:4,6:4,8:4,9:8}
def parse(p):
    b=dec(open(p,'rb').read());off=0
    (n,)=struct.unpack_from("<i",b,off);off+=4
    t=list(struct.unpack_from("<%di"%n,b,off));off+=4*n
    (rc,)=struct.unpack_from("<i",b,off);off+=4
    rows=[]
    for _ in range(rc):
        cols=[]
        for ty in t:
            if ty==7:
                (ln,)=struct.unpack_from("<i",b,off);off+=4;cols.append(b[off:off+ln].decode('cp949','replace'));off+=ln
            else:
                sz=SIZE.get(ty,1);cols.append(int.from_bytes(b[off:off+sz],'little',signed=ty in (1,3,5)));off+=sz
        rows.append(cols)
    return t,rows
V=T+"/v1279b/KnightOnline/Data/"
files=sorted(glob.glob(V+"Item_Ext_*.tbl"))
for f in files:
    t,rows=parse(f)
    # columns of interest: dwID(0), dwBaseID(2)
    hits=[r for r in rows if any(110110000<=x<=110110009 for x in (r[0],r[2]))]
    ids=[r[0] for r in rows]
    print(os.path.basename(f),"rows=",len(rows),"idrange=",min(ids,"0") if False else (min(ids),max(ids)),"daggerhits=",len(hits))
# show first few rows of ext_0 to understand
t,rows=parse(V+"Item_Ext_0_us.tbl")
print("ext0 cols:",t)
for r in rows[:5]:
    print("  id=%d base=%d remark=%r"% (r[0],r[2],r[3]))
