import struct,sys
T="/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode"
def dec(data):
    key_r=0x0816; c1=0x6081; c2=0x1608
    out=bytearray(len(data))
    for i,c in enumerate(data):
        out[i]=c ^ ((key_r>>8)&0xff)
        key_r=((c+key_r)*c1+c2)&0xffff
    return bytes(out)
SIZE={1:1,2:1,3:2,4:2,5:4,6:4,8:4,9:8}
def parse(path,encrypted=True):
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
            else:
                sz=SIZE.get(t,1); cols.append(d[off:off+sz]); off+=sz
        rows.append(cols)
    return ncol,types,rows
def parse_csv(path):
    import csv
    rows=[]
    with open(path, newline='', encoding='latin1') as f:
        rd=csv.reader(f); next(rd)
        for r in rd:
            if len(r)>=4: rows.append(r)
    return rows
# V1279
nc,ty,rows=parse(T+"/v1279b/KnightOnline/Data/item_org_us.tbl")
print("V1279 ncol=%d nrow=%d" % (nc,len(rows)))
v1279={struct.unpack("<I",r[0][:4])[0]: r[2] for r in rows}
# current client
nc2,ty2,rows2=parse("/Users/berkgulher/OpenKO-Client/assets/Client2/Data/Item_Org_us.tbl")
print("CURRENT ncol=%d nrow=%d" % (nc2,len(rows2)))
cur={struct.unpack("<I",r[0][:4])[0]: r[2] for r in rows2}
added=sorted((k,cur[k]) for k in cur if k not in v1279)
removed=sorted((k,v1279[k]) for k in v1279 if k not in cur)
print("added=%d removed=%d common=%d" % (len(added),len(removed),len(set(cur)&set(v1279))))
open(T+"/added_items.txt","w").write("\n".join("%d\t%s"%x for x in added))
open(T+"/removed_items.txt","w").write("\n".join("%d\t%s"%x for x in removed))
print("--- ADDED containing Class/Grade ---")
for k,n in added:
    if "Class" in n or "Grade" in n:         print("  %d\t%s"%(k,n))
