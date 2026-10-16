import sys, collections
sys.path.insert(0,'/tmp')
from tbl import parse
old='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/item_backup_20261015-035935/Client/Item_Org_us.tbl'
v='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data/item_org_us.tbl'
for name,p in [('OLD',old),('V1279',v)]:
    c,t,rows=parse(p)
    print(name,'ncols',c,'nrows',len(rows))
    print('  idx35 last val of row0:',rows[0][-1])
    g=collections.Counter(r[35] for r in rows)
    print('  bySellGroup(35):',dict(sorted(g.items())))
    # id col0 samples
    # compare org ids set
