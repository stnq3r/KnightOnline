import sys,re
def decrypt(data):
    key_r=0x0816; key_c1=0x6081; key_c2=0x1608
    out=bytearray(len(data))
    for i,c in enumerate(data):
        out[i]=c ^ ((key_r>>8)&0xff)
        key_r=((c+key_r)*key_c1+key_c2)&0xffff
    return bytes(out)
def show(path):
    d=open(path,'rb').read()
    p=decrypt(d)
    print("==== %s (%d bytes) ====" % (path, len(d)))
    for pat in [b"Upgrade Scroll", b"High Class", b"Middle Class", b"Low Class", b"Blessed Upgrade"]:
        c=p.count(pat)
        print("  %-16s count=%d" % (pat.decode(), c))
    # sample first few printable strings
    strs=re.findall(rb'[\x20-\x7e]{4,}', p)
    print("  sample:", [s.decode() for s in strs[:8]])
    return p
for f in sys.argv[1:]:
    show(f)
