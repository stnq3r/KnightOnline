import struct
T="/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode"
BK=T+"/item_backup_20261015-035935/Client/"
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
EXT=["dwID","szHeader","dwBaseID","szRemark","dwIDK0","dwIDResrc","dwIDIcon","byMagicOrRare","siDamage","siAttackIntervalPercentage","siHitRate","siEvationRate","siMaxDurability","siPriceMultiply","siDefense","siDefenseRateDagger","siDefenseRateSword","siDefenseRateBlow","siDefenseRateAxe","siDefenseRateSpear","siDefenseRateArrow","byDamageFire","byDamageIce","byDamageThuner","byDamagePoison","byStillHP","byDamageMP","byStillMP","byReturnPhysicalDamage","bySoulBind","siBonusStr","siBonusSta","siBonusDex","siBonusInt","siBonusMagicAttak","siBonusHP","siBonusMSP","siRegistFire","siRegistIce","siRegistElec","siRegistMagic","siRegistPoison","siRegistCurse","dwEffectID1","dwEffectID2","siNeedLevel","siNeedRank","siNeedTitle","siNeedStrength","siNeedStamina","siNeedDexterity","siNeedInteli","siNeedMagicAttack"]
t,rows=parse(BK+"Item_Ext_0_us.tbl")
d={r[0]:r for r in rows}
for n in range(10):
    r=d.get(n)
    print("row %d remark=%r"%(n, r[3] if r else None))
    if r:
        for nm,v in zip(EXT,r):
            if v not in (0,""): print("    %-24s %s"%(nm,v))
