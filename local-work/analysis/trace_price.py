f = '/Users/berkgulher/OpenKO-Client/src/Client/WarFare/UITradePriceDlg.cpp'
c = open(f).read()
if '<cstdio>' not in c:
    c = c.replace('#include <cstdlib>', '#include <cstdlib>\n#include <cstdio>')
def ins(text, marker, add):
    i = text.find(marker)
    if i == -1: return text
    j = text.find('\n', i)
    return text[:j+1] + add + text[j+1:]
c = ins(c, 'void CUITradePriceDlg::Open(std::function<void(int)> fnOnOK, std::function<void()> fnOnCancel)\n{',
    '\tstd::fprintf(stderr, "[P-T] Open\\n"); std::fflush(stderr);\n')
c = ins(c, 'void CUITradePriceDlg::Close()\n{',
    '\tstd::fprintf(stderr, "[P-T] Close\\n"); std::fflush(stderr);\n')
c = ins(c, 'bool CUITradePriceDlg::OnKeyPress(int iKey)\n{',
    '\tstd::fprintf(stderr, "[P-T] OnKeyPress key=%d\\n", iKey); std::fflush(stderr);\n')
c = ins(c, 'bool CUITradePriceDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)\n{',
    '\tstd::fprintf(stderr, "[P-T] ReceiveMessage sender=%p msg=%u\\n", (void*)pSender, dwMsg); std::fflush(stderr);\n')
# Before fn(iPrice) in OK
c = c.replace('\t\t\tauto fn = m_fnOnOK;\n\t\t\tClose();',
    '\t\t\tauto fn = m_fnOnOK;\n\t\t\tstd::fprintf(stderr, "[P-T] OK invoking fn price=%d\\n", iPrice); std::fflush(stderr);\n\t\t\tClose();', 1)
open(f, 'w').write(c)
print("price done")
