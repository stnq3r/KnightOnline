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
BASIC=["dwID","byExtIndex","szName","szRemark","dwIDK0","byIDK1","dwIDResrc","dwIDIcon","dwSoundID0","dwSoundID1","byClass","byIsRobeType","byAttachPoint","byNeedRace","byNeedClass","siDamage","siAttackInterval","siAttackRange","siWeight","siMaxDurability","iPrice","iSaleType","siDefense","byContable","dwEffectID1","dwEffectID2","cNeedLevel","cIDK2","byNeedRank","byNeedTitle","byNeedStrength","byNeedStamina","byNeedDexterity","byNeedInteli","byNeedMagicAttack","bySellGroup","byGrade"]
_,org=parse(BK+"Item_Org_us.tbl")
orgby={r[0]:dict(zip(BASIC,r)) for r in org}
extfiles={}
import glob,os
for f in glob.glob(BK+"Item_Ext_*.tbl"):
    i=int(os.path.basename(f).split("_")[2].split(".")[0])
    _,rows=parse(f)
    extfiles[i]={r[0]:r for r in rows}  # dwID -> row
COLS="Num|strName|Kind|Slot|Race|Class|Damage|Delay|Range|Weight|Duration|BuyPrice|SellPrice|Ac|Countable|Effect1|Effect2|ReqLevel|ReqLevelMax|ReqRank|ReqTitle|ReqStr|ReqSta|ReqDex|ReqIntel|ReqCha|SellingGroup|ItemType|Hitrate|Evasionrate|DaggerAc|SwordAc|MaceAc|AxeAc|SpearAc|BowAc|FireDamage|IceDamage|LightningDamage|PoisonDamage|HPDrain|MPDamage|MPDrain|MirrorDamage|Droprate|StrB|StaB|DexB|IntelB|ChaB|MaxHpB|MaxMpB|FireR|ColdR|LightningR|MagicR|PoisonR|CurseR".split("|")
def E(O,N):
    ef=O["byExtIndex"]
    return extfiles.get(ef,{}).get(N)
def gen(O,N):
    e=E(O,N) or [0]*53
    g=lambda nm: e[ ["dwID","szHeader","dwBaseID","szRemark","dwIDK0","dwIDResrc","dwIDIcon","byMagicOrRare","siDamage","siAttackIntervalPercentage","siHitRate","siEvationRate","siMaxDurability","siPriceMultiply","siDefense","siDefenseRateDagger","siDefenseRateSword","siDefenseRateBlow","siDefenseRateAxe","siDefenseRateSpear","siDefenseRateArrow","byDamageFire","byDamageIce","byDamageThuner","byDamagePoison","byStillHP","byDamageMP","byStillMP","byReturnPhysicalDamage","bySoulBind","siBonusStr","siBonusSta","siBonusDex","siBonusInt","siBonusMagicAttak","siBonusHP","siBonusMSP","siRegistFire","siRegistIce","siRegistElec","siRegistMagic","siRegistPoison","siRegistCurse","dwEffectID1","dwEffectID2","siNeedLevel","siNeedRank","siNeedTitle","siNeedStrength","siNeedStamina","siNeedDexterity","siNeedInteli","siNeedMagicAttack"].index(nm)]
    return {
     "Num":O["dwID"]+N,"strName":"%s (+%d)"%(O["szName"],N),"Kind":O["byClass"],"Slot":O["byAttachPoint"],
     "Race":O["byNeedRace"],"Class":O["byNeedClass"],"Damage":O["siDamage"]+g("siDamage"),
     "Delay":int(O["siAttackInterval"]*g("siAttackIntervalPercentage")/100),"Range":O["siAttackRange"],
     "Weight":O["siWeight"],"Duration":O["siMaxDurability"]+g("siMaxDurability"),
     "BuyPrice":O["iPrice"]*g("siPriceMultiply"),"SellPrice":0,"Ac":O["siDefense"]+g("siDefense"),
     "Countable":O["byContable"],"Effect1":O["dwEffectID1"],"Effect2":O["dwEffectID2"],
     "ReqLevel":O["cNeedLevel"]+g("siNeedLevel"),"ReqLevelMax":O["cIDK2"],"ReqRank":O["byNeedRank"]+g("siNeedRank"),
     "ReqTitle":O["byNeedTitle"]+g("siNeedTitle"),"ReqStr":O["byNeedStrength"]+g("siNeedStrength"),
     "ReqSta":O["byNeedStamina"]+g("siNeedStamina"),"ReqDex":O["byNeedDexterity"]+g("siNeedDexterity"),
     "ReqIntel":O["byNeedInteli"]+g("siNeedInteli"),"ReqCha":O["byNeedMagicAttack"]+g("siNeedMagicAttack"),
     "SellingGroup":O["bySellGroup"],"ItemType":O["byIsRobeType"],"Hitrate":g("siHitRate"),"Evasionrate":g("siEvationRate"),
     "DaggerAc":g("siDefenseRateDagger"),"SwordAc":g("siDefenseRateSword"),"MaceAc":g("siDefenseRateBlow"),
     "AxeAc":g("siDefenseRateAxe"),"SpearAc":g("siDefenseRateSpear"),"BowAc":g("siDefenseRateArrow"),
     "FireDamage":g("byDamageFire"),"IceDamage":g("byDamageIce"),"LightningDamage":g("byDamageThuner"),
     "PoisonDamage":g("byDamagePoison"),"HPDrain":g("byStillHP"),"MPDamage":g("byDamageMP"),"MPDrain":g("byStillMP"),
     "MirrorDamage":g("byReturnPhysicalDamage"),"Droprate":g("bySoulBind"),"StrB":g("siBonusStr"),"StaB":g("siBonusSta"),
     "DexB":g("siBonusDex"),"IntelB":g("siBonusInt"),"ChaB":g("siBonusMagicAttak"),"MaxHpB":g("siBonusHP"),
     "MaxMpB":g("siBonusMSP"),"FireR":g("siRegistFire"),"ColdR":g("siRegistIce"),"LightningR":g("siRegistElec"),
     "MagicR":g("siRegistMagic"),"PoisonR":g("siRegistPoison"),"CurseR":g("siRegistCurse")}
mism={c:0 for c in COLS};tot=0;skip=0;mismatchrows=0
examples=[]
for ln in open(T+"/item_full.txt",encoding="utf-8",errors="replace"):
    ln=ln.rstrip("\n")
    if not ln: continue
    parts=ln.split("|")
    if len(parts)!=58: continue
    num=int(parts[0]);O=orgby.get(num-num%1000)
    if not O: skip+=1;continue
    G=gen(O,num%1000)
    tot+=1;bad=0
    for i,c in enumerate(COLS):
        try: v=int(parts[i])
        except: v=parts[i]
        if G[c]!=v: mism[c]+=1;bad+=1
    if bad: 
        mismatchrows+=1
        if len(examples)<8: examples.append((num,parts,G))
print("compared=%d skip(no-org)=%d mismatchRows=%d"%(tot,skip,mismatchrows))
print("--- per-column mismatch counts (nonzero) ---")
for c in COLS:
    if mism[c]: print("  %-16s %d"%(c,mism[c]))
print("--- examples ---")
for num,parts,G in examples[:4]:
    print(" Num",num)
    for i,c in enumerate(COLS):
        try: v=int(parts[i])
        except: v=parts[i]
        if G[c]!=v: print("    %-14s db=%-14s gen=%s"%(c,v,G[c]))
