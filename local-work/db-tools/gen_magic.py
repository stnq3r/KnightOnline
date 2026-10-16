import sys, os
sys.path.insert(0,"/tmp")
from tbl import parse

DBDUMP="/tmp/db_magic.txt"

def load_db():
    cur=None; D={}
    for ln in open(DBDUMP):
        ln=ln.rstrip(); s=ln.strip()
        if s.startswith("==") and s.endswith("=="): cur=s.strip("="); D[cur]=[]; continue
        if cur is None or s=="": continue
        D[cur].append([x.rstrip() for x in ln.split("|")])
    return D
DB=load_db()

MAIN_FILE=["Skill_Magic_Main_us.tbl","Skill_Magic_Main_en.tbl","Skill_Magic_Main.tbl"]
def find(base,name):
    for n in name if isinstance(name,list) else [name]:
        p=os.path.join(base,n)
        if os.path.exists(p): return p
    return None

def load_main(base):
    p=find(base,MAIN_FILE)
    nc,ty,rows=parse(p)
    return {r[0]:r for r in rows}, p

# mapping directives:
#  ("C",i) client field i ; ("MAIN_NAME",) ; ("MAIN_DESC",) ; ("KEEP",default) ; ("CONST",v) ; ("FLY",i)
MAIN_MAP=[("C",0),("C",1),("C",2),("C",3),("KEEP","0"),("CONST","0"),("CONST","0"),("FLY",11),
          ("C",12),("C",14),("C",15),("C",16),("C",17),("C",18),("C",19),("C",20),
          ("C",21),("C",22),("C",25),("C",26),("C",27),("C",28),("KEEP","0"),("CONST","0")]
T1_MAP=[("C",0),("MAIN_NAME",),("MAIN_DESC",),("C",1),("C",2),("C",3),("KEEP","0"),("C",4),
        ("C",5),("C",6),("C",7),("C",8)]
T2_MAP=[("C",0),("MAIN_NAME",),("MAIN_DESC",),("C",1),("KEEP","100"),("C",2),("C",4),("C",5),("KEEP","0")]
T3_MAP=[("C",0),("MAIN_NAME",),("MAIN_DESC",),("C",1),("KEEP","0"),("C",2),("C",3),("KEEP","0"),
        ("C",4),("C",5),("C",6)]
T4_MAP=[("C",0),("MAIN_NAME",),("MAIN_DESC",),("C",1),("C",2),("C",3),("C",4),("C",5),("C",6),
        ("C",7),("C",8),("C",9),("C",10),("C",11),("C",12),("C",13),("KEEP","100"),("KEEP","100"),
        ("C",14),("C",15),("C",16),("C",17),("C",18),("C",19),("C",20),("C",21),("C",22),
        ("C",23),("C",24),("C",25)]
T6_MAP=[("C",0),("C",1),("C",2),("C",3),("C",4),("C",5),("C",6),("C",7),("C",8),("C",9),
        ("C",10),("C",11),("C",12),("C",13),("C",14),("C",15),("C",16),("C",17),("C",18),
        ("C",19),("C",21),("C",22),("C",23),("C",24),("C",25)]
T7_MAP=[("C",0),("MAIN_NAME",),("MAIN_DESC",),("KEEP","0"),("KEEP","0"),("KEEP","0"),("KEEP","0"),
        ("KEEP","0"),("C",1),("KEEP","100"),("KEEP","0"),("KEEP","0"),("KEEP","100"),("KEEP","0")]
T9_MAP=[("C",0),("C",1),("C",2),("C",3),("C",4),("C",5),("C",6),("C",7),("C",8),("C",9),
        ("C",10),("C",11),("C",12),("C",13)]

TABLES={
 "MAGIC":(MAIN_MAP,MAIN_FILE),
 "MAGIC_TYPE1":(T1_MAP,"Skill_Magic_1.tbl"),
 "MAGIC_TYPE2":(T2_MAP,"Skill_Magic_2.tbl"),
 "MAGIC_TYPE3":(T3_MAP,"skill_magic_3.tbl"),
 "MAGIC_TYPE4":(T4_MAP,"skill_magic_4.tbl"),
 "MAGIC_TYPE6":(T6_MAP,"Skill_Magic_6.tbl"),
 "MAGIC_TYPE7":(T7_MAP,"Skill_Magic_7.tbl"),
 "MAGIC_TYPE9":(T9_MAP,"skill_magic_9.tbl"),
}

def load_client(base, fname):
    p=find(base,fname)
    if not p: return None
    nc,ty,rows=parse(p)
    return {r[0]:r for r in rows}

def gen_table(table, base):
    mapping,fname=TABLES[table]
    db={int(r[0]):r for r in DB[table]}
    main,mpath=load_main(base)
    cl=load_client(base, fname)
    out={}
    for k,cr in cl.items():
        row=[]
        for spec in mapping:
            if spec[0]=="C":
                v=cr[spec[1]]
            elif spec[0]=="MAIN_NAME":
                v=main.get(k,[None,"",""])[1]
            elif spec[0]=="MAIN_DESC":
                v=main.get(k,[None,"","",""])[3]
            elif spec[0]=="FLY":
                v="1" if cr[spec[1]]!=0 else "0"
            elif spec[0]=="CONST":
                v=spec[1]
            elif spec[0]=="KEEP":
                if k in db and len(db[k])>len(row):
                    v=db[k][len(row)]
                else:
                    v=spec[1]
            row.append(str(v))
        out[k]=row
    return out

def validate(base):
    for table in TABLES:
        gen=gen_table(table,base)
        db={int(r[0]):r for r in DB[table]}
        common=[k for k in gen if k in db]
        ncol=len(gen[list(gen)[0]]) if gen else 0
        colbad=[0]*ncol; tot=0
        for k in common:
            tot+=1
            for i in range(ncol):
                if str(gen[k][i]) != str(db[k][i]):
                    colbad[i]+=1
        print("### %s: gen=%d db=%d common=%d"%(table,len(gen),len(db),len(common)))
        names=DBCOLNAMES[table]
        for i in range(ncol):
            if colbad[i]:
                print("    col%-2d %-16s diff %d/%d"%(i,names[i],colbad[i],tot))
        # ids not in db (new) / not in gen (removed)
        print("    new(only gen): %d  removed(only db): %d"%(len([k for k in gen if k not in db]),len([k for k in db if k not in gen])))

DBCOLNAMES={
"MAGIC":"MagicNum EnName KrName Description BeforeAction TargetAction SelfEffect FlyingEffect TargetEffect Moral SkillLevel Skill Msp HP ItemGroup UseItem CastTime ReCastTime SuccessRate Type1 Type2 Range Etc Event".split(),
"MAGIC_TYPE1":"iNum Name Description Type HitRate Hit AddDamage Delay ComboType ComboCount ComboDamage Range".split(),
"MAGIC_TYPE2":"iNum Name Description HitType HitRate AddDamage AddRange NeedArrow AddDamagePlus".split(),
"MAGIC_TYPE3":"iNum Name Description Radius Angle DirectType FirstDamage EndDamage TimeDamage Duration Attribute".split(),
"MAGIC_TYPE4":"iNum Name Description BuffType Radius Duration AttackSpeed Speed AC ACPct Attack MagicAttack MaxHP MaxHpPct MaxMP MaxMpPct HitRate AvoidRate Str Sta Dex Intel Cha FireR ColdR LightningR MagicR DiseaseR PoisonR ExpPct".split(),
"MAGIC_TYPE6":"iNum Name Description Size TransformID Duration MaxHp MaxMp Speed AttackSpeed TotalHit TotalAc TotalHitRate TotalEvasionRate TotalFireR TotalColdR TotalLightningR TotalMagicR TotalDiseaseR TotalPoisonR Class UserSkillUse NeedItem SkillSuccessRate MonsterFriendly".split(),
"MAGIC_TYPE7":"nIndex strName strNote byValidGroup byNatoinChange shMonsterNum byTargetChange byStateChange byRadius shHitrate shDuration shDamage byVisoin nNeedItem".split(),
"MAGIC_TYPE9":"iNum Name Description ValidGroup NationChange MonsterNum TargetChange StateChange Radius Hitrate Duration AddDamage Vision NeedItem".split(),
}

NAME50={"Name","EnName","KrName","strName"}
DESC100={"Description","strNote"}
def collimit(colname):
    if colname in NAME50: return 50
    if colname in DESC100: return 100
    return None

import re as _re
def load_types(path="/tmp/create_magic.sql"):
    T={}
    txt=open(path).read()
    for t,body in _re.findall(r'CREATE TABLE \[(\w+)\] \((.*?)\n\)', txt, _re.S):
        for c,ty in _re.findall(r'\[(\w+)\]\s+(\w+)', body):
            if ty in ("tinyint","smallint","int","bigint","varchar","nvarchar"):
                T[(t,c)]=ty
    return T
TYPES=load_types()
RANGE={"tinyint":(0,255),"smallint":(-32768,32767),"int":(-2147483648,2147483647)}

def sql_quote(v, limit=None, coltype=None):
    s=str(v)
    if s.upper()=="NULL": return "NULL"
    try:
        n=int(s)
        if coltype in RANGE:
            lo,hi=RANGE[coltype]
            n=max(lo,min(hi,n))
        return str(n)
    except: pass
    if limit and len(s)>limit: s=s[:limit]
    if s=="" : return "N''"
    return "N'"+s.replace("'","''")+"'"

def emit_single(table, rows):
    names=DBCOLNAMES[table]
    hdr="INSERT INTO [%s] ([%s]) VALUES"%(table, "], [".join(names))
    lines=[hdr]
    ks=sorted(rows)
    for i,k in enumerate(ks):
        tup="("+", ".join(sql_quote(v, collimit(names[j]), TYPES.get((table,names[j]))) for j,v in enumerate(rows[k]))+")"
        if i<len(ks)-1: tup+=","
        lines.append(tup)
    return "\n".join(lines)+"\n"

def emit_chunked(table, rows, chunk=500):
    names=DBCOLNAMES[table]
    hdr="INSERT INTO [%s] ([%s]) VALUES"%(table, "], [".join(names))
    ks=sorted(rows)
    lines=[]
    for start in range(0,len(ks),chunk):
        part=ks[start:start+chunk]
        lines.append(hdr)
        for i,k in enumerate(part):
            tup="("+", ".join(sql_quote(v, collimit(names[j]), TYPES.get((table,names[j]))) for j,v in enumerate(rows[k]))+")"
            if i<len(part)-1: tup+=","
            lines.append(tup)
        lines.append("GO")
    return "\n".join(lines)+"\n"

def gen_sql(base, outdir):
    os.makedirs(outdir, exist_ok=True)
    for table in TABLES:
        rows=gen_table(table, base)
        sql=emit_single(table, rows)
        p=os.path.join(outdir,"6_InsertData_%s.sql"%table)
        open(p,"w").write(sql)
        print("%-14s %d rows -> %s"%(table,len(rows),p))

def gen_apply(base, outpath):
    out=["USE [KN_online]","GO"]
    rows_by={}
    for table in TABLES:
        out.append("DELETE FROM [%s];"%table); out.append("GO")
    for table in TABLES:
        rows=gen_table(table, base); rows_by[table]=rows
        out.append(emit_chunked(table, rows).rstrip()); out.append("GO")
    open(outpath,"w").write("\n".join(out)+"\n")
    print("apply ->",outpath,os.path.getsize(outpath),"bytes")

if __name__=="__main__":
    mode=sys.argv[1]; base=sys.argv[2]
    if mode=="validate": validate(base)
    elif mode=="gen": gen_sql(base, sys.argv[3])
    elif mode=="apply": gen_apply(base, sys.argv[3])
