import sys
sys.path.insert(0,"/tmp")
from tbl import parse
CLIENT="/Users/berkgulher/OpenKO-Client/assets/Client/Data/"

def load_db(p):
    cur=None; D={}
    for ln in open(p):
        ln=ln.rstrip(); s=ln.strip()
        if s.startswith("==") and s.endswith("=="): cur=s.strip("="); D[cur]=[]; continue
        if cur is None or s=="": continue
        D[cur].append([x.rstrip() for x in ln.split("|")])
    return D
DB=load_db("/tmp/db_magic.txt")

DBc = {
"MAGIC_TYPE1":"iNum Name Description Type HitRate Hit AddDamage Delay ComboType ComboCount ComboDamage Range",
"MAGIC_TYPE2":"iNum Name Description HitType HitRate AddDamage AddRange NeedArrow AddDamagePlus",
"MAGIC_TYPE3":"iNum Name Description Radius Angle DirectType FirstDamage EndDamage TimeDamage Duration Attribute",
"MAGIC_TYPE4":"iNum Name Description BuffType Radius Duration AttackSpeed Speed AC ACPct Attack MagicAttack MaxHP MaxHpPct MaxMP MaxMpPct HitRate AvoidRate Str Sta Dex Intel Cha FireR ColdR LightningR MagicR DiseaseR PoisonR ExpPct",
"MAGIC_TYPE6":"iNum Name Description Size TransformID Duration MaxHp MaxMp Speed AttackSpeed TotalHit TotalAc TotalHitRate TotalEvasionRate TotalFireR TotalColdR TotalLightningR TotalMagicR TotalDiseaseR TotalPoisonR Class UserSkillUse NeedItem SkillSuccessRate MonsterFriendly",
"MAGIC_TYPE7":"nIndex strName strNote byValidGroup byNatoinChange shMonsterNum byTargetChange byStateChange byRadius shHitrate shDuration shDamage byVisoin nNeedItem",
"MAGIC_TYPE9":"iNum Name Description ValidGroup NationChange MonsterNum TargetChange StateChange Radius Hitrate Duration AddDamage Vision NeedItem",
}
pairs=[("MAGIC_TYPE1","Skill_Magic_1.tbl"),("MAGIC_TYPE2","Skill_Magic_2.tbl"),
       ("MAGIC_TYPE3","skill_magic_3.tbl"),("MAGIC_TYPE4","skill_magic_4.tbl"),
       ("MAGIC_TYPE6","Skill_Magic_6.tbl"),("MAGIC_TYPE7","Skill_Magic_7.tbl"),
       ("MAGIC_TYPE9","skill_magic_9.tbl")]
def num(v):
    try: return int(v)
    except: return None
for dt,fn in pairs:
    cols=DBc[dt].split()
    db={int(r[0]):r for r in DB[dt]}
    nc,ty,rows=parse(CLIENT+fn)
    cl={r[0]:r for r in rows}
    ids=[k for k in db if k in cl]
    print("\n### %s (%d cols) vs %s (ncol=%d) common=%d"%(dt,len(cols),fn,nc,len(ids)))
    for di in range(1,len(cols)):
        vals=[num(db[k][di]) for k in ids]
        if all(v in (0,None) for v in vals):
            print("   %-16s = all-zero/na"%(cols[di],)); continue
        best=[]
        for ci in range(nc):
            ok=0;tot=0
            for k in ids:
                dv=num(db[k][di]); cv=cl[k][ci]
                if not isinstance(cv,int): continue
                tot+=1
                if dv==cv: ok+=1
            if tot: best.append((ok/tot,ok,tot,ci))
        best.sort(reverse=True)
        top=", ".join("c%d=%d%%(%d/%d)"%(ci,round(f*100),ok,tot) for f,ok,tot,ci in best[:3])
        print("   %-16s <- %s"%(cols[di],top))
