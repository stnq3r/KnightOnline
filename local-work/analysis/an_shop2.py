import sys, collections, os
sys.path.insert(0,'/tmp')
from tbl import parse
D='/var/folders/kl/17zsy6xn3k15my1gxvknzy400000gn/T/opencode/v1279b/KnightOnline/Data'
def f(n): return os.path.join(D,n)
orgc,orgt,orgs=parse(f('item_org_us.tbl'))
# load ext files
exts={}
for i in range(24):
    fn=f('Item_Ext_%d_us.tbl'%i)
    if not os.path.exists(fn): continue
    c,t,rows=parse(fn)
    ids=[r[0] for r in rows]
    exts[i]=set(ids)
# NPC groups
groups={'101000':'Mori[Arms]','101001':'Maren','103002':'Kamal[Accessory]','103072':'Rawal[Magic Item]',
'201000':'Nauke[Arms]','201001':'Amrase[Handmade Weapon Arms]','202000':'Cubense[Armor]','202001':'Abul[Handmade Armor]',
'203002':'Caspy[Accessory]','203072':'Nanggis[Magic Item]','231000':'Rental[Item Rental]','251000':'Elrond[Siege War Weapons]',
'252000':'Captain Hugor[Maintain]','253000':'Nion[Potions]','254000':'Charon','255000':'Ardin[sundries]'}
for g,name in groups.items():
    o=int(g)//1000; e=int(g)%1000
    n=0; extsused=collections.Counter()
    for r in orgs:
        if r[35]!=o: continue
        be=r[1]
        if be in exts and e in exts[be]:
            n+=1; extsused[be]+=1
    print('%-6s %-28s org=%-4d ext=%-4d  items=%d  exts=%s'%(g,name,o,e,n,dict(extsused)))
