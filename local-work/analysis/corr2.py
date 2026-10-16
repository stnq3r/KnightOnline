import sys
sys.path.insert(0,"/tmp")
from tbl import parse
CLIENT="/Users/berkgulher/OpenKO-Client/assets/Client/Data/"
def load_db(path):
    cur=None; D={}
    for ln in open(path):
        ln=ln.rstrip("\n"); s=ln.strip()
        if s.startswith("==") and s.endswith("=="): cur=s.strip("="); D[cur]=[]; continue
        if cur is None or s=="": continue
        D[cur].append([x.rstrip() for x in ln.split("|")])
    return D
DB=load_db("/tmp/db_magic.txt")
mb={int(r[0]):r for r in DB["MAGIC"]}
nc,ty,rows=parse(CLIENT+"Skill_Magic_Main_us.tbl")
cl={r[0]:r for r in rows}
CLIENT_COLS="dwID szEngName szName szDesc iSelfAnimID1 iSelfAnimID2 idwTargetAnimID iSelfFX1 iSelfPart1 iSelfFX2 iSelfPart2 iFlyingFX iTargetFX iTargetPart iTarget iNeedLevel iNeedSkill iExhaustMSP iExhaustHP dwNeedItem dwExhaustItem iCastTime iReCastTime fIDK0 fIDK1 iPercentSuccess dw1stTableType dw2ndTableType iValidDist iIDK2".split()
MAGIC_COLS="MagicNum EnName KrName Description BeforeAction TargetAction SelfEffect FlyingEffect TargetEffect Moral SkillLevel Skill Msp HP ItemGroup UseItem CastTime ReCastTime SuccessRate Type1 Type2 Range Etc Event".split()
ids=[k for k in mb if k in cl]
def num(v):
    try: return int(v)
    except: return None
for di in [4,5,6,7,22,23]:
    nz=[k for k in ids if num(mb[k][di]) not in (0,None)]
    print("\nDB.%-13s nonzero=%d"%(MAGIC_COLS[di],len(nz)))
    best=[]
    for ci in range(4,30):
        ok=0;tot=0
        for k in nz:
            dv=num(mb[k][di]); cv=cl[k][ci]
            if not isinstance(cv,int): continue
            tot+=1
            if dv==cv: ok+=1
        if tot: best.append((ok/tot,ok,tot,ci))
    best.sort(reverse=True)
    for f,ok,tot,ci in best[:4]:
        print("   %-16s %d%% (%d/%d)"%(CLIENT_COLS[ci],round(f*100),ok,tot))
    print("   samples:")
    for k in nz[:4]:
        print("     id",k,"DB=",mb[k][di],"| ", {CLIENT_COLS[ci]:cl[k][ci] for ci in range(4,16)})
