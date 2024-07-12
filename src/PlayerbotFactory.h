/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTFACTORY_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERBOTFACTORY_H

#include "InventoryAction.h"  // Include InventoryAction definitions and utilities
#include "Player.h"  // Include Player class definition
#include "PlayerbotAI.h"  // Include PlayerbotAI class definition

class Item;  // Forward declaration of Item class

// Structure defining an enchant template with class ID, spec ID, spell ID, and slot ID
struct ItemTemplate;

struct EnchantTemplate
{
    uint8   ClassId;  // ID of the class
    uint8   SpecId;  // ID of the specialization
    uint32  SpellId;  // ID of the spell
    uint8   SlotId;  // ID of the slot
};

// Define a container type for storing multiple EnchantTemplate objects
typedef std::vector<EnchantTemplate> EnchantContainer;

// TODO: Add more specs/roles
/* classid+talenttree
enum spec : uint8
{
    WARRIOR_ARMS = 10,
    WARRIOR_FURY = 11,
    WARRIOR_PROT = 12,
    ROLE_HEALER = 1,
    ROLE_MDPS = 2,
    ROLE_CDPS = 3,
};
*/

/* Define different roles as enums
enum roles : uint8
{
    ROLE_TANK   = 0,
    ROLE_HEALER = 1,
    ROLE_MDPS   = 2,
    ROLE_CDPS   = 3
};*/

// Enum defining IDs for various prioritised consumables
enum PriorizedConsumables
{
    CONSUM_ID_ROUGH_WEIGHTSTONE             = 3239,
    CONSUM_ID_COARSE_WEIGHTSTONE            = 3239,
    CONSUM_ID_HEAVY_WEIGHTSTONE             = 3241,
    CONSUM_ID_SOLID_WEIGHTSTONE             = 7965,
    CONSUM_ID_DENSE_WEIGHTSTONE             = 12643,
    CONSUM_ID_FEL_WEIGHTSTONE               = 28420,
    CONSUM_ID_ADAMANTITE_WEIGHTSTONE        = 28421,
    CONSUM_ID_ROUGH_SHARPENING_STONE        = 2862,
    CONSUM_ID_COARSE_SHARPENING_STONE       = 2863,
    CONSUM_ID_HEAVY_SHARPENING_STONE        = 2871,
    CONSUM_ID_SOL_SHARPENING_STONE          = 7964,
    CONSUM_ID_DENSE_SHARPENING_STONE        = 12404,
    CONSUM_ID_ELEMENTAL_SHARPENING_STONE    = 18262,
    CONSUM_ID_CONSECRATED_SHARPENING_STONE  = 23122,
    CONSUM_ID_FEL_SHARPENING_STONE          = 23528,
    CONSUM_ID_ADAMANTITE_SHARPENING_STONE   = 23529,
    CONSUM_ID_LINEN_BANDAGE                 = 1251,
    CONSUM_ID_HEAVY_LINEN_BANDAGE           = 2581,
    CONSUM_ID_WOOL_BANDAGE                  = 3530,
    CONSUM_ID_HEAVY_WOOL_BANDAGE            = 3531,
    CONSUM_ID_SILK_BANDAGE                  = 6450,
    CONSUM_ID_HEAVY_SILK_BANDAGE            = 6451,
    CONSUM_ID_MAGEWEAVE_BANDAGE             = 8544,
    CONSUM_ID_HEAVY_MAGEWEAVE_BANDAGE       = 8545,
    CONSUM_ID_RUNECLOTH_BANDAGE             = 14529,
    CONSUM_ID_HEAVY_RUNECLOTH_BANDAGE       = 14530,
    CONSUM_ID_NETHERWEAVE_BANDAGE           = 21990,
    CONSUM_ID_HEAVY_NETHERWEAVE_BANDAGE     = 21991,
    CONSUM_ID_BRILLIANT_MANA_OIL            = 20748,
    CONSUM_ID_MINOR_MANA_OIL                = 20745,
    CONSUM_ID_SUPERIOR_MANA_OIL             = 22521,
    CONSUM_ID_LESSER_MANA_OIL               = 20747,
    CONSUM_ID_BRILLIANT_WIZARD_OIL          = 20749,
    CONSUM_ID_MINOR_WIZARD_OIL              = 20744,
    CONSUM_ID_SUPERIOR_WIZARD_OIL           = 22522,
    CONSUM_ID_WIZARD_OIL                    = 20750,
    CONSUM_ID_LESSER_WIZARD_OIL             = 20746,
    CONSUM_ID_INSTANT_POISON                = 6947,
    CONSUM_ID_INSTANT_POISON_II             = 6949,
    CONSUM_ID_INSTANT_POISON_III            = 6950,
    CONSUM_ID_INSTANT_POISON_IV             = 8926,
    CONSUM_ID_INSTANT_POISON_V              = 8927,
    CONSUM_ID_INSTANT_POISON_VI             = 8928,
    CONSUM_ID_INSTANT_POISON_VII            = 21927,
    CONSUM_ID_DEADLY_POISON                 = 2892,
    CONSUM_ID_DEADLY_POISON_II              = 2893,
    CONSUM_ID_DEADLY_POISON_III             = 8984,
    CONSUM_ID_DEADLY_POISON_IV              = 8985,
    CONSUM_ID_DEADLY_POISON_V               = 20844,
    CONSUM_ID_DEADLY_POISON_VI              = 22053,
    CONSUM_ID_DEADLY_POISON_VII             = 22054
};

#define MAX_CONSUM_ID 28  // Maximum number of consumable IDs

// PlayerbotFactory class definition
class PlayerbotFactory
{
    public:
        // Constructor for initializing PlayerbotFactory with bot, level, item quality, and gear score limit
        PlayerbotFactory(Player* bot, uint32 level, uint32 itemQuality = 0, uint32 gearScoreLimit = 0);

        // Static method to get a random bot
        static ObjectGuid GetRandomBot();
        // Static method to initialize the factory
        static void Init();
        // Method to refresh the bot's state
        void Refresh();
        // Method to randomize the bot's properties, with an optional incremental flag
        void Randomize(bool incremental);
        // Static list of class quest IDs
        static std::list<uint32> classQuestIds;
        // Method to clear all bot's properties and inventory
        void ClearEverything();
        // Method to initialize bot's skills
        void InitSkills();

        // Static array of trade skill IDs
        static uint32 tradeSkills[];
        // Static method to calculate item score for a bot
        static float CalculateItemScore(uint32 item_id, Player* bot);
        // Static method to calculate enchant score for a bot
        static float CalculateEnchantScore(uint32 enchant_id, Player* bot);
        // Static method to calculate spell score for a bot, with an optional trigger parameter
        static float CalculateSpellScore(uint32 spell_id, Player* bot, uint32 trigger = ITEM_SPELLTRIGGER_ON_EQUIP);
        // Method to initialize talent tree with various options
        void InitTalentsTree(bool incremental = false, bool use_template = true, bool reset = false);
        // Static method to initialize talents by spec number
        static void InitTalentsBySpecNo(Player* bot, int specNo, bool reset);
        // Static method to initialize talents by parsed spec link
        static void InitTalentsByParsedSpecLink(Player* bot, std::vector<std::vector<uint32>> parsedSpecLink, bool reset);
        // Method to initialize available spells
        void InitAvailableSpells();
        // Method to initialize class-specific spells
        void InitClassSpells();
        // Method to initialize equipment with an optional incremental flag
        void InitEquipment(bool incremental);
        // Method to initialize the bot's pet
        void InitPet();
        // Method to initialize ammo for the bot
        void InitAmmo();
        // Static method to calculate mixed gear score based on gear score and quality
        static uint32 CalcMixedGearScore(uint32 gs, uint32 quality);
        // Method to initialize pet talents
        void InitPetTalents();
        
        // Method to initialize reagents
        void InitReagents();
        // Method to initialize glyphs with an optional incremental flag
        void InitGlyphs(bool increment = false);
        // Method to initialize food items
        void InitFood();
        // Method to initialize mounts
        void InitMounts();
        // Method to initialize bags with an optional flag to destroy old ones
        void InitBags(bool destroyOld = true);
        // Method to apply enchantments and gems with an optional flag to destroy old ones
        void ApplyEnchantAndGemsNew(bool destoryOld = true);
        // Method to initialize instance quests
        void InitInstanceQuests();
        // Method to unbind from instances
        void UnbindInstance();
    private:
        // Method to prepare the bot factory
        void Prepare();
        // Commented out methods for future implementation
        // void InitSecondEquipmentSet();
        // void InitEquipmentNew(bool incremental);
        // Method to check if an item can be equipped based on its template and desired quality
        bool CanEquipItem(ItemTemplate const* proto, uint32 desiredQuality);
        // Method to check if an unseen item can be equipped in a given slot
        bool CanEquipUnseenItem(uint8 slot, uint16& dest, uint32 item);
        // Method to initialize trade skills
        void InitTradeSkills();
        // Method to update trade skills
        void UpdateTradeSkills();
        // Method to set a random trade skill
        void SetRandomSkill(uint16 id);
        // Method to initialize spells
        void InitSpells();
        // Method to clear all spells
        void ClearSpells();
        // Method to clear all skills
        void ClearSkills();
        // Method to initialize special spells
        void InitSpecialSpells();
        // Method to initialize talents based on spec number
        void InitTalents(uint32 specNo);
        // Method to initialize talents based on a template
        void InitTalentsByTemplate(uint32 specNo);
        // Method to initialize quests based on a quest map
        void InitQuests(std::list<uint32>& questMap);
        // Method to clear the bot's inventory
        void ClearInventory();
        // Method to clear all items from the bot's inventory
        void ClearAllItems();
        // Method to reset quests
        void ResetQuests();
        // Method to initialize potions
        void InitPotions();
        
        // Method to check if the bot can equip armor based on its template
        bool CanEquipArmor(ItemTemplate const* proto);
        // Method to check if the bot can equip a weapon based on its template
        bool CanEquipWeapon(ItemTemplate const* proto);
        // Method to enchant an item
        void EnchantItem(Item* item);
        // Method to add item stats
        void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank);
        // Method to check item stats
        bool CheckItemStats(uint8 sp, uint8 ap, uint8 tank);
        // Method to cancel all auras
        void CancelAuras();
        // Method to check if an item is a desired replacement
        bool IsDesiredReplacement(Item* item);
        // Method to initialize the bot's inventory
        void InitInventory();
        // Method to initialize trade items in the inventory
        void InitInventoryTrade();
        // Method to initialize equipped items in the inventory
        void InitInventoryEquip();
        // Method to initialize skill items in the inventory
        void InitInventorySkill();
        // Method to store an item in the bot's inventory
        Item* StoreItem(uint32 itemId, uint32 count);
        // Method to initialize the bot's guild
        void InitGuild();
        // Method to initialize the bot's arena team
        void InitArenaTeam();
        // Method to initialize immersive elements for the bot
        void InitImmersive();
        // Method to add consumables to the bot's inventory
        void AddConsumables();
        // Static method to add prerequisite quests to a list of quest IDs
        static void AddPrevQuests(uint32 questId, std::list<uint32>& questIds);
        // Method to load the enchant container
        void LoadEnchantContainer();
        // Method to apply an enchant template
        void ApplyEnchantTemplate();
        // Method to apply an enchant template based on the spec
        void ApplyEnchantTemplate(uint8 spec);  
        // Method to get possible inventory types for a given slot
        std::vector<InventoryType> GetPossibleInventoryTypeListBySlot(EquipmentSlots slot);
        // Static method to check if a bot is a shield tank
        static bool IsShieldTank(Player* bot);
        // Static method to check if an item is not the same armor type as the bot's class
        static bool NotSameArmorType(uint32 item_subclass_armor, Player* bot);
        // Method to iterate over items with a given visitor and mask
        void IterateItems(IterateItemsVisitor* visitor, IterateItemsMask mask = ITERATE_ITEMS_IN_BAGS);
        // Method to iterate over items in bags with a given visitor
        void IterateItemsInBags(IterateItemsVisitor* visitor);
        // Method to iterate over equipped items with a given visitor
        void IterateItemsInEquip(IterateItemsVisitor* visitor);
        // Method to iterate over items in the bank with a given visitor
        void IterateItemsInBank(IterateItemsVisitor* visitor);
        // Method to get the begin iterator for the enchant container
        EnchantContainer::const_iterator GetEnchantContainerBegin() { return m_EnchantContainer.begin(); }
        // Method to get the end iterator for the enchant container
        EnchantContainer::const_iterator GetEnchantContainerEnd() { return m_EnchantContainer.end(); }
        
        uint32 level;  // Level of the bot
        uint32 itemQuality;  // Quality of the items
        uint32 gearScoreLimit;  // Gear score limit for the bot
        static std::list<uint32> specialQuestIds;  // Static list of special quest IDs
        std::vector<uint32> trainerIdCache;  // Cache of trainer IDs
        static std::vector<uint32> enchantSpellIdCache;  // Static cache of enchant spell IDs
        static std::vector<uint32> enchantGemIdCache;  // Static cache of enchant gem IDs

    protected:
        EnchantContainer m_EnchantContainer;  // Container for storing enchant templates
        Player* bot;  // Pointer to the bot player
        PlayerbotAI* botAI;  // Pointer to the bot's AI
};

#endif  // End of header guard
