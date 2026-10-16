// UITradePriceDlg.cpp: implementation of the CUITradePriceDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UITradePriceDlg.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

#include <cstdlib>
#include <cstdio>
#include <stdarg.h>

static FILE* MTraceFile() { static FILE* fp=nullptr; if(!fp) { fp=fopen("./merchant-trace.txt","a"); } return fp; }
static void MTrace(const char* fmt, ...) { FILE* fp=MTraceFile(); if(!fp) return; va_list ap; va_start(ap,fmt); vfprintf(fp,fmt,ap); va_end(ap); fputc('\n',fp); fflush(fp); }


CUITradePriceDlg::CUITradePriceDlg()
{
	m_pEditMoney = nullptr;
	m_pBtnOk     = nullptr;
	m_pBtnCancel = nullptr;

	SetVisible(false);
}

CUITradePriceDlg::~CUITradePriceDlg()
{
}

bool CUITradePriceDlg::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pEditMoney, GetChildByID<CN3UIEdit>("edit_money"));
	N3_VERIFY_UI_COMPONENT(m_pBtnOk, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtnCancel, GetChildByID<CN3UIButton>("btn_cancel"));

	return true;
}

void CUITradePriceDlg::Open(std::function<void(int)> fnOnOK, std::function<void()> fnOnCancel)
{
	MTrace("[P-T] Open\n");
	m_fnOnOK     = std::move(fnOnOK);
	m_fnOnCancel = std::move(fnOnCancel);

	if (m_pEditMoney != nullptr)
	{
		m_pEditMoney->SetString("");
		m_pEditMoney->SetFocus();
	}

	SetVisible(true);
}

void CUITradePriceDlg::Close()
{
	MTrace("[P-T] Close\n");
	CN3UIEdit* pFocused = GetFocusedEdit();
	if (pFocused != nullptr)
		pFocused->KillFocus();

	SetVisible(false);
}

bool CUITradePriceDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	MTrace("[P-T] ReceiveMessage sender=%p msg=%u\n", (void*)pSender, dwMsg);
	if (pSender == nullptr || !IsVisible())
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtnOk)
		{
			int iPrice = 0;
			if (m_pEditMoney != nullptr)
				iPrice = std::atoi(m_pEditMoney->GetString().c_str());

			auto fn = m_fnOnOK;
			MTrace("[P-T] OK invoking fn price=%d\n", iPrice);
			Close();
			if (fn)
				fn(iPrice);
			return true;
		}

		if (pSender == m_pBtnCancel)
		{
			auto fn = m_fnOnCancel;
			Close();
			if (fn)
				fn();
			return true;
		}
	}

	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}

bool CUITradePriceDlg::OnKeyPress(int iKey)
{
	MTrace("[P-T] OnKeyPress key=%d\n", iKey);
	switch (iKey)
	{
		case DIK_RETURN:
			ReceiveMessage(m_pBtnOk, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_ESCAPE:
			ReceiveMessage(m_pBtnCancel, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
