// UITradeBuyInventory.h: interface for the CUITradeBuyInventory class.
//
// Personal-merchant (player shop) buyer window.
// Layout: co_TradeBuyInventory_us.uif
//   - 12 shop slots   : AREA id 'at0'..'at11'
//   - 28 inv slots    : AREA id 'a0'..'a27'
//   - 12 price texts  : STRING id 'st0'..'st11'
//   - 28 count texts  : STRING id '200'..'227'
//   - money readout   : STRING id 'text_money'
//   - confirm button  : 'btn_ok'
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEBUYINVENTORY_H__C3D4E5F6_A7B8_4920_9CDE_F0123456789A__INCLUDED_)
#define AFX_UITRADEBUYINVENTORY_H__C3D4E5F6_A7B8_4920_9CDE_F0123456789A__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

class CN3UIString;
class CUIImageTooltipDlg;

class CUITradeBuyInventory : public CN3UIWndBase
{
public:
	__IconItemSkill* m_pShop[MAX_ITEM_PER_TRADE];   // display slots (read-only)
	__IconItemSkill* m_pMyInv[MAX_ITEM_INVENTORY];  // my inventory
	int m_iShopPrice[MAX_ITEM_PER_TRADE];            // price per shop slot (0 = empty)

	CN3UIString* m_pStrPrice[MAX_ITEM_PER_TRADE];
	CN3UIString* m_pStrMoney;
	CUIImageTooltipDlg* m_pUITooltipDlg;

	int m_iMerchantUID; // the merchant whose shop is currently shown
	int m_iPendingShopOrder;
	int m_iPendingInvOrder;
	int m_iPendingPurchaseCount = 0;
	bool m_bAwaitingPurchaseConfirmation = false;

public:
	CUITradeBuyInventory();
	~CUITradeBuyInventory() override;

	void Release() override;

	void Open(int iMerchantUID);
	void Close(bool bCancelToServer = true);

	void ItemMoveFromInvToThis();
	void ItemMoveFromThisToInv();
	void ClearShopSlots();

	void Render() override;
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void InitIconWnd(e_UIWND eWnd) override;
	void InitIconUpdate() override;

	__IconItemSkill* GetHighlightIconItem(CN3UIIcon* pUIIcon) override;
	void IconRestore() override;

	bool ReceiveIconDrop(__IconItemSkill* spItem, POINT ptCur) override;
	void CancelIconDrop(__IconItemSkill* spItem) override;
	void AcceptIconDrop(__IconItemSkill* spItem) override;

	CN3UIArea* GetChildAreaByiOrder(eUI_AREA_TYPE eUAT, int iOrder) override;

	int GetInvOrder(__IconItemSkill* spItem);
	int GetShopOrder(__IconItemSkill* spItem);
	RECT GetSampleRect();

	// Server callbacks
	void ReceiveItemList(const int* piItemIDs, const int* piCounts, const int* piDurabilities, const int* piPrices);
	void ReceiveItemPurchaseUpdate(int iItemID, int iRemainingCount, int iShopSlot);
	void ReceiveItemCountChange(int iInvOrder, int iCount, int iItemID, int iDurability);
	void ItemCountOK();
	void ItemCountCancel();
	void BeginPurchaseConfirmation(int shopOrder, int invOrder, int count);
	void ResolvePurchaseConfirmation(bool accepted);

	void UpdateMoneyDisplay();
};

#endif // !defined(AFX_UITRADEBUYINVENTORY_H__C3D4E5F6_A7B8_4920_9CDE_F0123456789A__INCLUDED_)
