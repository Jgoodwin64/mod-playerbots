/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ItemVisitors.h"
#include "Playerbots.h"

// Function to visit an item and check if it is usable
bool FindUsableItemVisitor::Visit(Item* item)
{
    // Check if the bot can use the item
    if (bot->CanUseItem(item->GetTemplate()) == EQUIP_ERR_OK)
        // If the item is usable, visit the item
        return FindItemVisitor::Visit(item);

    // If the item is not usable, return true
    return true;
}

// Function to accept an item template and check if it is a potion or flask
bool FindPotionVisitor::Accept(ItemTemplate const* proto)
{
    // Check if the item is a consumable and is either a potion or a flask
    if (proto->Class == ITEM_CLASS_CONSUMABLE && (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK))
    {
        // Iterate through the spells of the item template
        for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            // Get the spell info from the spell manager
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            // If spell info is not found, return false
            if (!spellInfo)
                return false;

            // Iterate through the effects of the spell
            for (uint8 i = 0; i < 3; i++)
            {
                // If the effect matches the desired effect ID, return true
                if (spellInfo->Effects[i].Effect == effectId)
                    return true;
            }
        }
    }

    // If the item is not a desired potion or flask, return false
    return false;
}

// Function to accept an item template and check if it is a mount
bool FindMountVisitor::Accept(ItemTemplate const* proto)
{
    // Iterate through the spells of the item template
    for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        // Get the spell info from the spell manager
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
        // If spell info is not found, return false
        if (!spellInfo)
            return false;

        // Iterate through the effects of the spell
        for (uint8 i = 0; i < 3; i++)
        {
            // If the effect is a mount aura, return true
            if (spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOUNTED)
                return true;
        }
    }

    // If the item is not a mount, return false
    return false;
}

// Function to accept an item template and check if it is a pet
bool FindPetVisitor::Accept(ItemTemplate const* proto)
{
    // Check if the item is a miscellaneous item
    if (proto->Class == ITEM_CLASS_MISC)
    {
        // Iterate through the spells of the item template
        for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            // Get the spell info from the spell manager
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            // If spell info is not found, return false
            if (!spellInfo)
                return false;

            // Iterate through the effects of the spell
            for (uint8 i = 0; i < 3; i++)
            {
                // If the effect is to summon a pet, return true
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SUMMON_PET)
                    return true;
            }
        }
    }

    // If the item is not a pet, return false
    return false;
}

// Constructor for the FindItemUsageVisitor class
FindItemUsageVisitor::FindItemUsageVisitor(Player* bot, ItemUsage usage) : FindUsableItemVisitor(bot), usage(usage)
{
    // Get the AI object context for the bot
    context = GET_PLAYERBOT_AI(bot)->GetAiObjectContext();
};

// Function to accept an item template and check if it matches the desired usage
bool FindItemUsageVisitor::Accept(ItemTemplate const* proto)
{
    // Check if the item usage matches the desired usage
    if (AI_VALUE2(ItemUsage, "item usage", proto->ItemId) == usage)
        return true;

    // If the item usage does not match, return false
    return false;
}

// Function to accept an item template and check if it matches the desired name
bool FindUsableNamedItemVisitor::Accept(ItemTemplate const* proto)
{
    // Check if the item template is valid and its name matches the desired name
    return proto && !proto->Name1.empty() && strstri(proto->Name1.c_str(), name.c_str());
}
