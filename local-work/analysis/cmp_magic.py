import sys
sys.path.insert(0,"/tmp")
from tbl import parse

CLIENT="/Users/berkgulher/OpenKO-Client/assets/Client/Data/"
T="/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode"

# DB dumps
def load_db(path):
    cur=None; D={}
    for ln in open(path):
        ln=ln.rstrip("\n")
        s=ln.strip()
        if s.startswith("==") and s.endswith("=="):
            cur=s.strip("="); D[cur]=[]; continue
        if cur is None or s=="": continue
        D[cur].append([x.rstrip() for x in ln.split("|")])
    return D
DB=load_db("/tmp/db_magic.txt")

MAGIC_COLS="MagicNum EnName KrName Description BeforeAction TargetAction SelfEffect FlyingEffect TargetEffect Moral SkillLevel Skill Msp HP ItemGroup UseItem CastTime ReCastTime SuccessRate Type1 Type2 Range Etc Event".split()
mb={int(r[0]):r for r in DB["MAGIC"]}

nc,ty,rows=parse(CLIENT+"Skill_Magic_Main_us.tbl")
cl={r[0]:r for r in rows}

CLIENT_COLS="dwID szEngName szName szDesc iSelfAnimID1 iSelfAnimID2 idwTargetAnimID iSelfFX1 iSelfPart1 iSelfFX2 iSelfPart2 iFlyingFX iTargetFX iTargetPart iTarget iNeedLevel iNeedSkill iExhaustMSP iExhaustHP dwNeedItem dwExhaustItem iCastTime iReCastTime fIDK0 fIDK1 iPercentSuccess dw1stTableType dw2ndTableType iValidDist iIDK2".split()

for mid in [101001,101003,110001,120001,200001,300001]:
    if mid not in mb or mid not in cl: 
        print(mid,"absent"); continue
    print("==== id",mid)
    for i,c in enumerate(MAGIC_COLS):
        print("  DB.%-13s=%-12s | CLIENT.%-16s=%s"%(c,mb[mid][i],CLIENT_COLS[i] if i<30 else "",cl[mid][i] if i<30 else ""))
