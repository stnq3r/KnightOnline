// UITradeInventory.h: interface for the CUITradeInventory class.
//
// Personal merchant (player shop) setup window.
// Uses the retail co_tradeinventory_us.uif layout:
//   - 12 sale slots  : AREA id 'at0'..'at11'
//   - 28 inv slots   : AREA id 'a0'..'a27'
//   - 12 price texts : STRING id 'st0'..'st11'
//   - 28 count texts  : STRING id '200'..'227'
//   - message edit    : EDIT id 'edit_message'
//   - buttons         : 'btn_ok' (open shop), 'btn_cancel'
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADEINVENTORY_H__A1B2C3D4_E5F6_4708_9ABC_DEF012345678__INCLUDED_)
#define AFX_UITRADEINVENTORY_H__A1B2C3D4_E5F6_4708_9ABC_DEF012345678__INCLUDED_

#pragma once

#include "GameDef.h"
#include "N3UIWndBase.h"

class CN3UIEdit;
class CN3UIString;
class CN3UIButton;
class CUIImageTooltipDlg;
class CUITradePriceDlg;

class CUITradeInventory : public CN3UIWndBase
{
public:
	enum class PendingRequest : uint8_t
	{
		None,
		ItemAdd,
		ItemCancel
	};

	__IconItemSkill* m_pShop[MAX_ITEM_PER_TRADE];  // items listed for sale
	__IconItemSkill* m_pMyInv[MAX_ITEM_INVENTORY]; // my inventory (moved out of CUIInventory)
	int m_iShopPrice[MAX_ITEM_PER_TRADE];          // price per sale slot (0 = empty)
	int m_aiShopOriginInvSlot[MAX_ITEM_PER_TRADE]; // original inventory slot on the server

	CUIImageTooltipDlg* m_pUITooltipDlg;
	CUITradePriceDlg* m_pUITradePriceDlg;

	CN3UIEdit* m_pEditMsg;
	CN3UIString* m_pStrPrice[MAX_ITEM_PER_TRADE];
	CN3UIString* m_pStrShopCount[MAX_ITEM_PER_TRADE];

	// Pending add (drag from an inventory slot onto a sale slot; awaiting count/price entry)
	int m_iPendingInvOrder;
	int m_iPendingShopOrder;
	int m_iPendingCount;
	bool m_bPendingMerge; // adding to an already-listed stackable stack (reuses its price)

	bool m_bWaitingFromServer;
	PendingRequest m_ePendingRequest;

public:
	CUITradeInventory();
	~CUITradeInventory() override;

	void Release() override;

	void Open();
	void Close(bool bCancelToServer);
	void OnMerchantClosed();

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

	// Called by the price dialog once a price has been confirmed.
	void OnPriceEntered(int iPrice);

	// Sends the pending MERCHANT_ITEM_ADD using the already-resolved count/price.
	void SendPendingAdd(int iPrice);
	void ClearPendingAdd();

	// Merge a listed sale slot back into the inventory (cancel or close).
	void ReturnShopSlotToInventory(int iShopSlot);

	// Called by the quantity dialog (stackable items) before the price is asked.
	void ItemCountOK();
	void ItemCountCancel();

	// Server callbacks
	void ReceiveItemAdd(bool bOk, int iItemID, int iCount, int iDuration, int iPrice, int iSrcInvSlot, int iDstShopSlot);
	void ReceiveItemCancel(bool bOk, int iShopSlot);
	void ReceiveItemPurchased(int iItemID, int iShopSlot, int iRemaining, int iDuration);
	void ReceiveItemCountChange(int iInvOrder, int iCount, int iItemID, int iDurability);
	bool OwnsListedInventorySlot(int iInvOrder, int iItemID) const;

	static void DumpLayout(const char* pszFile, CN3UIBase* pNode, int iDepth);
	static void DumpAllLayouts();
};

#endif // !defined(AFX_UITRADEINVENTORY_H__A1B2C3D4_E5F6_4708_9ABC_DEF012345678__INCLUDED_)
