import sys; sys.path.insert(0,"/tmp")
import gen_item as G
V="/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data/"
org,extf=G.load(V)
rows=G.gen_rows(org,extf)
COLS=G.COLS
header="INSERT INTO [ITEM] (%s) VALUES"%", ".join("[%s]"%c for c in COLS)
def sqlval(c,v):
    if c=="strName": return "N'"+v.replace("'","''")+"'"
    return str(v)
lines=[header]
for i,r in enumerate(rows):
    t="("+", ".join(sqlval(c,r[c]) for c in COLS)+")"
    if i<len(rows)-1: t+=","
    lines.append(t)
out="\n".join(lines)+"\n"
open("/tmp/6_InsertData_ITEM_V1279.sql","w",encoding="utf-8").write(out)
print("rows",len(rows),"lines",len(lines),"bytes",len(out.encode("utf-8")))
# simulate kodb-util parser (ImportBatSize=16)
ls=out.split("\n")
B=16; batches=[]; l=1; r=l+B
hd=ls[0]+"\n"
while l<len(ls):
    if r>=len(ls): r=len(ls)-1
    if len(batches)>0:
        batches[-1]=batches[-1].strip()
        batches[-1]=batches[-1].rstrip(",")
    if l==r:
        if ls[l].strip()=="": break
    batches.append(hd+"\n".join(ls[l:r+1]))
    l=r+1; r+=B
print("batches",len(batches))
bad=0
for b in batches:
    if b.count("(")!=b.count(")"): bad+=1
print("unbalanced batches",bad)
print("sample batch tail:\n"+batches[0][:200]+"\n...\n"+batches[0][-120:])
