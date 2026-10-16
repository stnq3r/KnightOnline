import struct
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
                (ln,)=struct.unpack_from("<i",b,off);off+=4;cols.append(b[off:off+ln].decode('cp949'));off+=ln
            else:
                sz=SIZE.get(ty,1);cols.append(int.from_bytes(b[off:off+sz],'little',signed=ty in (1,3,5)));off+=sz
        rows.append(cols)
    return t,rows
BASIC=["dwID","byExtIndex","szName","szRemark","dwIDK0","byIDK1","dwIDResrc","dwIDIcon","dwSoundID0","dwSoundID1","byClass","byIsRobeType","byAttachPoint","byNeedRace","byNeedClass","siDamage","siAttackInterval","siAttackRange","siWeight","siMaxDurability","iPrice","iSaleType","siDefense","byContable","dwEffectID1","dwEffectID2","cNeedLevel","cIDK2","byNeedRank","byNeedTitle","byNeedStrength","byNeedStamina","byNeedDexterity","byNeedInteli","byNeedMagicAttack","bySellGroup","byGrade"]
V=T+"/v1279b/KnightOnline/Data/"
t,org=parse(V+"item_org_us.tbl")
print("=== V1279 ORG Dagger (110110000) ===")
for r in org:
    if r[0]==110110000:
        for nm,v in zip(BASIC,r): print("  %-20s %s"%(nm,v))
te,ext=parse(V+"Item_Ext_0_us.tbl")
EXT=["dwID","szHeader","dwBaseID","szRemark","dwIDK0","dwIDResrc","dwIDIcon","byMagicOrRare","siDamage","siAttackIntervalPercentage","siHitRate","siEvationRate","siMaxDurability","siPriceMultiply","siDefense","siDefenseRateDagger","siDefenseRateSword","siDefenseRateBlow","siDefenseRateAxe","siDefenseRateSpear","siDefenseRateArrow","byDamageFire","byDamageIce","byDamageThuner","byDamagePoison","byStillHP","byDamageMP","byStillMP","byReturnPhysicalDamage","bySoulBind","siBonusStr","siBonusSta","siBonusDex","siBonusInt","siBonusMagicAttak","siBonusHP","siBonusMSP","siRegistFire","siRegistIce","siRegistElec","siRegistMagic","siRegistPoison","siRegistCurse","dwEffectID1","dwEffectID2","siNeedLevel","siNeedRank","siNeedTitle","siNeedStrength","siNeedStamina","siNeedDexterity","siNeedInteli","siNeedMagicAttack"]
print("=== V1279 EXT Dagger rows (110110000..009) ===")
for r in ext:
    if 110110000<=r[0]<=110110009:
        print(" -- id",r[0])
        for nm,v in zip(EXT,r):
            if v not in (0,"",): print("     %-26s %s"%(nm,v))
print("=== DB Dagger rows ===")
for ln in open(T+"/dbdump.txt"):
    ln=ln.rstrip("\n")
    if ln.startswith("11011000") and "|" in ln:
        print("  ",ln)
