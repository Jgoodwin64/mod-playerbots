/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMITEMMGR_H
#define _PLAYERBOT_RANDOMITEMMGR_H

#include "AiFactory.h"
#include "Common.h"
#include "ItemTemplate.h"

#include <map>
#include <set>
#include <vector>

class ChatHandler;

struct ItemTemplate;

enum EquipmentSlots : uint32;

enum RandomItemType
{
    RANDOM_ITEM_GUILD_TASK,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE
};

#define MAX_STAT_SCALES 32

enum ItemSource
{
    ITEM_SOURCE_NONE,
    ITEM_SOURCE_DROP,
    ITEM_SOURCE_VENDOR,
    ITEM_SOURCE_QUEST,
    ITEM_SOURCE_CRAFT,
    ITEM_SOURCE_PVP
};

struct WeightScaleInfo
{
    uint32 id;
    std::string name;
};

struct WeightScaleStat
{
    std::string stat;
    uint32 weight;
};

struct StatWeight
{
    uint32 id;
    uint32 weight;
};

struct ItemInfoEntry
{
    ItemInfoEntry() : minLevel(0), source(0), sourceId(0), team(0), repRank(0), repFaction(0), quality(0), slot(0), itemId(0)
    {
        for (uint8 i = 1; i <= MAX_STAT_SCALES; ++i)
        {
            weights[i] = 0; // Initialize all weight scales to 0
        }
    }

    std::map<uint32, uint32> weights; // Weight scales for different stats
    uint32 minLevel; // Minimum level required for the item
    uint32 source; // Source of the item (drop, vendor, quest, etc.)
    uint32 sourceId; // ID of the source (NPC ID, quest ID, etc.)
    uint32 team; // Faction team (e.g., Alliance or Horde)
    uint32 repRank; // Reputation rank required
    uint32 repFaction; // Reputation faction required
    uint32 quality; // Quality of the item (e.g., common, rare)
    uint32 slot; // Equipment slot
    uint32 itemId; // Item ID
};

typedef std::vector<WeightScaleStat> WeightScaleStats;
//typedef std::map<WeightScaleInfo, WeightScaleStats> WeightScaleList;

struct WeightScale
{
    WeightScaleInfo info; // Information about the weight scale
    WeightScaleStats stats; // Statistics associated with the weight scale
};

//typedef map<uint32, WeightScale> WeightScales;

class RandomItemPredicate
{
    public:
        virtual ~RandomItemPredicate() { };

        virtual bool Apply(ItemTemplate const* proto) = 0; // Pure virtual function to apply predicate on an item template
};

typedef std::vector<uint32> RandomItemList;
typedef std::map<RandomItemType, RandomItemList> RandomItemCache;

class BotEquipKey
{
    public:
        BotEquipKey() : level(0), clazz(0), slot(0), quality(0), key(GetKey()) { }
        BotEquipKey(uint32 level, uint8 clazz, uint8 slot, uint32 quality) : level(level), clazz(clazz), slot(slot), quality(quality), key(GetKey()) { }
        BotEquipKey(BotEquipKey const& other)  : level(other.level), clazz(other.clazz), slot(other.slot), quality(other.quality), key(GetKey()) { }

        bool operator<(BotEquipKey const& other) const
        {
            return other.key < this->key; // Comparison operator for sorting keys
        }

        uint32 level;
        uint8 clazz;
        uint8 slot;
        uint32 quality;
        uint64 key;

    private:
        uint64 GetKey(); // Private method to generate a unique key based on item properties
};

typedef std::map<BotEquipKey, RandomItemList> BotEquipCache;

class RandomItemMgr
{
    public:
        RandomItemMgr();
        virtual ~RandomItemMgr();
        static RandomItemMgr* instance()
        {
            static RandomItemMgr instance; // Singleton instance
            return &instance;
        }

	public:
        void Init(); // Initialize the random item manager
        void InitAfterAhBot(); // Additional initialization after AH bot
        static bool HandleConsoleCommand(ChatHandler* handler, char const* args); // Handle console commands
        RandomItemList Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate); // Query for random items based on level, type, and predicate
        RandomItemList Query(uint32 level, uint8 clazz, uint8 slot, uint32 quality); // Query for random items based on level, class, slot, and quality
        uint32 GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId); // Get item upgrade for a player based on spec, slot, quality, and item ID
        std::vector<uint32> GetUpgradeList(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId, uint32 amount = 1); // Get list of item upgrades for a player
        bool HasStatWeight(uint32 itemId); // Check if an item has stat weights
        uint32 GetMinLevelFromCache(uint32 itemId); // Get minimum level for an item from cache
        uint32 GetStatWeight(Player* player, uint32 itemId); // Get stat weight for an item for a player
        uint32 GetLiveStatWeight(Player* player, uint32 itemId); // Get live stat weight for an item for a player
        uint32 GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate = nullptr); // Get a random item based on level and type
        uint32 GetAmmo(uint32 level, uint32 subClass); // Get random ammo based on level and subclass
        uint32 GetRandomPotion(uint32 level, uint32 effect); // Get random potion based on level and effect
        uint32 GetRandomFood(uint32 level, uint32 category); // Get random food based on level and category
        uint32 GetFood(uint32 level, uint32 category); // Get specific food based on level and category
        uint32 GetRandomTrade(uint32 level); // Get random trade item based on level
        uint32 CalculateStatWeight(uint8 playerclass, uint8 spec, ItemTemplate const* proto); // Calculate stat weight for an item for a player class and spec
        uint32 CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value); // Calculate weight for a single stat
        bool CanEquipArmor(uint8 clazz, uint32 level, ItemTemplate const* proto); // Check if a class can equip a specific armor
        bool ShouldEquipArmorForSpec(uint8 playerclass, uint8 spec, ItemTemplate const* proto); // Check if armor is suitable for a player's spec
        bool CanEquipWeapon(uint8 clazz, ItemTemplate const* proto); // Check if a class can equip a specific weapon
        bool ShouldEquipWeaponForSpec(uint8 playerclass, uint8 spec, ItemTemplate const* proto); // Check if weapon is suitable for a player's spec
        float GetItemRarity(uint32 itemId); // Get rarity of an item
        uint32 GetQuestIdForItem(uint32 itemId); // Get quest ID associated with an item
        std::vector<uint32> GetQuestIdsForItem(uint32 itemId); // Get all quest IDs associated with an item
        static bool IsUsedBySkill(ItemTemplate const* proto, uint32 skillId); // Check if an item is used by a specific skill
        bool IsTestItem(uint32 itemId) { return itemForTest.find(itemId) != itemForTest.end(); } // Check if an item is a test item
        std::vector<uint32> GetCachedEquipments(uint32 requiredLevel, uint32 inventoryType); // Get cached equipment items based on level and inventory type

    private:
        void BuildRandomItemCache(); // Build the cache for random items
        void BuildEquipCache(); // Build the cache for equipment items
        void BuildEquipCacheNew(); // Build the new cache for equipment items
        void BuildItemInfoCache(); // Build the cache for item info
        void BuildAmmoCache(); // Build the cache for ammo items
        void BuildFoodCache(); // Build the cache for food items
        void BuildPotionCache(); // Build the cache for potion items
        void BuildTradeCache(); // Build the cache for trade items
        void BuildRarityCache(); // Build the cache for item rarities
        bool CanEquipItem(BotEquipKey key, ItemTemplate const* proto); // Check if an item can be equipped based on key and item template
        bool CanEquipItemNew(ItemTemplate const* proto); // Check if an item can be equipped based on item template (new method)
        void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank); // Add item stats to specified variables
        bool CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank); // Check if item stats are valid for a class
    private:
        std::map<uint32, RandomItemCache> randomItemCache; // Cache for random items
        std::map<RandomItemType, RandomItemPredicate*> predicates; // Predicates for filtering items by type
        BotEquipCache equipCache; // Cache for equippable items
        std::map<EquipmentSlots, std::set<InventoryType>> viableSlots; // Map of viable slots for equipment
        std::map<uint32, std::map<uint32, uint32> > ammoCache; // Cache for ammo items
        std::map<uint32, std::map<uint32, std::vector<uint32> > > potionCache; // Cache for potion items
        std::map<uint32, std::map<uint32, std::vector<uint32> > > foodCache; // Cache for food items
        std::map<uint32, std::vector<uint32> > tradeCache; // Cache for trade items
        std::map<uint32, float> rarityCache; // Cache for item rarities
        std::map<uint8, WeightScale> m_weightScales[MAX_CLASSES]; // Weight scales for different classes
        std::map<std::string, uint32 > weightStatLink; // Link between stat names and their IDs
        std::map<std::string, uint32 > weightRatingLink; // Link between rating names and their IDs
        std::map<uint32, ItemInfoEntry> itemInfoCache; // Cache for item info entries
        std::set<uint32> itemForTest; // Set of items used for testing
        static std::set<uint32> itemCache; // Static cache of items
        // equipCacheNew[RequiredLevel][InventoryType]
        std::map<uint32, std::map<uint32, std::vector<uint32>>> equipCacheNew; // New cache for equippable items based on level and inventory type
};

#define sRandomItemMgr RandomItemMgr::instance() // Macro for accessing the singleton instance of RandomItemMgr

#endif
