import sys, collections
sys.path.insert(0,'/tmp')
from tbl import parse
import os
D='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data'
cols,types,rows=parse(os.path.join(D,'item_org_us.tbl'))
print('ncols',cols,'nrows',len(rows))
# byExtIndex = idx1
extidx=collections.Counter()
grp=collections.Counter()
for r in rows:
    extidx[r[1]]+=1
    grp[r[35]]+=1
print('byExtIndex dist:', dict(sorted(extidx.items())))
print()
print('bySellGroup idx35 dist (org counts):')
for k in sorted(grp): print(' ',k,grp[k])
print()
# also idx34
g34=collections.Counter(r[34] for r in rows)
print('idx34 dist sample:', dict(sorted(g34.items())[:20]))
print('sample row0:',rows[0])
