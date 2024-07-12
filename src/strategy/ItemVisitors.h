/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ITEMVISITORS_H
#define _PLAYERBOT_ITEMVISITORS_H

#include "ChatHelper.h"
#include "Common.h"
#include "Item.h"
#include "ItemUsageValue.h"

// Forward declarations
class AiObjectContext;
class Player;

// Function to perform a case-insensitive substring search
char* strstri(char const* str1, char const* str2);

// Enum for iterating over different item locations
enum IterateItemsMask : uint32
{
    ITERATE_ITEMS_IN_BAGS   = 1,   // Iterate over items in bags
    ITERATE_ITEMS_IN_EQUIP  = 2,   // Iterate over equipped items
    ITERATE_ITEMS_IN_BANK   = 4,   // Iterate over items in the bank
    ITERATE_ALL_ITEMS       = 255  // Iterate over all items
};

// Base class for item visitors
class IterateItemsVisitor
{
    public:
        // Constructor
        IterateItemsVisitor() { }

        // Virtual function to visit an item
        virtual bool Visit(Item* item) = 0;
};

// Derived class for finding items
class FindItemVisitor : public IterateItemsVisitor
{
    public:
        // Constructor
        FindItemVisitor() : IterateItemsVisitor(), result() { }

        // Function to visit an item and check if it is accepted
        bool Visit(Item* item) override
        {
            // If the item is not accepted, return true
            if (!Accept(item->GetTemplate()))
                return true;

            // Add the item to the result if it is accepted
            result.push_back(item);
            return true;
        }

        // Function to get the result
        std::vector<Item*>& GetResult() { return result; }

    protected:
        // Virtual function to accept an item template
        virtual bool Accept(ItemTemplate const* proto) = 0;

    private:
        std::vector<Item*> result;  // Vector to store the result
};

// Derived class for finding usable items
class FindUsableItemVisitor : public FindItemVisitor
{
    public:
        // Constructor
        FindUsableItemVisitor(Player* bot) : FindItemVisitor(), bot(bot) { }

        // Function to visit an item and check if it is usable
        bool Visit(Item* item) override;

    private:
        Player* bot;  // Pointer to the bot player
};

// Derived class for finding items by quality
class FindItemsByQualityVisitor : public IterateItemsVisitor
{
    public:
        // Constructor
        FindItemsByQualityVisitor(uint32 quality, uint32 count) : IterateItemsVisitor(), quality(quality), count(count) { }

        // Function to visit an item and check if it matches the quality
        bool Visit(Item* item) override
        {
            // If the item quality does not match, return true
            if (item->GetTemplate()->Quality != quality)
                return true;

            // If the result size exceeds the count, return false
            if (result.size() >= (size_t)count)
                return false;

            // Add the item to the result if it matches the quality
            result.push_back(item);
            return true;
        }

        // Function to get the result
        std::vector<Item*>& GetResult()
        {
            return result;
        }

    private:
        uint32 quality;  // Desired quality of items
        uint32 count;    // Maximum count of items to find
        std::vector<Item*> result;  // Vector to store the result
};

// Derived class for finding items to trade by quality
class FindItemsToTradeByQualityVisitor : public FindItemsByQualityVisitor
{
    public:
        // Constructor
        FindItemsToTradeByQualityVisitor(uint32 quality, uint32 count) : FindItemsByQualityVisitor(quality, count) { }

        // Function to visit an item and check if it can be traded
        bool Visit(Item* item) override
        {
            // If the item is soulbound, return true
            if (item->IsSoulBound())
                return true;

            // Check if the item matches the quality and can be traded
            return FindItemsByQualityVisitor::Visit(item);
        }
};

// Derived class for finding items to trade by class
class FindItemsToTradeByClassVisitor : public IterateItemsVisitor
{
    public:
        // Constructor
        FindItemsToTradeByClassVisitor(uint32 itemClass, uint32 itemSubClass, uint32 count)
            : IterateItemsVisitor(), itemClass(itemClass), itemSubClass(itemSubClass), count(count) { } // Reorder args - whipowill 

        // Function to visit an item and check if it matches the class and subclass
        bool Visit(Item* item) override
        {
            // If the item is soulbound, return true
            if (item->IsSoulBound())
                return true;

            // If the item does not match the class and subclass, return true
            if (item->GetTemplate()->Class != itemClass || item->GetTemplate()->SubClass != itemSubClass)
                return true;

            // If the result size exceeds the count, return false
            if (result.size() >= (size_t)count)
                return false;

            // Add the item to the result if it matches the class and subclass
            result.push_back(item);
            return true;
        }

        // Function to get the result
        std::vector<Item*>& GetResult()
        {
            return result;
        }

    private:
        uint32 itemClass;     // Desired class of items
        uint32 itemSubClass;  // Desired subclass of items
        uint32 count;         // Maximum count of items to find
        std::vector<Item*> result;  // Vector to store the result
};

// Derived class for querying item count
class QueryItemCountVisitor : public IterateItemsVisitor
{
    public:
        // Constructor
        QueryItemCountVisitor(uint32 itemId) : count(0), itemId(itemId) { }

        // Function to visit an item and count it if it matches the item ID
        bool Visit(Item* item) override
        {
            // If the item ID matches, increment the count
            if (item->GetTemplate()->ItemId == itemId)
                count += item->GetCount();

            return true;
        }

        // Function to get the item count
        uint32 GetCount() { return count; }

    protected:
        uint32 count;    // Count of items
        uint32 itemId;   // Desired item ID
};

// Derived class for querying named item count
class QueryNamedItemCountVisitor : public QueryItemCountVisitor
{
    public:
        // Constructor
        QueryNamedItemCountVisitor(std::string const name) : QueryItemCountVisitor(0), name(name) { }

        // Function to visit an item and count it if its name matches the desired name
        bool Visit(Item* item) override
        {
            // Get the item template
            ItemTemplate const* proto = item->GetTemplate();
            // If the item template and name match, increment the count
            if (proto && proto->Name1.c_str() && strstri(proto->Name1.c_str(), name.c_str()))
                count += item->GetCount();

            return true;
        }

    private:
        std::string const name;  // Desired name of items
};

// Derived class for finding named items
class FindNamedItemVisitor : public FindItemVisitor
{
    public:
        // Constructor
        FindNamedItemVisitor([[maybe_unused]] Player* bot, std::string const name) : FindItemVisitor(), name(name) { }

        // Function to accept an item template and check if its name matches the desired name
        bool Accept(ItemTemplate const* proto) override
        {
            return proto && proto->Name1.c_str() && strstri(proto->Name1.c_str(), name.c_str());
        }

    private:
        std::string const name;  // Desired name of items
};

// Derived class for finding items by ID
class FindItemByIdVisitor : public FindItemVisitor
{
    public:
        // Constructor
        FindItemByIdVisitor(uint32 id) : FindItemVisitor(), id(id) { }

        // Function to accept an item template and check if it matches the desired ID
        bool Accept(ItemTemplate const* proto) override
        {
            return proto->ItemId == id;
        }

    private:
        uint32 id;  // Desired item ID
};

// Derived class for finding items by a set of IDs
class FindItemByIdsVisitor : public FindItemVisitor
{
    public:
        // Constructor
        FindItemByIdsVisitor(ItemIds ids) : FindItemVisitor(), ids(ids) { }

        // Function to accept an item template and check if its ID is in the desired set of IDs
        bool Accept(ItemTemplate const* proto) override
        {
            return ids.find(proto->ItemId) != ids.end();
        }

    private:
        ItemIds ids;  // Set of desired item IDs
};

// Derived class for listing items
class ListItemsVisitor : public IterateItemsVisitor
{
    public:
        // Constructor
        ListItemsVisitor() : IterateItemsVisitor() { }

        std::map<uint32, uint32> items;    // Map to store item IDs and counts
        std::map<uint32, bool> soulbound;  // Map to store item IDs and their soulbound status

        // Function to visit an item and list it
        bool Visit(Item* item) override
        {
            // Get the item ID
            uint32 id = item->GetTemplate()->ItemId;

            // If the item ID is not in the map, add it with a count of 0
            if (items.find(id) == items.end())
                items[id] = 0;

            // Increment the count of the item
            items[id] += item->GetCount();
            // Store the soulbound status of the item
            soulbound[id] = item->IsSoulBound();
            return true;
        }
};

// Derived class for counting items by quality
class ItemCountByQuality : public IterateItemsVisitor
{
    public:
        // Constructor
        ItemCountByQuality() : IterateItemsVisitor()
        {
            // Initialize the count for each item quality to 0
            for (uint32 i = 0; i < MAX_ITEM_QUALITY; ++i)
                count[i] = 0;
        }

        // Function to visit an item and count it by its quality
        bool Visit(Item* item) override
        {
            ++count[item->GetTemplate()->Quality];
            return true;
        }

    public:
        std::map<uint32, uint32> count;  // Map to store the count of items by quality
};

// Derived class for finding potions
class FindPotionVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindPotionVisitor(Player* bot, uint32 effectId) : FindUsableItemVisitor(bot), effectId(effectId) { }

        // Function to accept an item template and check if it is a potion with the desired effect
        bool Accept(ItemTemplate const* proto) override;

    private:
        uint32 effectId;  // Desired effect ID of the potion
};

// Derived class for finding food
class FindFoodVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindFoodVisitor(Player* bot, uint32 spellCategory, bool conjured = false) : FindUsableItemVisitor(bot),
            spellCategory(spellCategory), conjured(conjured) { }

        // Function to accept an item template and check if it is food with the desired spell category
        bool Accept(ItemTemplate const* proto) override
        {
            return proto->Class == ITEM_CLASS_CONSUMABLE && (proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD) &&
                proto->Spells[0].SpellCategory == spellCategory && (!conjured || proto->IsConjuredConsumable());
        }

    private:
        uint32 spellCategory;  // Desired spell category of the food
        bool conjured;         // Whether the food is conjured
};

// Derived class for finding mounts
class FindMountVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindMountVisitor(Player* bot) : FindUsableItemVisitor(bot) { }

        // Function to accept an item template and check if it is a mount
        bool Accept(ItemTemplate const* proto) override;

    private:
        uint32 effectId;  // Desired effect ID of the mount
};

// Derived class for finding pets
class FindPetVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindPetVisitor(Player* bot) : FindUsableItemVisitor(bot) { }

        // Function to accept an item template and check if it is a pet
        bool Accept(ItemTemplate const* proto) override;
};

// Derived class for finding ammunition
class FindAmmoVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindAmmoVisitor(Player* bot, uint32 weaponType) : FindUsableItemVisitor(bot), weaponType(weaponType) { }

        // Function to accept an item template and check if it is ammunition for the desired weapon type
        bool Accept(ItemTemplate const* proto)override
        {
            // Check if the item is a projectile
            if (proto->Class == ITEM_CLASS_PROJECTILE)
            {
                uint32 subClass = 0;
                // Determine the subclass of the projectile based on the weapon type
                switch (weaponType)
                {
                    case ITEM_SUBCLASS_WEAPON_GUN:
                        subClass = ITEM_SUBCLASS_BULLET;
                        break;
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                        subClass = ITEM_SUBCLASS_ARROW;
                        break;
                }

                // If the subclass is not determined, return false
                if (!subClass)
                    return false;

                // If the item subclass matches the determined subclass, return true
                if (proto->SubClass == subClass)
                    return true;
            }

            // If the item is not the desired ammunition, return false
            return false;
        }

    private:
        uint32 weaponType;  // Desired weapon type for the ammunition
};

// Derived class for finding quest items
class FindQuestItemVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindQuestItemVisitor(Player* bot) : FindUsableItemVisitor(bot) { }

        // Function to accept an item template and check if it is a quest item
        bool Accept(ItemTemplate const* proto) override
        {
            // Check if the item is a quest item
            if (proto->Class == ITEM_CLASS_QUEST)
            {
                return true;
            }
            return false;
        }
};

// Derived class for finding recipes
class FindRecipeVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindRecipeVisitor(Player* bot, SkillType skill = SKILL_NONE) : FindUsableItemVisitor(bot), skill(skill) { }

        // Function to accept an item template and check if it is a recipe for the desired skill
        bool Accept(ItemTemplate const* proto) override
        {
            // Check if the item is a recipe
            if (proto->Class == ITEM_CLASS_RECIPE)
            {
                // If no specific skill is desired, return true
                if (skill == SKILL_NONE)
                    return true;

                // Check if the recipe matches the desired skill
                switch (proto->SubClass)
                {
                    case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                        return skill == SKILL_LEATHERWORKING;
                    case ITEM_SUBCLASS_TAILORING_PATTERN:
                        return skill == SKILL_TAILORING;
                    case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                        return skill == SKILL_ENGINEERING;
                    case ITEM_SUBCLASS_BLACKSMITHING:
                        return skill == SKILL_BLACKSMITHING;
                    case ITEM_SUBCLASS_COOKING_RECIPE:
                        return skill == SKILL_COOKING;
                    case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                        return skill == SKILL_ALCHEMY;
                    case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                        return skill == SKILL_FIRST_AID;
                    case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                        return skill == SKILL_ENCHANTING;
                    case ITEM_SUBCLASS_FISHING_MANUAL:
                        return skill == SKILL_FISHING;
                }
            }

            // If the item is not a desired recipe, return false
            return false;
        }

    private:
        SkillType skill;  // Desired skill type for the recipe
};

// Derived class for finding items by usage
class FindItemUsageVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindItemUsageVisitor(Player* bot, ItemUsage usage = ITEM_USAGE_NONE);

        // Function to accept an item template and check if it matches the desired usage
        bool Accept(ItemTemplate const* proto) override;

    private:
        AiObjectContext* context;  // AI object context for the bot
        ItemUsage usage;           // Desired item usage
};

// Derived class for finding usable named items
class FindUsableNamedItemVisitor : public FindUsableItemVisitor
{
    public:
        // Constructor
        FindUsableNamedItemVisitor(Player* bot, std::string name): FindUsableItemVisitor(bot), name(name) {}

        // Function to accept an item template and check if its name matches the desired name
        bool Accept(ItemTemplate const* proto) override;

    private:
        std::string name;  // Desired name of items
};

#endif
