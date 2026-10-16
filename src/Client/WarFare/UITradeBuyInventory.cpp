// UITradeBuyInventory.cpp: implementation of the CUITradeBuyInventory class.
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UITradeBuyInventory.h"
#include "UITradeInventory.h"
#include "CountableItemEditDlg.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "UIInventory.h"
#include "UIImageTooltipDlg.h"
#include "UIManager.h"
#include "APISocket.h"
#include "GameBase.h"
#include "LocalInput.h"
#include "PacketDef.h"

#include <shared/packets.h>

#include <algorithm>
#include <limits>

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIArea.h>
#include <N3Base/N3UIImage.h>
#include "N3UIIcon.h"

CUITradeBuyInventory::CUITradeBuyInventory()
{
	m_pStrMoney    = nullptr;
	m_pUITooltipDlg = nullptr;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		m_pShop[i]      = nullptr;
		m_pStrPrice[i]  = nullptr;
		m_iShopPrice[i] = 0;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		m_pMyInv[i] = nullptr;

	m_iMerchantUID = -1;
	m_iPendingShopOrder = -1;
	m_iPendingInvOrder = -1;

	SetVisible(false);
}

CUITradeBuyInventory::~CUITradeBuyInventory()
{
	CUITradeBuyInventory::Release();
}

void CUITradeBuyInventory::Release()
{
	CN3UIBase::Release();

	delete m_pUITooltipDlg;
	m_pUITooltipDlg = nullptr;

	ClearShopSlots();

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyInv[i] != nullptr)
		{
			delete m_pMyInv[i]->pUIIcon;
			m_pMyInv[i]->pUIIcon = nullptr;
			delete m_pMyInv[i];
			m_pMyInv[i] = nullptr;
		}
	}
}

void CUITradeBuyInventory::InitIconWnd(e_UIWND eWnd)
{
	__TABLE_UI_RESRC* pTblUI = CGameBase::s_pTbl_UI.Find(CGameBase::s_pPlayer->m_InfoBase.eNation);

	m_pUITooltipDlg = new CUIImageTooltipDlg();
	if (m_pUITooltipDlg != nullptr && pTblUI != nullptr)
	{
		m_pUITooltipDlg->Init(this);
		m_pUITooltipDlg->LoadFromFile(pTblUI->szItemInfo);
		m_pUITooltipDlg->InitPos();
		m_pUITooltipDlg->SetVisible(false);
	}

	CN3UIWndBase::InitIconWnd(eWnd);

	for (CN3UIBase* pChild : m_Children)
	{
		if (pChild->UIType() != UI_TYPE_AREA)
			continue;

		const std::string& id = pChild->GetID();
		CN3UIArea* pArea      = static_cast<CN3UIArea*>(pChild);

		if (id.rfind("at", 0) == 0 && id.size() > 2)
			pArea->m_eAreaType = UI_AREA_TYPE_PER_TRADE_OTHER;
		else if (id.rfind("a", 0) == 0 && id.size() > 1)
			pArea->m_eAreaType = UI_AREA_TYPE_PER_TRADE_INV;
	}

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_pStrPrice[i] = GetChildByID<CN3UIString>("st" + std::to_string(i));

	m_pStrMoney = GetChildByID<CN3UIString>("text_money");
}

void CUITradeBuyInventory::InitIconUpdate()
{
	constexpr float UVAspect = 45.0f / 64.0f;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] == nullptr)
			continue;

		m_pShop[i]->pUIIcon = new CN3UIIcon();
		m_pShop[i]->pUIIcon->Init(this);
		m_pShop[i]->pUIIcon->SetTex(m_pShop[i]->szIconFN);
		m_pShop[i]->pUIIcon->SetUVRect(0, 0, UVAspect, UVAspect);
		m_pShop[i]->pUIIcon->SetUIType(UI_TYPE_ICON);
		m_pShop[i]->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);

		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, i);
		if (pArea != nullptr)
		{
			m_pShop[i]->pUIIcon->SetRegion(pArea->GetRegion());
			m_pShop[i]->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		if (m_pShop[i]->iDurability == 0)
			m_pShop[i]->pUIIcon->SetStyle(m_pShop[i]->pUIIcon->GetStyle() | UISTYLE_DURABILITY_EXHAUST);
	}
}

CN3UIArea* CUITradeBuyInventory::GetChildAreaByiOrder(eUI_AREA_TYPE eUAT, int iOrder)
{
	std::string szID;
	if (eUAT == UI_AREA_TYPE_PER_TRADE_OTHER)
		szID = "at" + std::to_string(iOrder);
	else if (eUAT == UI_AREA_TYPE_PER_TRADE_INV)
		szID = "a" + std::to_string(iOrder);
	else
		return CN3UIWndBase::GetChildAreaByiOrder(eUAT, iOrder);

	for (CN3UIBase* pChild : m_Children)
	{
		if (pChild->UIType() == UI_TYPE_AREA && pChild->GetID() == szID)
			return static_cast<CN3UIArea*>(pChild);
	}

	return nullptr;
}

__IconItemSkill* CUITradeBuyInventory::GetHighlightIconItem(CN3UIIcon* pUIIcon)
{
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] != nullptr && m_pShop[i]->pUIIcon == pUIIcon)
			return m_pShop[i];
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyInv[i] != nullptr && m_pMyInv[i]->pUIIcon == pUIIcon)
			return m_pMyInv[i];
	}

	return nullptr;
}

int CUITradeBuyInventory::GetInvOrder(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyInv[i] == spItem)
			return i;
	}
	return -1;
}

int CUITradeBuyInventory::GetShopOrder(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] == spItem)
			return i;
	}
	return -1;
}

RECT CUITradeBuyInventory::GetSampleRect()
{
	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, 0);
	if (pArea == nullptr)
		return {};

	POINT ptCur   = CGameProcedure::s_pLocalInput->MouseGetPos();
	RECT rect     = pArea->GetRegion();
	float fWidth  = static_cast<float>(rect.right - rect.left) * 0.5f;
	float fHeight = static_cast<float>(rect.bottom - rect.top) * 0.5f;
	rect.left     = ptCur.x - static_cast<int>(fWidth);
	rect.right    = ptCur.x + static_cast<int>(fWidth);
	rect.top      = ptCur.y - static_cast<int>(fHeight);
	rect.bottom   = ptCur.y + static_cast<int>(fHeight);
	return rect;
}

void CUITradeBuyInventory::ItemMoveFromInvToThis()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (pInven->m_pMyInvWnd[i] == nullptr)
			continue;

		__IconItemSkill* spItem = pInven->m_pMyInvWnd[i];
		spItem->pUIIcon->SetParent(this);
		pInven->m_pMyInvWnd[i] = nullptr;

		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, i);
		if (pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		m_pMyInv[i] = spItem;
	}
}

void CUITradeBuyInventory::ItemMoveFromThisToInv()
{
	CUIInventory* pInven = CGameProcedure::s_pProcMain->m_pUIInventory;
	if (pInven == nullptr)
		return;

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyInv[i] == nullptr)
			continue;

		__IconItemSkill* spItem = m_pMyInv[i];
		spItem->pUIIcon->SetParent(pInven);
		m_pMyInv[i] = nullptr;

		CN3UIArea* pArea = pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, i);
		if (pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}

		pInven->m_pMyInvWnd[i] = spItem;
	}
}

void CUITradeBuyInventory::ClearShopSlots()
{
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] != nullptr)
		{
			RemoveChild(m_pShop[i]->pUIIcon);
			if (m_pShop[i]->pUIIcon != nullptr)
			{
				m_pShop[i]->pUIIcon->Release();
				delete m_pShop[i]->pUIIcon;
				m_pShop[i]->pUIIcon = nullptr;
			}
			delete m_pShop[i];
			m_pShop[i] = nullptr;
		}

		m_iShopPrice[i] = 0;

		if (m_pStrPrice[i] != nullptr)
			m_pStrPrice[i]->SetVisible(false);
	}
}

void CUITradeBuyInventory::Open(int iMerchantUID)
{
	m_iMerchantUID = iMerchantUID;
	m_iPendingShopOrder = -1;
	m_iPendingInvOrder = -1;
	m_iPendingPurchaseCount = 0;
	m_bAwaitingPurchaseConfirmation = false;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_iShopPrice[i] = 0;

	ClearShopSlots();
	ItemMoveFromInvToThis();
	UpdateMoneyDisplay();
	SetVisible(true);
}

void CUITradeBuyInventory::Close(bool bCancelToServer)
{
	const int merchantUID = m_iMerchantUID;
	if (CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->IsVisible()
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWnd() == UIWND_PER_TRADE
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWndDistrict() == UIWND_DISTRICT_PER_TRADE_OTHER)
		CN3UIWndBase::s_pCountableItemEdit->Close();
	m_iPendingShopOrder = -1;
	m_iPendingInvOrder = -1;
	m_iPendingPurchaseCount = 0;
	m_bAwaitingPurchaseConfirmation = false;
	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();

	SetState(UI_STATE_COMMON_NONE);
	AllHighLightIconFree();

	ItemMoveFromThisToInv();
	ClearShopSlots();
	m_iMerchantUID = -1;
	UpdateMoneyDisplay();
	SetVisible(false);

	if (bCancelToServer && merchantUID >= 0 && CGameProcedure::s_pProcMain != nullptr)
		CGameProcedure::s_pProcMain->MsgSend_MerchantTradeCancel();
}

void CUITradeBuyInventory::UpdateMoneyDisplay()
{
	if (m_pStrMoney == nullptr)
		return;

	if (CGameBase::s_pPlayer == nullptr)
	{
		m_pStrMoney->SetVisible(false);
		return;
	}

	m_pStrMoney->SetVisible(true);
	m_pStrMoney->SetStringAsInt(static_cast<int>(CGameBase::s_pPlayer->m_InfoExt.iGold));
}

void CUITradeBuyInventory::Render()
{
	if (!m_bVisible)
		return;

	POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
	if (m_pUITooltipDlg != nullptr)
		m_pUITooltipDlg->DisplayTooltipsDisable();

	bool bTooltipRender     = false;
	__IconItemSkill* spItem = nullptr;

	for (UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (GetState() == UI_STATE_ICON_MOVING && pChild->UIType() == UI_TYPE_ICON
			&& CN3UIWndBase::s_sSelectedIconInfo.pItemSelect != nullptr
			&& static_cast<CN3UIIcon*>(pChild) == CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon)
			continue;

		pChild->Render();

		if (GetState() == UI_STATE_COMMON_NONE && pChild->UIType() == UI_TYPE_ICON
			&& (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
		{
			bTooltipRender = true;
			spItem         = GetHighlightIconItem(static_cast<CN3UIIcon*>(pChild));
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		CN3UIString* pStr = GetChildStringByiOrder(i + 200);
		if (pStr == nullptr)
			continue;

		if (m_pMyInv[i] != nullptr && m_pMyInv[i]->IsStackable())
		{
			if (GetState() == UI_STATE_ICON_MOVING && m_pMyInv[i] == CN3UIWndBase::s_sSelectedIconInfo.pItemSelect)
			{
				pStr->SetVisible(false);
			}
			else
			{
				pStr->SetVisible(true);
				pStr->SetStringAsInt(m_pMyInv[i]->iCount);
				pStr->Render();
			}
		}
		else
		{
			pStr->SetVisible(false);
		}
	}

	if (GetState() == UI_STATE_ICON_MOVING && CN3UIWndBase::s_sSelectedIconInfo.pItemSelect != nullptr)
		CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon->Render();

	if (bTooltipRender && spItem != nullptr && m_pUITooltipDlg != nullptr)
	{
		// Hovering a shop listing shows the price the buyer would pay.
		const int iShopOrder = GetShopOrder(spItem);
		if (iShopOrder != -1 && m_iShopPrice[iShopOrder] > 0)
		{
			m_pUITooltipDlg->SetCustomPrice(m_iShopPrice[iShopOrder]);
			m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, true, true);
		}
		else
		{
			m_pUITooltipDlg->SetCustomPrice(-1);
			m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem);
		}
	}
}

uint32_t CUITradeBuyInventory::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!IsVisible())
	{
		dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
		return dwRet;
	}

	if (GetState() == UI_STATE_ICON_MOVING
		&& (CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_PER_TRADE
			|| CN3UIWndBase::s_sSelectedIconInfo.UIWndSelect.UIWnd == UIWND_UNKNOWN))
	{
		CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
		CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
	}

	return CN3UIWndBase::MouseProc(dwFlags, ptCur, ptOld);
}

void CUITradeBuyInventory::IconRestore()
{
	__IconItemSkill* spItem = CN3UIWndBase::s_sSelectedIconInfo.pItemSelect;
	if (spItem == nullptr)
		return;

	int iOrder = GetInvOrder(spItem);
	if (iOrder != -1)
	{
		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, iOrder);
		if (pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}
		return;
	}

	iOrder = GetShopOrder(spItem);
	if (iOrder != -1)
	{
		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, iOrder);
		if (pArea != nullptr)
		{
			spItem->pUIIcon->SetRegion(pArea->GetRegion());
			spItem->pUIIcon->SetMoveRect(pArea->GetRegion());
		}
	}
}

void CUITradeBuyInventory::CancelIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUITradeBuyInventory::AcceptIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

bool CUITradeBuyInventory::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	if (!m_bVisible)
		return false;

	// Source must be one of the merchant's shop slots.
	int iShopOrder = GetShopOrder(spItem);
	if (iShopOrder == -1 || spItem == nullptr)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	// Find the destination empty inventory slot.
	int iDestOrder = -1;
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, i);
		if (pArea != nullptr && pArea->IsIn(ptCur.x, ptCur.y) && m_pMyInv[i] == nullptr)
		{
			iDestOrder = i;
			break;
		}
	}

	if (iDestOrder == -1)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	IconRestore();

	if (spItem->pItemBasic == nullptr || spItem->pItemExt == nullptr)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	const bool countable = spItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
		|| spItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL;
	if (countable && CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->IsVisible())
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	if (countable && CN3UIWndBase::s_pCountableItemEdit != nullptr)
	{
		m_iPendingShopOrder = iShopOrder;
		m_iPendingInvOrder = iDestOrder;
		CN3UIWndBase::s_pCountableItemEdit->Open(
			UIWND_PER_TRADE, UIWND_DISTRICT_PER_TRADE_OTHER, false);
	}
	else if (CGameProcedure::s_pProcMain != nullptr)
	{
		const int count = countable ? spItem->iCount : 1;
		if (count > 0)
			BeginPurchaseConfirmation(iShopOrder, iDestOrder, count);
	}

	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
	return true;
}

void CUITradeBuyInventory::ItemCountOK()
{
	CCountableItemEditDlg* pCountDlg = CN3UIWndBase::s_pCountableItemEdit;
	if (pCountDlg == nullptr || !pCountDlg->IsVisible()
		|| pCountDlg->GetCallerWnd() != UIWND_PER_TRADE
		|| pCountDlg->GetCallerWndDistrict() != UIWND_DISTRICT_PER_TRADE_OTHER)
		return;

	const int shopOrder = m_iPendingShopOrder;
	const int invOrder = m_iPendingInvOrder;
	const int count = pCountDlg->GetQuantity();
	pCountDlg->Close();
	m_iPendingShopOrder = -1;
	m_iPendingInvOrder = -1;

	if (!IsVisible() || m_iMerchantUID < 0 || shopOrder < 0 || shopOrder >= MAX_ITEM_PER_TRADE
		|| invOrder < 0 || invOrder >= MAX_ITEM_INVENTORY || m_pShop[shopOrder] == nullptr
		|| m_pShop[shopOrder]->pItemBasic == nullptr || m_pShop[shopOrder]->pItemExt == nullptr
		|| m_pMyInv[invOrder] != nullptr || count <= 0 || count > m_pShop[shopOrder]->iCount
		|| CGameProcedure::s_pProcMain == nullptr)
		return;

	BeginPurchaseConfirmation(shopOrder, invOrder, count);
}

void CUITradeBuyInventory::BeginPurchaseConfirmation(int shopOrder, int invOrder, int count)
{
	if (!IsVisible() || m_iMerchantUID < 0 || shopOrder < 0 || shopOrder >= MAX_ITEM_PER_TRADE
		|| invOrder < 0 || invOrder >= MAX_ITEM_INVENTORY || m_pShop[shopOrder] == nullptr
		|| m_pShop[shopOrder]->pItemBasic == nullptr || m_pShop[shopOrder]->pItemExt == nullptr
		|| m_pMyInv[invOrder] != nullptr || count <= 0 || count > m_pShop[shopOrder]->iCount
		|| m_iShopPrice[shopOrder] <= 0 || CGameProcedure::s_pProcMain == nullptr
		|| !CGameProcedure::s_pProcMain->IsMerchantOwnerActive(static_cast<int16_t>(m_iMerchantUID)))
		return;

	m_iPendingShopOrder = shopOrder;
	m_iPendingInvOrder = invOrder;
	m_iPendingPurchaseCount = count;
	m_bAwaitingPurchaseConfirmation = true;

	const int64_t totalPrice = static_cast<int64_t>(m_iShopPrice[shopOrder]) * count;
	const std::string prompt = "Buy " + std::to_string(count) + " item(s) for "
		+ std::to_string(totalPrice) + " Noah?";
	CGameProcedure::MessageBoxPost(prompt, "", MB_YESNO, BEHAVIOR_MERCHANT_PURCHASE);
}

void CUITradeBuyInventory::ResolvePurchaseConfirmation(bool accepted)
{
	if (!m_bAwaitingPurchaseConfirmation)
		return;

	const int shopOrder = m_iPendingShopOrder;
	const int invOrder = m_iPendingInvOrder;
	const int count = m_iPendingPurchaseCount;
	m_iPendingShopOrder = -1;
	m_iPendingInvOrder = -1;
	m_iPendingPurchaseCount = 0;
	m_bAwaitingPurchaseConfirmation = false;

	if (!accepted || !IsVisible() || m_iMerchantUID < 0 || shopOrder < 0
		|| shopOrder >= MAX_ITEM_PER_TRADE || invOrder < 0 || invOrder >= MAX_ITEM_INVENTORY
		|| m_pShop[shopOrder] == nullptr || m_pShop[shopOrder]->pItemBasic == nullptr
		|| m_pShop[shopOrder]->pItemExt == nullptr || m_pMyInv[invOrder] != nullptr
		|| count <= 0 || count > m_pShop[shopOrder]->iCount
		|| CGameProcedure::s_pProcMain == nullptr
		|| !CGameProcedure::s_pProcMain->IsMerchantOwnerActive(static_cast<int16_t>(m_iMerchantUID)))
		return;

	const int itemID = static_cast<int>(m_pShop[shopOrder]->pItemBasic->dwID
		+ m_pShop[shopOrder]->pItemExt->dwID);
	CGameProcedure::s_pProcMain->MsgSend_MerchantItemBuy(itemID, count, shopOrder, invOrder);
}

void CUITradeBuyInventory::ItemCountCancel()
{
	m_iPendingShopOrder = -1;
	m_iPendingInvOrder = -1;
	m_iPendingPurchaseCount = 0;
	if (CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWnd() == UIWND_PER_TRADE
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWndDistrict() == UIWND_DISTRICT_PER_TRADE_OTHER)
		CN3UIWndBase::s_pCountableItemEdit->Close();
}

bool CUITradeBuyInventory::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->GetID() == "btn_ok")
		{
			Close();
			return true;
		}
	}

	switch (dwMsg & 0x000f0000)
	{
		case UIMSG_ICON_DOWN_FIRST:
		{
			AllHighLightIconFree();

			__IconItemSkill* spItem = GetHighlightIconItem(static_cast<CN3UIIcon*>(pSender));
			if (spItem == nullptr)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			int iShopOrder = GetShopOrder(spItem);
			if (iShopOrder == -1)
			{
				// Inventory icon drags inside this window have no effect for now.
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			s_sSelectedIconInfo.UIWndSelect.UIWnd         = UIWND_PER_TRADE;
			s_sSelectedIconInfo.UIWndSelect.UIWndDistrict = UIWND_DISTRICT_PER_TRADE_OTHER;
			s_sSelectedIconInfo.UIWndSelect.iOrder        = iShopOrder;
			s_sSelectedIconInfo.pItemSelect               = spItem;

			static_cast<CN3UIIcon*>(pSender)->SetRegion(GetSampleRect());
			static_cast<CN3UIIcon*>(pSender)->SetMoveRect(GetSampleRect());

			PlayItemSound(spItem->pItemBasic);
		}
		break;

		case UIMSG_ICON_DOWN:
			if (GetState() == UI_STATE_ICON_MOVING && s_sSelectedIconInfo.pItemSelect != nullptr)
			{
				s_sSelectedIconInfo.pItemSelect->pUIIcon->SetRegion(GetSampleRect());
				s_sSelectedIconInfo.pItemSelect->pUIIcon->SetMoveRect(GetSampleRect());
			}
			break;

		case UIMSG_ICON_UP:
			if (s_sSelectedIconInfo.pItemSelect != nullptr)
			{
				if (!CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(s_sSelectedIconInfo.pItemSelect))
					IconRestore();

				PlayItemSound(s_sSelectedIconInfo.pItemSelect->pItemBasic);
			}
			break;

		default:
			break;
	}

	return true;
}

// ---------------------------------------------------------------------------
// Server callbacks
// ---------------------------------------------------------------------------

void CUITradeBuyInventory::ReceiveItemList(const int* piItemIDs, const int* piCounts, const int* piDurabilities, const int* piPrices)
{
	ClearShopSlots();

	if (piItemIDs == nullptr || piCounts == nullptr || piPrices == nullptr)
		return;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		int iID  = piItemIDs[i];
		int iCnt = piCounts[i];
		if (iID == 0 || iCnt == 0)
			continue;

		__TABLE_ITEM_BASIC* pItem  = CGameProcedure::s_pTbl_Items_Basic.Find(iID / 1000 * 1000);
		__TABLE_ITEM_EXT* pItemExt = nullptr;
		if (pItem != nullptr && pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
			pItemExt = CGameProcedure::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iID % 1000);

		if (pItem == nullptr || pItemExt == nullptr)
		{
			CLogWriter::Write("Merchant list - Unknown Item %d", iID);
			continue;
		}

		e_PartPosition ePart = PART_POS_UNKNOWN;
		e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
		std::string szIconFN;
		e_ItemType eType = CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &szIconFN, ePart, ePlug,
			CGameBase::s_pPlayer->m_InfoBase.eRace);
		if (eType == ITEM_TYPE_UNKNOWN)
		{
			CLogWriter::Write("Merchant list - Unknown Item Type %d", iID);
			continue;
		}

		__IconItemSkill* spItem     = new __IconItemSkill;
		spItem->pItemBasic          = pItem;
		spItem->pItemExt            = pItemExt;
		spItem->szIconFN            = szIconFN;
		spItem->iCount              = iCnt;
		spItem->iDurability         = piDurabilities != nullptr ? piDurabilities[i] : 0;

		m_pShop[i]                  = spItem;
		m_iShopPrice[i]             = piPrices[i];

		if (m_pStrPrice[i] != nullptr)
		{
			m_pStrPrice[i]->SetVisible(true);
			m_pStrPrice[i]->SetStringAsInt(piPrices[i]);
		}
	}

	InitIconUpdate();
}

void CUITradeBuyInventory::ReceiveItemPurchaseUpdate(int iItemID, int iRemainingCount, int iShopSlot)
{
	if (iShopSlot < 0 || iShopSlot >= MAX_ITEM_PER_TRADE || iRemainingCount < 0
		|| m_pShop[iShopSlot] == nullptr || m_pShop[iShopSlot]->pItemBasic == nullptr
		|| m_pShop[iShopSlot]->pItemExt == nullptr
		|| static_cast<int>(m_pShop[iShopSlot]->pItemBasic->dwID + m_pShop[iShopSlot]->pItemExt->dwID) != iItemID
		|| iRemainingCount > m_pShop[iShopSlot]->iCount)
		return;

	if (iRemainingCount > 0)
	{
		m_pShop[iShopSlot]->iCount = iRemainingCount;
		return;
	}

	RemoveChild(m_pShop[iShopSlot]->pUIIcon);
	if (m_pShop[iShopSlot]->pUIIcon != nullptr)
	{
		m_pShop[iShopSlot]->pUIIcon->Release();
		delete m_pShop[iShopSlot]->pUIIcon;
		m_pShop[iShopSlot]->pUIIcon = nullptr;
	}
	delete m_pShop[iShopSlot];
	m_pShop[iShopSlot] = nullptr;
	m_iShopPrice[iShopSlot] = 0;
	if (m_pStrPrice[iShopSlot] != nullptr)
		m_pStrPrice[iShopSlot]->SetVisible(false);
}

void CUITradeBuyInventory::ReceiveItemCountChange(int iInvOrder, int iCount, int iItemID, int iDurability)
{
	if (!m_bVisible || iInvOrder < 0 || iInvOrder >= MAX_ITEM_INVENTORY
		|| iItemID <= 0 || iCount < 0 || iDurability < -1
		|| iCount > std::numeric_limits<int16_t>::max()
		|| iDurability > std::numeric_limits<int16_t>::max())
		return;

	__IconItemSkill*& slot = m_pMyInv[iInvOrder];
	if (iCount == 0)
	{
		if (slot == nullptr || slot->GetItemID() != iItemID)
			return;

		RemoveChild(slot->pUIIcon);
		if (slot->pUIIcon != nullptr)
		{
			slot->pUIIcon->Release();
			delete slot->pUIIcon;
		}
		delete slot;
		slot = nullptr;
		return;
	}

	if (slot != nullptr && slot->GetItemID() == iItemID)
	{
		slot->iCount = iCount;
		if (iDurability >= 0)
			slot->iDurability = iDurability;
		return;
	}

	__TABLE_ITEM_BASIC* pItem = CGameProcedure::s_pTbl_Items_Basic.Find(iItemID / 1000 * 1000);
	if (pItem == nullptr || pItem->byExtIndex < 0 || pItem->byExtIndex >= MAX_ITEM_EXTENSION)
		return;

	__TABLE_ITEM_EXT* pItemExt = CGameProcedure::s_pTbl_Items_Exts[pItem->byExtIndex].Find(iItemID % 1000);
	if (pItemExt == nullptr || CGameBase::s_pPlayer == nullptr)
		return;

	e_PartPosition part = PART_POS_UNKNOWN;
	e_PlugPosition plug = PLUG_POS_UNKNOWN;
	std::string iconFN;
	if (CGameBase::MakeResrcFileNameForUPC(pItem, pItemExt, nullptr, &iconFN, part, plug,
		CGameBase::s_pPlayer->m_InfoBase.eRace) == ITEM_TYPE_UNKNOWN || iconFN.empty())
		return;

	__IconItemSkill* replacement = new __IconItemSkill;
	replacement->pItemBasic = pItem;
	replacement->pItemExt = pItemExt;
	replacement->szIconFN = iconFN;
	replacement->iCount = iCount;
	replacement->iDurability = std::max(iDurability, 0);
	replacement->pUIIcon = new CN3UIIcon();
	replacement->pUIIcon->Init(this);
	replacement->pUIIcon->SetTex(replacement->szIconFN);
	constexpr float UVAspect = 45.0f / 64.0f;
	replacement->pUIIcon->SetUVRect(0, 0, UVAspect, UVAspect);
	replacement->pUIIcon->SetUIType(UI_TYPE_ICON);
	replacement->pUIIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, iInvOrder);
	if (pArea != nullptr)
	{
		replacement->pUIIcon->SetRegion(pArea->GetRegion());
		replacement->pUIIcon->SetMoveRect(pArea->GetRegion());
	}

	if (slot != nullptr)
	{
		RemoveChild(slot->pUIIcon);
		if (slot->pUIIcon != nullptr)
		{
			slot->pUIIcon->Release();
			delete slot->pUIIcon;
		}
		delete slot;
	}
	slot = replacement;
}
