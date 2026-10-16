import struct,glob,os,collections
def dec(d):
    kr,c1,c2=0x0816,0x6081,0x1608;o=bytearray(len(d))
    for i,c in enumerate(d):
        o[i]=c^((kr>>8)&0xff);kr=((c+kr)*c1+c2)&0xffff
    return bytes(o)
SIZE={1:1,2:1,3:2,4:2,5:4,6:4,8:4,9:8}
def parse2(p):
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
    return rows
BASIC=["dwID","byExtIndex","szName","szRemark","dwIDK0","byIDK1","dwIDResrc","dwIDIcon","dwSoundID0","dwSoundID1","byClass","byIsRobeType","byAttachPoint","byNeedRace","byNeedClass","siDamage","siAttackInterval","siAttackRange","siWeight","siMaxDurability","iPrice","iSaleType","siDefense","byContable","dwEffectID1","dwEffectID2","cNeedLevel","cIDK2","byNeedRank","byNeedTitle","byNeedStrength","byNeedStamina","byNeedDexterity","byNeedInteli","byNeedMagicAttack","bySellGroup","byGrade"]
EXT=["dwID","szHeader","dwBaseID","szRemark","dwIDK0","dwIDResrc","dwIDIcon","byMagicOrRare","siDamage","siAttackIntervalPercentage","siHitRate","siEvationRate","siMaxDurability","siPriceMultiply","siDefense","siDefenseRateDagger","siDefenseRateSword","siDefenseRateBlow","siDefenseRateAxe","siDefenseRateSpear","siDefenseRateArrow","byDamageFire","byDamageIce","byDamageThuner","byDamagePoison","byStillHP","byDamageMP","byStillMP","byReturnPhysicalDamage","bySoulBind","siBonusStr","siBonusSta","siBonusDex","siBonusInt","siBonusMagicAttak","siBonusHP","siBonusMSP","siRegistFire","siRegistIce","siRegistElec","siRegistMagic","siRegistPoison","siRegistCurse","dwEffectID1","dwEffectID2","siNeedLevel","siNeedRank","siNeedTitle","siNeedStrength","siNeedStamina","siNeedDexterity","siNeedInteli","siNeedMagicAttack"]
COLS="Num|strName|Kind|Slot|Race|Class|Damage|Delay|Range|Weight|Duration|BuyPrice|SellPrice|Ac|Countable|Effect1|Effect2|ReqLevel|ReqLevelMax|ReqRank|ReqTitle|ReqStr|ReqSta|ReqDex|ReqIntel|ReqCha|SellingGroup|ItemType|Hitrate|Evasionrate|DaggerAc|SwordAc|MaceAc|AxeAc|SpearAc|BowAc|FireDamage|IceDamage|LightningDamage|PoisonDamage|HPDrain|MPDamage|MPDrain|MirrorDamage|Droprate|StrB|StaB|DexB|IntelB|ChaB|MaxHpB|MaxMpB|FireR|ColdR|LightningR|MagicR|PoisonR|CurseR".split("|")
def gen(O,E):
    N=E["dwID"]
    specific = (E["dwBaseID"]!=0 and E["dwBaseID"]==O["dwID"])
    name = E["szHeader"] if specific else "%s (+%d)"%(O["szName"],N%10)
    clamp=lambda v: v if v>0 else 0
    return {
     "Num":O["dwID"]+N,"strName":name,"Kind":O["byClass"],"Slot":O["byAttachPoint"],
     "Race":O["byNeedRace"],"Class":O["byNeedClass"],"Damage":O["siDamage"]+E["siDamage"],
     "Delay":int(O["siAttackInterval"]*E["siAttackIntervalPercentage"]/100),"Range":O["siAttackRange"],
     "Weight":O["siWeight"],"Duration":O["siMaxDurability"]+E["siMaxDurability"],
     "BuyPrice":O["iPrice"]*E["siPriceMultiply"],"SellPrice":0,"Ac":O["siDefense"]+E["siDefense"],
     "Countable":O["byContable"],"Effect1":O["dwEffectID1"]+E["dwEffectID1"],"Effect2":O["dwEffectID2"]+E["dwEffectID2"],
     "ReqLevel":clamp(O["cNeedLevel"]+E["siNeedLevel"]),"ReqLevelMax":clamp(O["cIDK2"]+E["siNeedLevel"]),
     "ReqRank":O["byNeedRank"]+E["siNeedRank"],
     "ReqTitle":O["byNeedTitle"]+E["siNeedTitle"],"ReqStr":O["byNeedStrength"]+E["siNeedStrength"],
     "ReqSta":O["byNeedStamina"]+E["siNeedStamina"],"ReqDex":O["byNeedDexterity"]+E["siNeedDexterity"],
     "ReqIntel":O["byNeedInteli"]+E["siNeedInteli"],"ReqCha":O["byNeedMagicAttack"]+E["siNeedMagicAttack"],
     "SellingGroup":O["bySellGroup"],"ItemType":E["byMagicOrRare"],"Hitrate":E["siHitRate"],"Evasionrate":E["siEvationRate"],
     "DaggerAc":E["siDefenseRateDagger"],"SwordAc":E["siDefenseRateSword"],"MaceAc":E["siDefenseRateBlow"],
     "AxeAc":E["siDefenseRateAxe"],"SpearAc":E["siDefenseRateSpear"],"BowAc":E["siDefenseRateArrow"],
     "FireDamage":E["byDamageFire"],"IceDamage":E["byDamageIce"],"LightningDamage":E["byDamageThuner"],
     "PoisonDamage":E["byDamagePoison"],"HPDrain":E["byStillHP"],"MPDamage":E["byDamageMP"],"MPDrain":E["byStillMP"],
     "MirrorDamage":E["byReturnPhysicalDamage"],"Droprate":0,"StrB":E["siBonusStr"],"StaB":E["siBonusSta"],
     "DexB":E["siBonusDex"],"IntelB":E["siBonusInt"],"ChaB":E["siBonusMagicAttak"],"MaxHpB":E["siBonusHP"],
     "MaxMpB":E["siBonusMSP"],"FireR":E["siRegistFire"],"ColdR":E["siRegistIce"],"LightningR":E["siRegistElec"],
     "MagicR":E["siRegistMagic"],"PoisonR":E["siRegistPoison"],"CurseR":E["siRegistCurse"]}
def load(BK):
    org={r[0]:dict(zip(BASIC,r)) for r in parse2(BK+"Item_Org_us.tbl")}
    extf={}
    for f in glob.glob(BK+"Item_Ext_*.tbl"):
        i=int(os.path.basename(f).split("_")[2].split(".")[0]); extf[i]={r[0]:dict(zip(EXT,r)) for r in parse2(f)}
    return org,extf
if __name__=="__main__":
    BK="item_backup_20261015-035935/Client/"
    org,extf=load(BK)
    db={}
    for ln in open("item_full.txt",encoding="utf-8",errors="replace"):
        ln=ln.rstrip("\n")
        if not ln: continue
        p=ln.split("|")
        if len(p)!=58: continue
        db[int(p[0])]=p
    mism=collections.Counter(); rows=0
    for b,O in org.items():
        for dwid,E in extf.get(O["byExtIndex"],{}).items():
            if not (E["dwBaseID"]==0 or E["dwBaseID"]==b): continue
            num=b+dwid
            if num not in db: continue
            G=gen(O,E); rows+=1
            for i,c in enumerate(COLS):
                if str(G[c])!=str(db[num][i]): mism[c]+=1
    print("compared rows:",rows)
    for c in COLS:
        if mism[c]: print("  %-16s %d"%(c,mism[c]))
