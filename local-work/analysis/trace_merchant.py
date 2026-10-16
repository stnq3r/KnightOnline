# Insert FILE* trace at key points in UITradeInventory.cpp, UITradePriceDlg.cpp
import sys
# UITradeInventory
f = '/Users/berkgulher/OpenKO-Client/src/Client/WarFare/UITradeInventory.cpp'
c = open(f).read()
# Add static FILE* and open in DumpAllLayouts init (reuse). Simpler: just fprintf to a fixed path.
# Add #include <cstdio> if not present
if '<cstdio>' not in c:
    c = c.replace('#include <fstream>', '#include <fstream>\n#include <cstdio>')
# Add fprintf at Open entry, Close entry, OnPriceEntered entry, MsgSend call, ItemMove functions
import re
def ins_after(text, marker, addition):
    i = text.find(marker)
    if i == -1: return text
    j = text.find('\n', i)
    return text[:j+1] + addition + text[j+1:]

# Open entry
c = ins_after(c, 'void CUITradeInventory::Open()\n{',
    '\tstd::fprintf(stderr, "[M-T] Open entry\\n"); std::fflush(stderr);\n')
# Close entry
c = ins_after(c, 'void CUITradeInventory::Close(bool bCancelToServer)\n{',
    '\tstd::fprintf(stderr, "[M-T] Close entry cancel=%d\\n", (int)bCancelToServer); std::fflush(stderr);\n')
# OnPriceEntered entry
c = ins_after(c, 'void CUITradeInventory::OnPriceEntered(int iPrice)\n{',
    '\tstd::fprintf(stderr, "[M-T] OnPriceEntered price=%d pendInv=%d pendShop=%d\\n", iPrice, m_iPendingInvOrder, m_iPendingShopOrder); std::fflush(stderr);\n')
# Before MsgSend_MerchantItemAdd
c = c.replace('CGameProcedure::s_pProcMain->MsgSend_MerchantItemAdd(',
    'std::fprintf(stderr, "[M-T] sending ITEM_ADD\\n"); std::fflush(stderr); CGameProcedure::s_pProcMain->MsgSend_MerchantItemAdd(', 1)
# Before MsgSend_MerchantInsert (btn_ok)
c = c.replace('CGameProcedure::s_pProcMain->MsgSend_MerchantInsert(szMsg);',
    'std::fprintf(stderr, "[M-T] sending INSERT msgLen=%zu\\n", szMsg.size()); std::fflush(stderr); CGameProcedure::s_pProcMain->MsgSend_MerchantInsert(szMsg);', 1)
# Before MsgSend_MerchantClose (cancel / Close)
c = c.replace('CGameProcedure::s_pProcMain->MsgSend_MerchantClose();',
    'std::fprintf(stderr, "[M-T] sending CLOSE\\n"); std::fflush(stderr); CGameProcedure::s_pProcMain->MsgSend_MerchantClose();', 1)
# ReceiveIconDrop
c = ins_after(c, 'bool CUITradeInventory::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)\n{',
    '\tstd::fprintf(stderr, "[M-T] ReceiveIconDrop spItem=%p\\n", (void*)spItem); std::fflush(stderr);\n')
# ItemMoveFromInvToThis
c = ins_after(c, 'void CUITradeInventory::ItemMoveFromInvToThis()\n{',
    '\tstd::fprintf(stderr, "[M-T] ItemMoveFromInvToThis\\n"); std::fflush(stderr);\n')
# Render entry
c = ins_after(c, 'void CUITradeInventory::Render()\n{',
    '\tstd::fprintf(stderr, "[M-T] Render\\n"); std::fflush(stderr);\n'.replace('\\', ''))
# Actually that last one, the ins_after already inserts literal — need raw \n
# Let me redo the Render one cleanly:
c = re.sub(r'void CUITradeInventory::Render\\(\\)\\n\\{\\n',
    'void CUITradeInventory::Render()\\n{\\n\\tstd::fprintf(stderr, "[M-T] Render\\n"); std::fflush(stderr);\\n', c, count=1)
open(f, 'w').write(c)
print("UITradeInventory done")
