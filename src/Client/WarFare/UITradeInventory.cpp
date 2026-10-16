// UITradeInventory.cpp: implementation of the CUITradeInventory class.
//
// Personal merchant (player shop) setup window.
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UITradeInventory.h"
#include "UITradePriceDlg.h"
#include "CountableItemEditDlg.h"
#include "GameProcMain.h"
#include "PlayerMySelf.h"
#include "UIInventory.h"
#include "UIImageTooltipDlg.h"
#include "UIManager.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "text_resources.h"

#include <shared/packets.h>

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIArea.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIEdit.h>
#include "N3UIIcon.h"

#include <fstream>
#include <cstdio>
#include <string>
#include <stdarg.h>

static FILE* MTraceFile() { static FILE* fp=nullptr; if(!fp) { fp=fopen("./merchant-trace.txt","a"); } return fp; }
static void MTrace(const char* fmt, ...) { FILE* fp=MTraceFile(); if(!fp) return; va_list ap; va_start(ap,fmt); vfprintf(fp,fmt,ap); va_end(ap); fputc('\n',fp); fflush(fp); }


// Client inventory order (0..MAX_ITEM_INVENTORY-1) maps to server slots
// SLOT_MAX..SLOT_MAX+MAX_ITEM_INVENTORY-1. This mirrors the server's
// SLOT_MAX (equipped slots) offset.
static constexpr int kSlotMax = 14;

// Items at or above this id are flagged non-tradeable by the server (ITEM_NO_TRADE).
static constexpr uint32_t kItemNoTrade = 900000001;

CUITradeInventory::CUITradeInventory()
{
	m_pUITooltipDlg   = nullptr;
	m_pUITradePriceDlg = nullptr;
	m_pEditMsg        = nullptr;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		m_pShop[i]      = nullptr;
		m_pStrPrice[i]  = nullptr;
		m_pStrShopCount[i] = nullptr;
		m_iShopPrice[i] = 0;
		m_aiShopOriginInvSlot[i] = -1;
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		m_pMyInv[i] = nullptr;

	m_iPendingInvOrder  = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount     = 0;
	m_bWaitingFromServer = false;
	m_ePendingRequest = PendingRequest::None;

	SetVisible(false);
}

CUITradeInventory::~CUITradeInventory()
{
	CUITradeInventory::Release();
}

void CUITradeInventory::Release()
{
	CN3UIBase::Release();

	delete m_pUITooltipDlg;
	m_pUITooltipDlg = nullptr;

	delete m_pUITradePriceDlg;
	m_pUITradePriceDlg = nullptr;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] != nullptr)
		{
			delete m_pShop[i];
			m_pShop[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyInv[i] != nullptr)
		{
			delete m_pMyInv[i];
			m_pMyInv[i] = nullptr;
		}
	}
}

void CUITradeInventory::InitIconWnd(e_UIWND eWnd)
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

	// The uif stores area types as NONE, so assign them here based on the
	// control's ID prefix: 'at#' = sale slot, 'a#' = my inventory slot.
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

	// Price strings for the sale slots.
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_pStrPrice[i] = GetChildByID<CN3UIString>("st" + std::to_string(i));

	// Sale-slot quantity overlays (mirrors the inventory count display).
	const std::string szFontName = fmt::format_text_resource(IDS_FONT_ID);
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pStrPrice[i] != nullptr)
			m_pStrPrice[i]->SetStyle(UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNRIGHT);

		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, i);
		if (pArea == nullptr)
			continue;

		const RECT rcArea = pArea->GetRegion();
		const RECT rcCount{ rcArea.left, rcArea.bottom - 18, rcArea.right, rcArea.bottom };

		m_pStrShopCount[i] = new CN3UIString();
		m_pStrShopCount[i]->Init(this);
		m_pStrShopCount[i]->SetRegion(rcCount);
		m_pStrShopCount[i]->SetStyle(
			UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNLEFT | UISTYLE_STRING_ALIGNBOTTOM);
		m_pStrShopCount[i]->SetFont(szFontName, 12, FALSE, FALSE);
		m_pStrShopCount[i]->SetColor(0xffffffff);
		m_pStrShopCount[i]->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pEditMsg, GetChildByID<CN3UIEdit>("edit_message"));

	// Price entry dialog.
	if (pTblUI != nullptr)
	{
		m_pUITradePriceDlg = new CUITradePriceDlg();
		m_pUITradePriceDlg->Init(CGameProcedure::s_pUIMgr);
		m_pUITradePriceDlg->LoadFromFile(pTblUI->szTradePrice);
		m_pUITradePriceDlg->SetVisible(false);
		m_pUITradePriceDlg->SetStyle(m_pUITradePriceDlg->GetStyle() | UISTYLE_ALWAYSTOP);
		RECT rcPrice = m_pUITradePriceDlg->GetRegion();
		const int iW = CN3Base::s_CameraData.vp.Width;
		const int iH = CN3Base::s_CameraData.vp.Height;
		m_pUITradePriceDlg->SetPos((iW - (rcPrice.right - rcPrice.left)) / 2,
			(iH - (rcPrice.bottom - rcPrice.top)) / 2);
	}
}

void CUITradeInventory::InitIconUpdate()
{
}

CN3UIArea* CUITradeInventory::GetChildAreaByiOrder(eUI_AREA_TYPE eUAT, int iOrder)
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

__IconItemSkill* CUITradeInventory::GetHighlightIconItem(CN3UIIcon* pUIIcon)
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

int CUITradeInventory::GetInvOrder(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
	{
		if (m_pMyInv[i] == spItem)
			return i;
	}

	return -1;
}

int CUITradeInventory::GetShopOrder(__IconItemSkill* spItem)
{
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] == spItem)
			return i;
	}

	return -1;
}

RECT CUITradeInventory::GetSampleRect()
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

void CUITradeInventory::ItemMoveFromInvToThis()
{
	MTrace("[M-T] ItemMoveFromInvToThis\n");
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

void CUITradeInventory::ItemMoveFromThisToInv()
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

void CUITradeInventory::ClearShopSlots()
{
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] != nullptr)
		{
			RemoveChild(m_pShop[i]->pUIIcon);
			m_pShop[i]->pUIIcon->Release();
			delete m_pShop[i]->pUIIcon;
			m_pShop[i]->pUIIcon = nullptr;
			delete m_pShop[i];
			m_pShop[i] = nullptr;
		}

		m_iShopPrice[i] = 0;
		m_aiShopOriginInvSlot[i] = -1;

		if (m_pStrPrice[i] != nullptr)
			m_pStrPrice[i]->SetVisible(false);
		if (m_pStrShopCount[i] != nullptr)
			m_pStrShopCount[i]->SetVisible(false);
	}
}

void CUITradeInventory::Open()
{
	MTrace("[M-T] Open entry\n");
	m_bWaitingFromServer = false;
	m_ePendingRequest = PendingRequest::None;
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		m_iShopPrice[i] = 0;

	m_iPendingInvOrder  = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount     = 0;

	ItemMoveFromInvToThis();

	if (m_pEditMsg != nullptr)
		m_pEditMsg->SetString("");

	SetVisible(true);
}

void CUITradeInventory::Close(bool bCancelToServer)
{
	MTrace("[M-T] Close entry cancel=%d\n", (int)bCancelToServer);

	if (CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWnd() == UIWND_PER_TRADE
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWndDistrict() == UIWND_DISTRICT_PER_TRADE_OTHER
		&& CN3UIWndBase::s_pCountableItemEdit->IsVisible())
		CN3UIWndBase::s_pCountableItemEdit->Close();

	if (m_bWaitingFromServer)
		return;

	// Return any unlisted items from the sale slots back to the inventory.
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] != nullptr)
			ReturnShopSlotToInventory(i);
	}

	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();

	SetState(UI_STATE_COMMON_NONE);
	AllHighLightIconFree();
	m_iPendingInvOrder = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount = 0;
	m_ePendingRequest = PendingRequest::None;

	ItemMoveFromThisToInv();

	ClearShopSlots();

	if (CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWnd() == UIWND_PER_TRADE
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWndDistrict() == UIWND_DISTRICT_PER_TRADE_OTHER
		&& CN3UIWndBase::s_pCountableItemEdit->IsVisible())
		CN3UIWndBase::s_pCountableItemEdit->Close();

	if (m_pUITradePriceDlg != nullptr)
		m_pUITradePriceDlg->Close();

	if (bCancelToServer && CGameProcedure::s_pProcMain != nullptr)
	{
		MTrace("[M-T] sending CLOSE\n");
		CGameProcedure::s_pProcMain->MsgSend_MerchantClose();
	}

	SetVisible(false);
}

void CUITradeInventory::OnMerchantClosed()
{
	// A server-side close may arrive while an old request or price dialog is
	// still pending. Invalidate it before returning the remaining listed items.
	m_bWaitingFromServer = false;
	m_ePendingRequest = PendingRequest::None;
	m_iPendingInvOrder = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount = 0;
	if (CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWnd() == UIWND_PER_TRADE
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWndDistrict() == UIWND_DISTRICT_PER_TRADE_OTHER
		&& CN3UIWndBase::s_pCountableItemEdit->IsVisible())
		CN3UIWndBase::s_pCountableItemEdit->Close();
	if (m_pUITradePriceDlg != nullptr)
		m_pUITradePriceDlg->Close();
	if (GetState() == UI_STATE_ICON_MOVING)
		IconRestore();
	SetState(UI_STATE_COMMON_NONE);
	AllHighLightIconFree();

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] != nullptr)
			ReturnShopSlotToInventory(i);
	}
}

bool CUITradeInventory::OwnsListedInventorySlot(int iInvOrder, int iItemID) const
{
	if (iInvOrder < 0 || iInvOrder >= MAX_ITEM_INVENTORY || iItemID <= 0)
		return false;

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		const __IconItemSkill* item = m_pShop[i];
		if (item != nullptr && m_aiShopOriginInvSlot[i] == iInvOrder && item->GetItemID() == iItemID)
			return true;
	}

	return false;
}

void CUITradeInventory::Render()
{
	MTrace("[M-T] Render\n");
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
			&& reinterpret_cast<CN3UIIcon*>(pChild) == CN3UIWndBase::s_sSelectedIconInfo.pItemSelect->pUIIcon)
			continue;

		pChild->Render();

		if (GetState() == UI_STATE_COMMON_NONE && pChild->UIType() == UI_TYPE_ICON
			&& (pChild->GetStyle() & UISTYLE_ICON_HIGHLIGHT))
		{
			bTooltipRender = true;
			spItem         = GetHighlightIconItem(reinterpret_cast<CN3UIIcon*>(pChild));
		}
	}

	// Inventory item counts.
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

	// Sale-slot item counts (stackable items only).
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		CN3UIString* pStr = m_pStrShopCount[i];
		if (pStr == nullptr)
			continue;

		if (m_pShop[i] != nullptr && m_pShop[i]->IsStackable())
		{
			pStr->SetVisible(true);
			pStr->SetStringAsInt(m_pShop[i]->iCount);
			pStr->Render();
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
		// Hovering a listed sale slot shows its asking price in the tooltip.
		const int iShopOrder = GetShopOrder(spItem);
		if (iShopOrder != -1 && m_iShopPrice[iShopOrder] > 0)
		{
			m_pUITooltipDlg->SetCustomPrice(m_iShopPrice[iShopOrder]);
			m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem, true, false);
		}
		else
		{
			m_pUITooltipDlg->SetCustomPrice(-1);
			m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, spItem);
		}
	}
}

uint32_t CUITradeInventory::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!IsVisible() || m_bWaitingFromServer)
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

bool CUITradeInventory::ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur)
{
	MTrace("[M-T] ReceiveIconDrop spItem=%p\n", (void*)spItem);
	if (!m_bVisible)
		return false;

	int iShopOrder = -1;
	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, i);
		if (pArea != nullptr && pArea->IsIn(ptCur.x, ptCur.y))
		{
			iShopOrder = i;
			break;
		}
	}

	if (iShopOrder == -1)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	// Source must be one of our own inventory slots.
	int iSrcOrder = GetInvOrder(spItem);
	if (iSrcOrder == -1 || spItem == nullptr)
	{
		AllHighLightIconFree();
		SetState(UI_STATE_COMMON_NONE);
		return false;
	}

	// Non-tradeable items may not be listed on the shop.
	if (spItem->pItemBasic != nullptr)
	{
		const uint32_t dwSrcID = static_cast<uint32_t>(spItem->pItemBasic->dwID)
			+ (spItem->pItemExt != nullptr ? static_cast<uint32_t>(spItem->pItemExt->dwID) : 0);
		if (dwSrcID >= kItemNoTrade)
		{
			CGameProcedure::MessageBoxPost("This item cannot be traded.", "Personal Shop", MB_OK);
			AllHighLightIconFree();
			SetState(UI_STATE_COMMON_NONE);
			return false;
		}
	}

	// Restore the dragged icon; we'll move it for real once count/price is entered.
	IconRestore();

	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);

	__IconItemSkill* spSrcItem = m_pMyInv[iSrcOrder];

	// A stackable item may only occupy a single sale slot: if this item is
	// already listed, redirect the add to that slot and merge into its stack.
	int iDstOrder = iShopOrder;
	bool bMerge = false;
	if (spSrcItem != nullptr && spSrcItem->IsStackable())
	{
		const int iItemID = spSrcItem->GetItemID();
		for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
		{
			if (m_pShop[i] != nullptr && m_pShop[i]->GetItemID() == iItemID)
			{
				iDstOrder = i;
				bMerge = true;
				break;
			}
		}
	}

	m_iPendingInvOrder  = iSrcOrder;
	m_iPendingShopOrder = iDstOrder;
	m_iPendingCount     = 0;
	m_bPendingMerge     = bMerge;

	int iMaxCount = 1;
	const bool bCountable = spSrcItem != nullptr && spSrcItem->pItemBasic != nullptr
		&& (spSrcItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE
			|| spSrcItem->pItemBasic->byContable == UIITEM_TYPE_COUNTABLE_SMALL);
	if (bCountable)
	{
		iMaxCount = spSrcItem->iCount;
		if (iMaxCount <= 0)
			iMaxCount = 1;
	}

	// Retail order: stackable items ask for a quantity first, then a price.
	if (bCountable && CN3UIWndBase::s_pCountableItemEdit != nullptr)
	{
		if (CN3UIWndBase::s_pCountableItemEdit->IsVisible())
		{
			ClearPendingAdd();
			return false;
		}

		m_iPendingCount = iMaxCount;
		CN3UIWndBase::s_pCountableItemEdit->Open(
			UIWND_PER_TRADE, UIWND_DISTRICT_PER_TRADE_OTHER, false);
		CN3UIWndBase::s_pCountableItemEdit->SetQuantity(iMaxCount);
		return true;
	}

	m_iPendingCount = iMaxCount;

	// Merging into an existing listing reuses its price; no price prompt.
	if (bMerge)
	{
		SendPendingAdd(m_iShopPrice[iDstOrder]);
		return true;
	}

	// Ask for a price.
	if (m_pUITradePriceDlg != nullptr)
	{
		m_pUITradePriceDlg->Open([this](int iPrice) { this->OnPriceEntered(iPrice); }, [this]()
		{
			ClearPendingAdd();
		});
	}

	return true;
}

void CUITradeInventory::ItemCountOK()
{
	CCountableItemEditDlg* pCountDlg = CN3UIWndBase::s_pCountableItemEdit;
	if (pCountDlg == nullptr || !pCountDlg->IsVisible()
		|| pCountDlg->GetCallerWnd() != UIWND_PER_TRADE
		|| pCountDlg->GetCallerWndDistrict() != UIWND_DISTRICT_PER_TRADE_OTHER)
		return;

	const int iCount = pCountDlg->GetQuantity();
	pCountDlg->Close();

	__IconItemSkill* spItem = (m_iPendingInvOrder >= 0 && m_iPendingInvOrder < MAX_ITEM_INVENTORY)
		? m_pMyInv[m_iPendingInvOrder]
		: nullptr;
	if (!IsVisible() || spItem == nullptr || iCount <= 0 || iCount > spItem->iCount)
	{
		ClearPendingAdd();
		return;
	}

	m_iPendingCount = iCount;

	// Merge into an existing stack: reuses the listed price.
	if (m_bPendingMerge)
	{
		SendPendingAdd(m_iShopPrice[m_iPendingShopOrder]);
		return;
	}

	// Quantity chosen; now ask for the price.
	if (m_pUITradePriceDlg != nullptr)
	{
		m_pUITradePriceDlg->Open([this](int iPrice) { this->OnPriceEntered(iPrice); }, [this]()
		{
			ClearPendingAdd();
		});
	}
}

void CUITradeInventory::ItemCountCancel()
{
	m_iPendingInvOrder  = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount     = 0;

	if (CN3UIWndBase::s_pCountableItemEdit != nullptr
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWnd() == UIWND_PER_TRADE
		&& CN3UIWndBase::s_pCountableItemEdit->GetCallerWndDistrict() == UIWND_DISTRICT_PER_TRADE_OTHER
		&& CN3UIWndBase::s_pCountableItemEdit->IsVisible())
		CN3UIWndBase::s_pCountableItemEdit->Close();
}

void CUITradeInventory::OnPriceEntered(int iPrice)
{
	MTrace("[M-T] OnPriceEntered price=%d pendInv=%d pendShop=%d\n", iPrice, m_iPendingInvOrder, m_iPendingShopOrder);
	if (iPrice <= 0 || m_iPendingInvOrder < 0 || m_iPendingShopOrder < 0)
	{
		ClearPendingAdd();
		m_bWaitingFromServer = false;
		m_ePendingRequest = PendingRequest::None;
		return;
	}

	const int iSrcOrder = m_iPendingInvOrder;
	const int iDstOrder = m_iPendingShopOrder;

	__IconItemSkill* spItem = m_pMyInv[iSrcOrder];
	if (spItem == nullptr || spItem->pItemBasic == nullptr || spItem->pItemExt == nullptr
		|| iDstOrder >= MAX_ITEM_PER_TRADE)
	{
		ClearPendingAdd();
		m_ePendingRequest = PendingRequest::None;
		return;
	}

	if (!m_bPendingMerge && m_pShop[iDstOrder] != nullptr)
	{
		ClearPendingAdd();
		m_ePendingRequest = PendingRequest::None;
		return;
	}

	SendPendingAdd(iPrice);
}

void CUITradeInventory::IconRestore()
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

void CUITradeInventory::CancelIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

void CUITradeInventory::AcceptIconDrop(__IconItemSkill* /*spItem*/)
{
	AllHighLightIconFree();
	SetState(UI_STATE_COMMON_NONE);
}

bool CUITradeInventory::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;
	if (m_bWaitingFromServer)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender->GetID() == "btn_ok")
		{
			std::string szMsg;
			if (m_pEditMsg != nullptr)
				szMsg = m_pEditMsg->GetString();

			if (CGameProcedure::s_pProcMain != nullptr)
			{
				MTrace("[M-T] sending INSERT msgLen=%zu\n", szMsg.size());
				CGameProcedure::s_pProcMain->MsgSend_MerchantInsert(szMsg);
			}

			SetVisible(false);
			ItemMoveFromThisToInv();
			return true;
		}

		if (pSender->GetID() == "btn_cancel")
		{
			Close(true);
			return true;
		}
	}

	// Drag & drop handling for icons.
	switch (dwMsg & 0x000f0000)
	{
		case UIMSG_ICON_DOWN_FIRST:
		{
			AllHighLightIconFree();

			__IconItemSkill* spItem = GetHighlightIconItem(reinterpret_cast<CN3UIIcon*>(pSender));
			if (spItem == nullptr)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			int iOrder = GetInvOrder(spItem);
			e_UIWND_DISTRICT eDistrict = UIWND_DISTRICT_PER_TRADE_INV;
			if (iOrder == -1)
			{
				iOrder    = GetShopOrder(spItem);
				eDistrict = UIWND_DISTRICT_PER_TRADE_MY;
			}

			if (iOrder == -1)
			{
				SetState(UI_STATE_COMMON_NONE);
				return false;
			}

			s_sSelectedIconInfo.UIWndSelect.UIWnd         = UIWND_PER_TRADE;
			s_sSelectedIconInfo.UIWndSelect.UIWndDistrict = eDistrict;
			s_sSelectedIconInfo.UIWndSelect.iOrder        = iOrder;
			s_sSelectedIconInfo.pItemSelect               = spItem;

			reinterpret_cast<CN3UIIcon*>(pSender)->SetRegion(GetSampleRect());
			reinterpret_cast<CN3UIIcon*>(pSender)->SetMoveRect(GetSampleRect());

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
				// If dragging a sale slot back onto the inventory, unlist it.
				POINT ptCur = CGameProcedure::s_pLocalInput->MouseGetPos();
				int iShopOrder = GetShopOrder(s_sSelectedIconInfo.pItemSelect);
				bool bReturnedToInv = false;

				if (iShopOrder != -1)
				{
					for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
					{
						CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, i);
						if (pArea != nullptr && pArea->IsIn(ptCur.x, ptCur.y) && m_pMyInv[i] == nullptr)
						{
							m_iPendingInvOrder = i;
							m_iPendingShopOrder = iShopOrder;
							m_bWaitingFromServer = true;
							m_ePendingRequest = PendingRequest::ItemCancel;
							IconRestore();
							AllHighLightIconFree();
							SetState(UI_STATE_COMMON_NONE);

							if (CGameProcedure::s_pProcMain != nullptr)
								CGameProcedure::s_pProcMain->MsgSend_MerchantItemCancel(iShopOrder);
							else
								ReceiveItemCancel(false, iShopOrder);
							bReturnedToInv = true;
							break;
						}
					}
				}

				if (!bReturnedToInv)
				{
					if (!CGameProcedure::s_pUIMgr->BroadcastIconDropMsg(s_sSelectedIconInfo.pItemSelect))
						IconRestore();
				}

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

void CUITradeInventory::ReceiveItemAdd(bool bOk, int iItemID, int iCount, int iDuration, int iPrice, int iSrcInvSlot, int iDstShopSlot)
{
	if (!m_bWaitingFromServer || m_ePendingRequest != PendingRequest::ItemAdd
		|| iSrcInvSlot != m_iPendingInvOrder || iDstShopSlot != m_iPendingShopOrder
		|| iSrcInvSlot < 0 || iSrcInvSlot >= MAX_ITEM_INVENTORY
		|| iDstShopSlot < 0 || iDstShopSlot >= MAX_ITEM_PER_TRADE)
		return;

	const auto clearPending = [this]()
	{
		m_bWaitingFromServer = false;
		m_ePendingRequest = PendingRequest::None;
		m_iPendingInvOrder = -1;
		m_iPendingShopOrder = -1;
		m_iPendingCount = 0;
	};

	// A rejected add leaves the inventory exactly as it was.
	if (!bOk)
	{
		clearPending();
		return;
	}

	__IconItemSkill* spSrc = m_pMyInv[iSrcInvSlot];
	if (spSrc == nullptr || spSrc->pItemBasic == nullptr || spSrc->pItemExt == nullptr
		|| static_cast<uint32_t>(spSrc->pItemBasic->dwID + spSrc->pItemExt->dwID)
			!= static_cast<uint32_t>(iItemID)
		|| iCount <= 0 || iPrice <= 0 || iDuration < 0 || iCount > spSrc->iCount
		|| m_pShop[iDstShopSlot] != nullptr)
	{
		clearPending();
		return;
	}

	CN3UIArea* pArea = GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_OTHER, iDstShopSlot);

	if (iCount >= spSrc->iCount)
	{
		// Whole stack: move the icon from the inventory into the sale slot.
		m_pMyInv[iSrcInvSlot] = nullptr;
		m_pShop[iDstShopSlot] = spSrc;
		if (pArea != nullptr)
		{
			spSrc->pUIIcon->SetRegion(pArea->GetRegion());
			spSrc->pUIIcon->SetMoveRect(pArea->GetRegion());
		}
	}
	else
	{
		// Partial stack: clone an icon for the sale slot and reduce the source
		// stack that stays in the inventory.
		__IconItemSkill* spShop = spSrc->Clone(this);
		spShop->iCount = iCount;
		spShop->iDurability = iDuration;
		if (pArea != nullptr)
		{
			spShop->pUIIcon->SetRegion(pArea->GetRegion());
			spShop->pUIIcon->SetMoveRect(pArea->GetRegion());
		}
		spShop->pUIIcon->SetVisible(true);
		m_pShop[iDstShopSlot] = spShop;
		spSrc->iCount -= iCount;
	}

	m_aiShopOriginInvSlot[iDstShopSlot] = iSrcInvSlot;
	m_iShopPrice[iDstShopSlot] = iPrice;
	if (m_pStrPrice[iDstShopSlot] != nullptr)
	{
		m_pStrPrice[iDstShopSlot]->SetVisible(true);
		m_pStrPrice[iDstShopSlot]->SetStringAsInt(iPrice);
	}

	clearPending();
}

void CUITradeInventory::ReturnShopSlotToInventory(int iShopSlot)
{
	if (iShopSlot < 0 || iShopSlot >= MAX_ITEM_PER_TRADE)
		return;

	__IconItemSkill* spShop = m_pShop[iShopSlot];
	if (spShop == nullptr)
		return;

	// When the setup window is visible the icons live in m_pMyInv; once the shop
	// is live the window is hidden and the inventory is back in CUIInventory.
	CUIInventory* pInven = CGameProcedure::s_pProcMain != nullptr
		? CGameProcedure::s_pProcMain->m_pUIInventory
		: nullptr;
	const bool bSelfVisible = IsVisible();
	__IconItemSkill** ppInv = bSelfVisible
		? m_pMyInv
		: (pInven != nullptr ? pInven->m_pMyInvWnd : nullptr);
	if (ppInv == nullptr)
		return;

	const int iOrigin = m_aiShopOriginInvSlot[iShopSlot];
	const auto itemMatches = [spShop](const __IconItemSkill* p) -> bool
	{
		return p != nullptr && p->IsStackable() && p->GetItemID() == spShop->GetItemID();
	};

	int destination = -1;
	if (iOrigin >= 0 && iOrigin < MAX_ITEM_INVENTORY
		&& (ppInv[iOrigin] == nullptr || itemMatches(ppInv[iOrigin])))
	{
		destination = iOrigin;
	}
	if (destination == -1)
	{
		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			if (itemMatches(ppInv[i]))
			{
				destination = i;
				break;
			}
		}
	}
	if (destination == -1)
	{
		for (int i = 0; i < MAX_ITEM_INVENTORY; i++)
		{
			if (ppInv[i] == nullptr)
			{
				destination = i;
				break;
			}
		}
	}

	if (destination == -1)
		return; // No room; keep it listed.

	__IconItemSkill* pCur = ppInv[destination];
	if (pCur != nullptr)
	{
		pCur->iCount += spShop->iCount;
		RemoveChild(spShop->pUIIcon);
		if (spShop->pUIIcon != nullptr)
		{
			spShop->pUIIcon->Release();
			delete spShop->pUIIcon;
			spShop->pUIIcon = nullptr;
		}
		delete spShop;
	}
	else
	{
		ppInv[destination] = spShop;
		spShop->pUIIcon->SetParent(bSelfVisible
			? static_cast<CN3UIBase*>(this)
			: static_cast<CN3UIBase*>(pInven));
		CN3UIArea* pArea = bSelfVisible
			? GetChildAreaByiOrder(UI_AREA_TYPE_PER_TRADE_INV, destination)
			: (pInven != nullptr ? pInven->GetChildAreaByiOrder(UI_AREA_TYPE_INV, destination) : nullptr);
		if (pArea != nullptr)
		{
			spShop->pUIIcon->SetRegion(pArea->GetRegion());
			spShop->pUIIcon->SetMoveRect(pArea->GetRegion());
		}
	}

	m_pShop[iShopSlot] = nullptr;
	m_iShopPrice[iShopSlot] = 0;
	m_aiShopOriginInvSlot[iShopSlot] = -1;
	if (m_pStrPrice[iShopSlot] != nullptr)
		m_pStrPrice[iShopSlot]->SetVisible(false);
	if (m_pStrShopCount[iShopSlot] != nullptr)
		m_pStrShopCount[iShopSlot]->SetVisible(false);
}

void CUITradeInventory::ReceiveItemCancel(bool bOk, int iShopSlot)
{
	if (!m_bWaitingFromServer || m_ePendingRequest != PendingRequest::ItemCancel
		|| m_iPendingShopOrder < 0 || m_iPendingShopOrder >= MAX_ITEM_PER_TRADE)
		return;

	const int shopSlot = m_iPendingShopOrder;
	if (bOk && iShopSlot == shopSlot)
		ReturnShopSlotToInventory(shopSlot);

	m_bWaitingFromServer = false;
	m_ePendingRequest = PendingRequest::None;
	m_iPendingInvOrder = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount = 0;
}

void CUITradeInventory::ReceiveItemPurchased(int iItemID, int iShopSlot, int iRemaining, int iDuration)
{
	if (iShopSlot < 0 || iShopSlot >= MAX_ITEM_PER_TRADE)
		return;

	__IconItemSkill* spShop = m_pShop[iShopSlot];
	if (spShop == nullptr || spShop->GetItemID() != iItemID)
		return;

	if (iRemaining > 0)
	{
		spShop->iCount = iRemaining;
		return;
	}

	RemoveChild(spShop->pUIIcon);
	if (spShop->pUIIcon != nullptr)
	{
		spShop->pUIIcon->Release();
		delete spShop->pUIIcon;
		spShop->pUIIcon = nullptr;
	}
	delete spShop;
	m_pShop[iShopSlot] = nullptr;
	m_iShopPrice[iShopSlot] = 0;
	m_aiShopOriginInvSlot[iShopSlot] = -1;
	if (m_pStrPrice[iShopSlot] != nullptr)
		m_pStrPrice[iShopSlot]->SetVisible(false);
	if (m_pStrShopCount[iShopSlot] != nullptr)
		m_pStrShopCount[iShopSlot]->SetVisible(false);
	(void) iDuration;
}

void CUITradeInventory::ReceiveItemCountChange(int iInvOrder, int iCount, int iItemID, int iDurability)
{
	if (iInvOrder < 0 || iInvOrder >= MAX_ITEM_INVENTORY || iItemID <= 0 || iCount < 0 || iDurability < -1)
		return;

	__IconItemSkill* inventoryItem = m_pMyInv[iInvOrder];
	if (inventoryItem != nullptr && inventoryItem->GetItemID() == iItemID)
	{
		if (iCount > 0)
		{
			inventoryItem->iCount = iCount;
			if (iDurability >= 0)
				inventoryItem->iDurability = iDurability;
		}
		else
		{
			RemoveChild(inventoryItem->pUIIcon);
			if (inventoryItem->pUIIcon != nullptr)
			{
				inventoryItem->pUIIcon->Release();
				delete inventoryItem->pUIIcon;
			}
			delete inventoryItem;
			m_pMyInv[iInvOrder] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ITEM_PER_TRADE; i++)
	{
		if (m_pShop[i] == nullptr || m_aiShopOriginInvSlot[i] != iInvOrder
			|| m_pShop[i]->GetItemID() != iItemID)
			continue;

		if (iCount > 0)
		{
			m_pShop[i]->iCount = iCount;
			if (iDurability >= 0)
				m_pShop[i]->iDurability = iDurability;
			continue;
		}

		RemoveChild(m_pShop[i]->pUIIcon);
		if (m_pShop[i]->pUIIcon != nullptr)
		{
			m_pShop[i]->pUIIcon->Release();
			delete m_pShop[i]->pUIIcon;
			m_pShop[i]->pUIIcon = nullptr;
		}
		delete m_pShop[i];
		m_pShop[i] = nullptr;
		m_aiShopOriginInvSlot[i] = -1;
		m_iShopPrice[i] = 0;
		if (m_pStrPrice[i] != nullptr)
			m_pStrPrice[i]->SetVisible(false);
	}
}

// ---------------------------------------------------------------------------
// Layout dump helpers (development aid)
// ---------------------------------------------------------------------------

void CUITradeInventory::DumpAllLayouts()
{
	const char* pszFiles[] = {
		"UI_us\\co_tradeinventory_us.uif",
		"UI_us\\co_TradeBuyInventory_us.uif",
		"UI_us\\co_TradePrice_us.uif",
		"UI_us\\co_tradeMessage_us.uif",
		"UI_us\\co_TradeMessageBox_us.uif",
		"UI_us\\co_tradecount_us.uif",
	};
	const char* pszOuts[] = {
		"tradeinv_layout.txt",
		"tradebuy_layout.txt",
		"tradeprice_layout.txt",
		"trademsg_layout.txt",
		"trademsgbox_layout.txt",
		"tradecount_layout.txt",
	};

	for (int i = 0; i < 6; i++)
	{
		CUITradeInventory* pDump = new CUITradeInventory();
		pDump->Init(nullptr);
		if (pDump->LoadFromFile(pszFiles[i]))
			CUITradeInventory::DumpLayout(pszOuts[i], pDump, 0);
		else
		{
			std::ofstream ofs(pszOuts[i], std::ios::trunc);
			ofs << "LOAD FAILED: " << pszFiles[i] << "\n";
		}
		pDump->Release();
		delete pDump;
	}
}

void CUITradeInventory::DumpLayout(const char* pszFile, CN3UIBase* pNode, int iDepth)
{
	std::ofstream ofs(pszFile, iDepth == 0 ? std::ios::trunc : std::ios::app);
	if (!ofs.is_open())
		return;

	const int iCount = pNode->GetChildrenCount();
	for (int i = 0; i < iCount; i++)
	{
		CN3UIBase* pChild = pNode->GetChildByIndex(i);
		if (pChild == nullptr)
			continue;

		const std::string indent(iDepth * 2, ' ');
		const RECT rc = pChild->GetRegion();

		switch (pChild->UIType())
		{
			case UI_TYPE_AREA:
			{
				CN3UIArea* pArea = static_cast<CN3UIArea*>(pChild);
				ofs << indent << "AREA   id='" << pChild->GetID() << "' type=" << static_cast<int>(pArea->m_eAreaType) << " region=[" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "]\n";
			}
			break;
			case UI_TYPE_BUTTON:
				ofs << indent << "BUTTON id='" << pChild->GetID() << "' region=[" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "]\n";
				break;
			case UI_TYPE_STRING:
				ofs << indent << "STRING id='" << pChild->GetID() << "' region=[" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "]\n";
				break;
			case UI_TYPE_EDIT:
				ofs << indent << "EDIT   id='" << pChild->GetID() << "' region=[" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "]\n";
				break;
			case UI_TYPE_IMAGE:
				ofs << indent << "IMAGE  id='" << pChild->GetID() << "' region=[" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "]\n";
				break;
			default:
				ofs << indent << "TYPE=" << static_cast<int>(pChild->UIType()) << " id='" << pChild->GetID() << "' region=[" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "]\n";
				break;
		}

		DumpLayout(pszFile, pChild, iDepth + 1);
	}
}

void CUITradeInventory::SendPendingAdd(int iPrice)
{
	if (m_iPendingInvOrder < 0 || m_iPendingShopOrder < 0)
	{
		ClearPendingAdd();
		m_bWaitingFromServer = false;
		m_ePendingRequest = PendingRequest::None;
		return;
	}

	__IconItemSkill* spItem = m_pMyInv[m_iPendingInvOrder];
	if (spItem == nullptr || spItem->pItemBasic == nullptr || spItem->pItemExt == nullptr)
	{
		ClearPendingAdd();
		m_bWaitingFromServer = false;
		m_ePendingRequest = PendingRequest::None;
		return;
	}

	int iCount = m_iPendingCount;
	if (iCount <= 0 || iCount > spItem->iCount)
		iCount = spItem->iCount;
	if (iCount <= 0)
		iCount = 1;

	m_iPendingCount = iCount;

	int iItemID = spItem->pItemBasic->dwID + spItem->pItemExt->dwID;
	const int iSrcOrder = m_iPendingInvOrder;
	const int iDstOrder = m_iPendingShopOrder;

	m_bWaitingFromServer = true;
	m_ePendingRequest = PendingRequest::ItemAdd;

	if (CGameProcedure::s_pProcMain != nullptr)
	{
		MTrace("[M-T] sending ITEM_ADD\n");
		CGameProcedure::s_pProcMain->MsgSend_MerchantItemAdd(iItemID, iCount, iPrice, iSrcOrder, iDstOrder, 0);
	}
	else
	{
		ReceiveItemAdd(false, iItemID, iCount, spItem->iDurability, iPrice, iSrcOrder, iDstOrder);
	}

	PlayItemSound(spItem->pItemBasic);
}

void CUITradeInventory::ClearPendingAdd()
{
	m_iPendingInvOrder = -1;
	m_iPendingShopOrder = -1;
	m_iPendingCount = 0;
	m_bPendingMerge = false;
}
