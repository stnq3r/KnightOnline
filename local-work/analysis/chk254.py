import sys, os
sys.path.insert(0,'/tmp')
from tbl import parse
D='/Users/berkgulher/OpenKO-Client/assets/Client/Data/'
oc,ot,orgs=parse(D+'Item_Org_us.tbl')
exts={}
for i in range(24):
    _,_,rows=parse(D+'Item_Ext_%d_us.tbl'%i)
    exts[i]=set(r[0] for r in rows)
# simulate group 254000 -> org 254, ext 0
o,e=254,0
show=[]
for r in orgs:
    if r[35]!=o: continue
    be=r[1]
    if be in exts and e in exts[be]:
        show.append((r[0],r[2]))
print('group254000 shows %d items:'%len(show))
for i,n in show: print('  ',i,n)
