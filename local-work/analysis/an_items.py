import sys, os, collections
sys.path.insert(0,'/tmp')
from tbl import parse
D='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data'
c,t,orgs=parse(os.path.join(D,'item_org_us.tbl'))
byg=collections.defaultdict(list)
for r in orgs:
    byg[r[35]].append((r[0],r[2]))
for g in sorted(byg):
    if g==0: continue
    print('=== group',g,'(%d items) ==='%len(byg[g]))
    for i,n in byg[g]: print('   ',i,n)
