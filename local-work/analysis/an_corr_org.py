import sys
sys.path.insert(0,'/tmp')
from tbl import parse
oldp='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/item_backup_20261015-035935/Client/Item_Org_us.tbl'
vp='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data/item_org_us.tbl'
oc,ot,orows=parse(oldp)
vc,vt,vrows=parse(vp)
om={r[0]:r for r in orows}
vm={r[0]:r for r in vrows}
common=set(om)&set(vm)
print('old',oc,'v',vc,'common',len(common))
# for each old col j, how well matches v idx35 (bySellGroup)
print('match vs V1279 bySellGroup(35):')
for j in range(oc):
    m=sum(1 for i in common if om[i][j]==vm[i][35])
    if m>len(common)*0.5:
        print('  old idx',j,'match',m,'/',len(common))
# also old idx34
print('dist old idx34:', __import__('collections').Counter(r[34] for r in orows).most_common(8))
print('dist old idx35:', __import__('collections').Counter(r[35] for r in orows).most_common(8))
print('dist old idx36:', __import__('collections').Counter(r[36] for r in orows).most_common(8))
