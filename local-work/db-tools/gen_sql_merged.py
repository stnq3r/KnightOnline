import sys; sys.path.insert(0,"/tmp")
import gen_item as G
V="/Users/berkgulher/OpenKO-Client/assets/Client/Data/"
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
open("/tmp/6_InsertData_ITEM_merged.sql","w",encoding="utf-8").write(out)
print("rows",len(rows),"bytes",len(out.encode("utf-8")))
