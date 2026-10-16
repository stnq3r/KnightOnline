// UITradePriceDlg.h: interface for the CUITradePriceDlg class.
//
// Price entry dialog used when listing a personal-merchant (player shop) item.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEPRICEDLG_H__B2C3D4E5_F6A7_4819_9BCD_EF0123456789__INCLUDED_)
#define AFX_UITRADEPRICEDLG_H__B2C3D4E5_F6A7_4819_9BCD_EF0123456789__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

#include <functional>

class CN3UIEdit;
class CN3UIButton;

class CUITradePriceDlg : public CN3UIBase
{
protected:
	CN3UIEdit* m_pEditMoney;
	CN3UIButton* m_pBtnOk;
	CN3UIButton* m_pBtnCancel;

	std::function<void(int)> m_fnOnOK;
	std::function<void()> m_fnOnCancel;

public:
	CUITradePriceDlg();
	~CUITradePriceDlg() override;

	bool Load(File& file) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	bool OnKeyPress(int iKey) override;
	void Open(std::function<void(int)> fnOnOK, std::function<void()> fnOnCancel);
	void Close();
};

#endif // !defined(AFX_UITRADEPRICEDLG_H__B2C3D4E5_F6A7_4819_9BCD_EF0123456789__INCLUDED_)
