#include "pch.h"
#include "EbenezerApp.h"
#include "User.h"

#include <shared/globals.h>
#include <shared/packets.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <limits>

namespace Ebenezer
{

// Personal merchant (player shop) system.
// Ported from the original 1.081 GameServer implementation (MerchantHandler.cpp).
// The "buying merchant" half of the original system is intentionally not restored.

enum MerchantOpenResponseCodes
{
	MERCHANT_OPEN_SUCCESS      = 1,
	MERCHANT_OPEN_NO_SESSION   = -1,
	MERCHANT_OPEN_DEAD         = -2,
	MERCHANT_OPEN_TRADING      = -3,
	MERCHANT_OPEN_MERCHANTING  = -4,
	MERCHANT_OPEN_INVALID_ZONE = -5,
	MERCHANT_OPEN_SHOPPING     = -6,
	MERCHANT_OPEN_UNDERLEVELED = 30
};

static bool IsMerchantZone(uint8_t zone)
{
	// Personal shops may only be opened in the starting towns / Moradon.
	return zone == ZONE_KARUS || zone == ZONE_ELMORAD || zone == ZONE_MORADON;
}

static void SendMerchantClose(CUser* pReceiver, int16_t ownerSocketID)
{
	if (pReceiver == nullptr)
		return;

	int sendIndex = 0;
	char sendBuffer[8] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_CLOSE, sendIndex);
	SetShort(sendBuffer, ownerSocketID, sendIndex);
	pReceiver->Send(sendBuffer, sendIndex);
}

static void SendMerchantItemListPacket(CUser* pReceiver, int16_t result, int16_t ownerSocketID,
	const _MERCH_DATA* items)
{
	if (pReceiver == nullptr)
		return;

	int sendIndex = 0;
	char sendBuffer[256] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_ITEM_LIST, sendIndex);
	SetShort(sendBuffer, result, sendIndex);
	SetShort(sendBuffer, ownerSocketID, sendIndex);
	for (int i = 0; i < MAX_MERCH_ITEMS; i++)
	{
		const _MERCH_DATA* item = result == 1 && items != nullptr ? &items[i] : nullptr;
		SetDWORD(sendBuffer, item != nullptr ? item->nNum : 0, sendIndex);
		SetShort(sendBuffer, item != nullptr ? item->sCount : 0, sendIndex);
		SetShort(sendBuffer, item != nullptr ? item->sDuration : 0, sendIndex);
		SetDWORD(sendBuffer, item != nullptr ? item->nPrice : 0, sendIndex);
		SetDWORD(sendBuffer, 0, sendIndex);
	}
	pReceiver->Send(sendBuffer, sendIndex);
}

void CUser::MerchantProcess(char* pBuf, int len)
{
	if (pBuf == nullptr || len <= 0)
		return;

	int index     = 0;
	uint8_t opcode = GetByte(pBuf, index);
	spdlog::info("User::MerchantProcess: opcode={:02X} [accountName={} characterName={} socketId={}]",
		opcode, m_strAccountID, m_pUserData->m_id, _socketId);
	switch (opcode)
	{
		case MERCHANT_OPEN:
			if (len == 1)
			MerchantOpen();
			break;

		case MERCHANT_CLOSE:
			if (len == 1)
			MerchantClose();
			break;

		case MERCHANT_ITEM_ADD:
			if (len == 1 + sizeof(uint32_t) + sizeof(int16_t) + sizeof(uint32_t) + sizeof(uint8_t) * 3)
				MerchantItemAdd(pBuf + index);
			break;

		case MERCHANT_ITEM_CANCEL:
			if (len == 1 + sizeof(uint8_t))
				MerchantItemCancel(pBuf + index);
			break;

		case MERCHANT_ITEM_LIST:
			if (len == 1 + sizeof(int16_t))
				MerchantItemList(pBuf + index);
			break;

		case MERCHANT_ITEM_BUY:
			if (len == 1 + sizeof(uint32_t) + sizeof(int16_t) + sizeof(uint8_t) * 3)
				MerchantItemBuy(pBuf + index);
			break;

		case MERCHANT_INSERT:
			if (len >= static_cast<int>(1 + sizeof(uint16_t)))
			{
				const uint16_t msgLen = static_cast<uint16_t>(GetShort(pBuf, index));
				if (msgLen <= MAX_MERCH_MESSAGE && len == index + msgLen)
					MerchantInsert(pBuf + 1);
			}
			break;

		case MERCHANT_TRADE_CANCEL:
			if (len == 1)
				CancelMerchant();
			break;

		default:
			spdlog::error("User::MerchantProcess: Unhandled opcode {:02X} [accountName={} characterName={}]",
				opcode, m_strAccountID, m_pUserData->m_id);
			break;
	}
}

void CUser::MerchantOpen()
{
	int16_t errorCode = 0;
	if (m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0)
		errorCode = MERCHANT_OPEN_DEAD;
	else if (m_sMerchantsSocketID >= 0)
		errorCode = MERCHANT_OPEN_SHOPPING;
	else if (m_sExchangeUser != -1)
		errorCode = MERCHANT_OPEN_TRADING;
	else if (!IsMerchantZone(m_pUserData->m_bZone))
		errorCode = MERCHANT_OPEN_INVALID_ZONE;
	else if (isMerchanting())
		errorCode = MERCHANT_OPEN_MERCHANTING;
	else
		errorCode = MERCHANT_OPEN_SUCCESS;

	int sendIndex = 0;
	char sendBuffer[16] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_OPEN, sendIndex);
	SetShort(sendBuffer, errorCode, sendIndex);
	Send(sendBuffer, sendIndex);

	// If we're already merchanting, the user may be desynced, so close the
	// current merchant first.
	if (errorCode == MERCHANT_OPEN_MERCHANTING)
		MerchantClose();
}

void CUser::MerchantClose()
{
	GiveMerchantItems();

	m_bMerchantState = MERCHANT_STATE_NONE;
	m_strMerchantMessage.clear();

	// Let everyone in our region know the shop closed.
	int sendIndex = 0;
	char sendBuffer[16] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_CLOSE, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	m_pMain->Send_Region(sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, this, false);
	// MerchantInsert is sent to the owner as well as nearby players, so send
	// the matching close notification to the owner to clear its local state.
	Send(sendBuffer, sendIndex);

	// Tell anyone currently browsing that the shop is gone.
	for (int16_t lookerId : m_arMerchantLookers)
	{
		auto pLooker = m_pMain->GetUserPtr(lookerId);
		if (pLooker == nullptr)
			continue;

		SendMerchantClose(pLooker.get(), static_cast<int16_t>(_socketId));
		pLooker->m_sMerchantsSocketID = -1;
	}
	m_arMerchantLookers.clear();
}

void CUser::MerchantItemAdd(char* pBuf)
{
	int index = 0, sendIndex = 0;
	uint32_t nItemID = 0, nGold = 0;
	int16_t sCount = 0;
	uint8_t bSrcPos = 0, bDstPos = 0;

	nItemID = GetDWORD(pBuf, index);
	sCount  = static_cast<int16_t>(GetShort(pBuf, index));
	nGold   = GetDWORD(pBuf, index);
	bSrcPos = GetByte(pBuf, index);
	bDstPos = GetByte(pBuf, index);
	GetByte(pBuf, index); // bMode: normal/premium flag (unused)

	char sendBuffer[64] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_ITEM_ADD, sendIndex);
	const uint8_t responseSrcSlot = bSrcPos < HAVE_MAX
		? static_cast<uint8_t>(SLOT_MAX + bSrcPos)
		: bSrcPos;
	auto sendResponse = [&](int16_t result, int16_t duration)
	{
		SetShort(sendBuffer, result, sendIndex);
		SetDWORD(sendBuffer, nItemID, sendIndex);
		SetShort(sendBuffer, sCount, sendIndex);
		SetShort(sendBuffer, duration, sendIndex);
		SetDWORD(sendBuffer, nGold, sendIndex);
		SetByte(sendBuffer, responseSrcSlot, sendIndex);
		SetByte(sendBuffer, bDstPos, sendIndex);
		Send(sendBuffer, sendIndex);
	};

	model::Item* pTable = m_pMain->m_ItemTableMap.GetData(nItemID);
	if (pTable == nullptr || nItemID == 0 || nItemID > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())
		|| nItemID >= static_cast<uint32_t>(ITEM_NO_TRADE)
		|| bSrcPos >= HAVE_MAX
		|| bDstPos >= MAX_MERCH_ITEMS
		|| sCount <= 0
		|| (pTable != nullptr && pTable->Countable == 0 && sCount != 1)
		|| nGold == 0 || nGold > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
	{
		sendResponse(0, 0);
		return;
	}

	uint8_t invSlot       = static_cast<uint8_t>(SLOT_MAX + bSrcPos);
	_ITEM_DATA* pSrcItem  = &m_pUserData->m_sItemArray[invSlot];
	if (pSrcItem->nNum != static_cast<int32_t>(nItemID) || pSrcItem->sCount < sCount)
	{
		sendResponse(0, 0);
		return;
	}

	if (m_arMerchantItems[bDstPos].nNum != 0)
	{
		sendResponse(0, 0);
		return;
	}

	_MERCH_DATA* pMerch   = &m_arMerchantItems[bDstPos];
	pMerch->nNum          = nItemID;
	pMerch->nPrice        = nGold;
	pMerch->sCount        = sCount;
	pMerch->sDuration     = pSrcItem->sDuration;
	pMerch->nSerialNum    = pSrcItem->nSerialNum;
	pMerch->bOriginalSlot = invSlot;
	pMerch->IsSoldOut     = false;
	pMerch->item          = *pSrcItem;
	pMerch->item.sCount   = sCount;

	// The listed quantity is removed from the inventory and held by the shop
	// until it is sold, cancelled, or the shop closes.
	pSrcItem->sCount -= sCount;
	if (pSrcItem->sCount <= 0)
		memset(pSrcItem, 0, sizeof(_ITEM_DATA));

	sendResponse(1, pMerch->sDuration);
}

void CUser::MerchantItemCancel(char* pBuf)
{
	int index = 0, sendIndex = 0;
	uint8_t bSrcPos = GetByte(pBuf, index);

	char sendBuffer[16] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_ITEM_CANCEL, sendIndex);

	if (bSrcPos >= MAX_MERCH_ITEMS || m_arMerchantItems[bSrcPos].nNum == 0)
	{
		SetShort(sendBuffer, 0, sendIndex);
		Send(sendBuffer, sendIndex);
		return;
	}

	if (!MerchantReturnItem(m_arMerchantItems[bSrcPos]))
	{
		SetShort(sendBuffer, 0, sendIndex);
		Send(sendBuffer, sendIndex);
		return;
	}

	SetShort(sendBuffer, 1, sendIndex);
	SetByte(sendBuffer, bSrcPos, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::MerchantItemList(char* pBuf)
{
	int index = 0;
	int16_t uid = static_cast<int16_t>(GetShort(pBuf, index));

	if (m_sMerchantsSocketID >= 0 && m_sMerchantsSocketID != uid)
		RemoveFromMerchantLookers();
	if (uid < 0 || uid == _socketId)
	{
		if (m_sMerchantsSocketID == uid)
			RemoveFromMerchantLookers();
		SendMerchantItemListPacket(this, 0, uid, nullptr);
		return;
	}

	auto pMerchant = m_pMain->GetUserPtr(uid);
	spdlog::info("MerchantItemList: uid={} pMerchantNotNull={} isMerchanting={} mySocketId={}",
		uid, pMerchant != nullptr, pMerchant ? pMerchant->isMerchanting() : false, _socketId);
	if (pMerchant == nullptr || !pMerchant->isMerchanting() || pMerchant->m_pUserData == nullptr
		|| m_pUserData == nullptr)
	{
		if (m_sMerchantsSocketID == uid)
			RemoveFromMerchantLookers();
		SendMerchantItemListPacket(this, 0, uid, nullptr);
		return;
	}

	// A shop may only be browsed from within the merchant's own region; this
	// prevents remote access to shops elsewhere in the zone.
	if (pMerchant->m_pUserData->m_bZone != m_pUserData->m_bZone
		|| std::abs(static_cast<int>(pMerchant->m_RegionX) - static_cast<int>(m_RegionX)) > 1
		|| std::abs(static_cast<int>(pMerchant->m_RegionZ) - static_cast<int>(m_RegionZ)) > 1)
	{
		if (m_sMerchantsSocketID == uid)
			RemoveFromMerchantLookers();
		SendMerchantItemListPacket(this, 0, uid, nullptr);
		return;
	}

	m_sMerchantsSocketID = uid;
	pMerchant->m_arMerchantLookers.remove(_socketId);
	pMerchant->m_arMerchantLookers.push_front(_socketId);
	SendMerchantItemListPacket(this, 1, uid, pMerchant->m_arMerchantItems);
}

void CUser::MerchantItemBuy(char* pBuf)
{
	int index = 0;
	uint32_t itemid = 0, req_gold = 0;
	int16_t item_count = 0, remaining_count = 0;
	uint8_t item_slot = 0, dest_slot = 0;

	itemid     = GetDWORD(pBuf, index);
	item_count = static_cast<int16_t>(GetShort(pBuf, index));
	item_slot  = GetByte(pBuf, index);
	dest_slot  = GetByte(pBuf, index);
	GetByte(pBuf, index); // rob_slot (unused)

	auto pMerchant = m_pMain->GetUserPtr(m_sMerchantsSocketID);
	if (pMerchant == nullptr)
		return;

	// The buyer must currently be browsing this merchant's shop.
	if (std::find(pMerchant->m_arMerchantLookers.begin(), pMerchant->m_arMerchantLookers.end(), _socketId)
		== pMerchant->m_arMerchantLookers.end())
		return;

	if (item_slot >= MAX_MERCH_ITEMS || dest_slot >= HAVE_MAX || item_count <= 0)
		return;

	_MERCH_DATA* pMerch = &pMerchant->m_arMerchantItems[item_slot];
	if (pMerch->nNum != itemid || pMerch->sCount == 0
		|| pMerchant->m_pUserData == nullptr)
		return;

	_ITEM_DATA* pItem = &m_pUserData->m_sItemArray[SLOT_MAX + dest_slot];

	if (pMerch->sCount < item_count)
		return;

	model::Item* proto = m_pMain->m_ItemTableMap.GetData(itemid);
	if (proto == nullptr || (proto->Countable == 0 && item_count != 1))
		return;

	if (pMerch->nPrice == 0
		|| pMerch->nPrice > static_cast<uint32_t>(std::numeric_limits<int32_t>::max() / item_count))
		return;
	req_gold = pMerch->nPrice * static_cast<uint32_t>(item_count);
	if (m_pUserData->m_iGold < static_cast<int32_t>(req_gold))
		return;

	// Destination slot must be empty, or the same stackable item.
	if (pItem->nNum != 0 && (pItem->nNum != static_cast<int32_t>(itemid) || proto->Countable == 0))
		return;
	if (pItem->sCount > std::numeric_limits<int16_t>::max() - item_count)
		return;

	remaining_count = static_cast<int16_t>(pMerch->sCount - item_count);

	// Transfer gold from buyer to merchant.
	if (req_gold > 0)
	{
		if (!GoldLose(static_cast<int>(req_gold)))
			return;

		pMerchant->GoldGain(static_cast<int>(req_gold));
	}

	// Give the item to the buyer.
	if (pItem->nNum == 0)
	{
		pItem->nNum    = itemid;
		pItem->sCount  = 0;
	}
	pItem->sCount    += item_count;
	pItem->sDuration  = pMerch->sDuration;
	pItem->nSerialNum = pMerch->nSerialNum;

	// Remove the sold quantity from the shop entry. The listed quantity was
	// already taken out of the seller's inventory when it was put up for sale.
	const int16_t remainingDuration = pMerch->sDuration;
	pMerch->sCount -= item_count;
	if (pMerch->sCount < 0)
		pMerch->sCount = 0;
	if (pMerch->sCount == 0)
		*pMerch = _MERCH_DATA {};

	SendMerchantStackChange(itemid, static_cast<uint16_t>(pItem->sCount), pItem->sDuration,
		dest_slot, pItem->sCount == item_count);

	// Notify the merchant that an item was purchased and update their shop window.
	int merchantIndex = 0;
	char merchantBuffer[96] {};
	SetByte(merchantBuffer, WIZ_MERCHANT, merchantIndex);
	SetByte(merchantBuffer, MERCHANT_ITEM_PURCHASED, merchantIndex);
	SetDWORD(merchantBuffer, itemid, merchantIndex);
	SetByte(merchantBuffer, item_slot, merchantIndex);
	SetShort(merchantBuffer, pMerch->sCount, merchantIndex);
	SetShort(merchantBuffer, remainingDuration, merchantIndex);
	SetString2(merchantBuffer, m_pUserData->m_id, merchantIndex);
	pMerchant->Send(merchantBuffer, merchantIndex);

	// Notify the buyer.
	int buyIndex = 0;
	char buyBuffer[32] {};
	SetByte(buyBuffer, WIZ_MERCHANT, buyIndex);
	SetByte(buyBuffer, MERCHANT_ITEM_BUY, buyIndex);
	SetShort(buyBuffer, 1, buyIndex);
	SetDWORD(buyBuffer, itemid, buyIndex);
	SetShort(buyBuffer, remaining_count, buyIndex);
	SetByte(buyBuffer, item_slot, buyIndex);
	SetByte(buyBuffer, dest_slot, buyIndex);
	Send(buyBuffer, buyIndex);
	pMerchant->BroadcastMerchantItemList();

	// If the merchant has nothing left for sale, close their shop.
	int itemsRemaining = 0;
	for (int i = 0; i < MAX_MERCH_ITEMS; i++)
	{
		if (pMerchant->m_arMerchantItems[i].nNum != 0)
			++itemsRemaining;
	}

	if (itemsRemaining == 0)
		pMerchant->MerchantClose();
	else if (remaining_count == 0)
		pMerchant->BroadcastMerchantAnnouncement();
}

void CUser::MerchantInsert(char* pBuf)
{
	// A shop may only be announced from a valid selling context: alive, in a
	// merchant town and not currently trading. This prevents a spoofed shop
	// broadcast from an invalid state (e.g. dead, PvP zone, mid-exchange).
	if (m_pUserData == nullptr || m_bResHpType == USER_DEAD || m_pUserData->m_sHp == 0
		|| !IsMerchantZone(m_pUserData->m_bZone) || m_sExchangeUser != -1)
		return;

	if (m_bMerchantState == MERCHANT_STATE_SELLING)
		return;

	int index = 0;
	uint16_t msgLen = static_cast<uint16_t>(GetShort(pBuf, index));
	if (msgLen > MAX_MERCH_MESSAGE)
		return;

	char message[MAX_MERCH_MESSAGE + 1] {};
	GetString(message, pBuf, msgLen, index);
	message[msgLen] = '\0';

	m_strMerchantMessage.assign(message, msgLen);
	m_bMerchantState = MERCHANT_STATE_SELLING;
	BroadcastMerchantAnnouncement();
}

void CUser::BroadcastMerchantAnnouncement()
{
	if (!isMerchanting() || m_pUserData == nullptr || m_pMain == nullptr)
		return;

	int sendIndex = 0;
	char sendBuffer[256] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_INSERT, sendIndex);
	SetShort(sendBuffer, 1, sendIndex);
	SetString2(sendBuffer, m_strMerchantMessage, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	for (int i = 0; i < MAX_MERCH_ITEMS; i++)
		SetDWORD(sendBuffer, m_arMerchantItems[i].nNum, sendIndex);
	for (int i = 0; i < MAX_MERCH_ITEMS; i++)
		SetDWORD(sendBuffer, m_arMerchantItems[i].nPrice, sendIndex);

	m_pMain->Send_Region(sendBuffer, sendIndex, m_pUserData->m_bZone, m_RegionX, m_RegionZ, this, false);
	Send(sendBuffer, sendIndex);
}

void CUser::SendMerchantAnnouncementTo(CUser* pReceiver)
{
	if (pReceiver == nullptr || !isMerchanting() || m_pUserData == nullptr)
		return;

	int sendIndex = 0;
	char sendBuffer[256] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_INSERT, sendIndex);
	SetShort(sendBuffer, 1, sendIndex);
	SetString2(sendBuffer, m_strMerchantMessage, sendIndex);
	SetShort(sendBuffer, _socketId, sendIndex);
	for (int i = 0; i < MAX_MERCH_ITEMS; i++)
		SetDWORD(sendBuffer, m_arMerchantItems[i].nNum, sendIndex);
	for (int i = 0; i < MAX_MERCH_ITEMS; i++)
		SetDWORD(sendBuffer, m_arMerchantItems[i].nPrice, sendIndex);
	pReceiver->Send(sendBuffer, sendIndex);
}

void CUser::SendMerchantAnnouncementsForRegion()
{
	if (m_pMain == nullptr || m_pUserData == nullptr)
		return;

	const int socketCount = m_pMain->GetUserSocketCount();
	for (int i = 0; i < socketCount; i++)
	{
		auto pMerchant = m_pMain->GetUserPtrUnchecked(i);
		if (pMerchant == nullptr || pMerchant.get() == this || !pMerchant->isMerchanting()
			|| pMerchant->m_pUserData == nullptr
			|| pMerchant->GetState() != CONNECTION_STATE_GAMESTART
			|| pMerchant->m_pUserData->m_bZone != m_pUserData->m_bZone
			|| std::abs(static_cast<int>(pMerchant->m_RegionX) - m_RegionX) > 1
			|| std::abs(static_cast<int>(pMerchant->m_RegionZ) - m_RegionZ) > 1)
			continue;

		pMerchant->SendMerchantAnnouncementTo(this);
	}
}

void CUser::BroadcastMerchantItemList()
{
	if (!isMerchanting())
		return;

	for (auto it = m_arMerchantLookers.begin(); it != m_arMerchantLookers.end();)
	{
		auto pLooker = m_pMain->GetUserPtr(*it);
		if (pLooker == nullptr || pLooker->m_sMerchantsSocketID != _socketId
			|| pLooker->GetState() != CONNECTION_STATE_GAMESTART)
		{
			if (pLooker != nullptr && pLooker->m_sMerchantsSocketID == _socketId)
				pLooker->m_sMerchantsSocketID = -1;
			it = m_arMerchantLookers.erase(it);
			continue;
		}

		SendMerchantItemListPacket(pLooker.get(), 1, static_cast<int16_t>(_socketId), m_arMerchantItems);
		++it;
	}
}

void CUser::PruneMerchantLookersOutsideRegion()
{
	if (!isMerchanting() || m_pMain == nullptr || m_pUserData == nullptr)
		return;

	for (auto it = m_arMerchantLookers.begin(); it != m_arMerchantLookers.end();)
	{
		auto pLooker = m_pMain->GetUserPtr(*it);
		const bool remainsInRegion = pLooker != nullptr && pLooker->m_pUserData != nullptr
			&& pLooker->m_pUserData->m_bZone == m_pUserData->m_bZone
			&& std::abs(static_cast<int>(pLooker->m_RegionX) - m_RegionX) <= 1
			&& std::abs(static_cast<int>(pLooker->m_RegionZ) - m_RegionZ) <= 1;
		if (remainsInRegion)
		{
			++it;
			continue;
		}

		if (pLooker != nullptr)
		{
			SendMerchantClose(pLooker.get(), static_cast<int16_t>(_socketId));
			if (pLooker->m_sMerchantsSocketID == _socketId)
				pLooker->m_sMerchantsSocketID = -1;
		}
		it = m_arMerchantLookers.erase(it);
	}
}

void CUser::CancelMerchant()
{
	if (m_sMerchantsSocketID < 0)
		return;

	RemoveFromMerchantLookers();

	int sendIndex = 0;
	char sendBuffer[16] {};
	SetByte(sendBuffer, WIZ_MERCHANT, sendIndex);
	SetByte(sendBuffer, MERCHANT_TRADE_CANCEL, sendIndex);
	SetShort(sendBuffer, 1, sendIndex);
	Send(sendBuffer, sendIndex);
}

void CUser::GiveMerchantItems()
{
	// Return any remaining listed items (including partially-sold stacks) to
	// the seller's inventory when the shop closes.
	for (auto& merch : m_arMerchantItems)
	{
		if (merch.nNum == 0)
			continue;

		if (!MerchantReturnItem(merch))
		{
			spdlog::error("User::GiveMerchantItems: no inventory space to return item {} x{} [accountName={} characterName={}]",
				merch.nNum, merch.sCount, m_strAccountID, m_pUserData->m_id);
			merch = _MERCH_DATA {};
		}
	}
}

bool CUser::MerchantReturnItem(_MERCH_DATA& merch)
{
	if (merch.nNum == 0 || merch.sCount == 0 || m_pUserData == nullptr)
	{
		merch = _MERCH_DATA {};
		return true;
	}

	model::Item* pTable    = m_pMain->m_ItemTableMap.GetData(merch.nNum);
	const bool bCountable  = pTable != nullptr && pTable->Countable != 0;

	int dst = -1;
	if (bCountable)
	{
		for (int i = 0; i < HAVE_MAX; i++)
		{
			if (m_pUserData->m_sItemArray[SLOT_MAX + i].nNum == static_cast<int32_t>(merch.nNum))
			{
				dst = i;
				break;
			}
		}
	}
	if (dst == -1)
	{
		for (int i = 0; i < HAVE_MAX; i++)
		{
			if (m_pUserData->m_sItemArray[SLOT_MAX + i].nNum == 0)
			{
				dst = i;
				break;
			}
		}
	}
	if (dst == -1)
		return false;

	_ITEM_DATA& slot = m_pUserData->m_sItemArray[SLOT_MAX + dst];
	if (slot.nNum == 0)
	{
		slot         = merch.item;
		slot.sCount  = merch.sCount;
	}
	else
	{
		int32_t merged = static_cast<int32_t>(slot.sCount) + merch.sCount;
		if (merged > std::numeric_limits<int16_t>::max())
			merged = std::numeric_limits<int16_t>::max();
		slot.sCount = static_cast<int16_t>(merged);
	}

	merch = _MERCH_DATA {};
	return true;
}

void CUser::RemoveFromMerchantLookers()
{
	auto pPreviousMerchantUser = m_pMain->GetUserPtr(m_sMerchantsSocketID);
	if (pPreviousMerchantUser != nullptr)
		pPreviousMerchantUser->m_arMerchantLookers.remove(_socketId);

	m_sMerchantsSocketID = -1;
}

void CUser::SendMerchantStackChange(uint32_t itemid, uint16_t count, int16_t duration,
	uint8_t slot, bool isNew)
{
	int sendIndex = 0;
	char sendBuffer[32] {};
	SetByte(sendBuffer, WIZ_ITEM_COUNT_CHANGE, sendIndex);
	SetShort(sendBuffer, 1, sendIndex); // number of entries
	SetByte(sendBuffer, 1, sendIndex);  // district: 1 = inventory bag
	SetByte(sendBuffer, slot, sendIndex);
	SetDWORD(sendBuffer, itemid, sendIndex);
	SetDWORD(sendBuffer, count, sendIndex);
	SetByte(sendBuffer, isNew ? ITEM_COUNT_CHANGE_NEW : ITEM_COUNT_CHANGE_EXISTING, sendIndex);
	SetShort(sendBuffer, duration, sendIndex);
	Send(sendBuffer, sendIndex);
}

} // namespace Ebenezer
