/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LootObjectStack.h"  // Include the LootObjectStack header file
#include "LootMgr.h"          // Include the LootMgr header file
#include "Playerbots.h"       // Include the Playerbots header file
#include "Unit.h"             // Include the Unit header file

#define MAX_LOOT_OBJECT_COUNT 10  // Define the maximum count for loot objects

// Constructor for LootTarget, initializing the guid and setting the asOfTime to the current time
LootTarget::LootTarget(ObjectGuid guid) : guid(guid), asOfTime(time(nullptr))
{
}

// Copy constructor for LootTarget
LootTarget::LootTarget(LootTarget const& other)
{
    guid = other.guid;          // Copy the guid from the other object
    asOfTime = other.asOfTime;  // Copy the asOfTime from the other object
}

// Assignment operator for LootTarget
LootTarget& LootTarget::operator=(LootTarget const& other)
{
    if ((void*)this == (void*)&other)  // Check for self-assignment
        return *this;

    guid = other.guid;          // Copy the guid from the other object
    asOfTime = other.asOfTime;  // Copy the asOfTime from the other object

    return *this;
}

// Less-than operator for comparing LootTarget objects based on guid
bool LootTarget::operator<(LootTarget const& other) const
{
    return guid < other.guid;  // Compare guids to determine order
}

// Shrink the LootTargetList by removing entries older than fromTime
void LootTargetList::shrink(time_t fromTime)
{
    for (std::set<LootTarget>::iterator i = begin(); i != end(); )
    {
        if (i->asOfTime <= fromTime)  // Check if the entry is older than fromTime
            erase(i++);  // Erase the entry and move to the next
        else
            ++i;  // Move to the next entry
    }
}

// Constructor for LootObject, initializing member variables and refreshing the object
LootObject::LootObject(Player* bot, ObjectGuid guid) : guid(), skillId(SKILL_NONE), reqSkillValue(0), reqItem(0)
{
    Refresh(bot, guid);  // Refresh the object with the given guid
}

// Refresh the LootObject with the details from the given lootGUID
void LootObject::Refresh(Player* bot, ObjectGuid lootGUID)
{
    // Reset member variables
    skillId = SKILL_NONE;
    reqSkillValue = 0;
    reqItem = 0;
    guid.Clear();

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);  // Get the bot AI
    if (!botAI) {
        return;  // Exit if bot AI is not available
    }

    // Check if the lootGUID corresponds to a creature
    Creature* creature = botAI->GetCreature(lootGUID);
    if (creature && creature->getDeathState() == DeathState::Corpse)
    {
        // Check if the creature is lootable
        if (creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
            guid = lootGUID;

        // Check if the creature is skinnable
        if (creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
        {
            skillId = creature->GetCreatureTemplate()->GetRequiredLootSkill();  // Get the required loot skill
            uint32 targetLevel = creature->getLevel();  // Get the creature's level
            reqSkillValue = targetLevel < 10 ? 1 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;  // Calculate the required skill value
            if (botAI->HasSkill((SkillType)skillId) && bot->GetSkillValue(skillId) >= reqSkillValue)
                guid = lootGUID;  // Set the guid if the bot has the required skill
        }

        return;
    }

    // Check if the lootGUID corresponds to a game object
    GameObject* go = botAI->GetGameObject(lootGUID);
    if (go && go->isSpawned() && go->GetGoState() == GO_STATE_READY)
    {
        bool isQuestItemOnly = false;

        // Check if the game object has quest items
        GameObjectQuestItemList const* items = sObjectMgr->GetGameObjectQuestItemList(go->GetEntry());
        for (int i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; i++)
        {
            if (!items || i >= items->size())
                break;

            auto itemId = uint32((*items)[i]);

            if (IsNeededForQuest(bot, itemId))
            {
                this->guid = guid;  // Set the guid if the item is needed for a quest
                return;
            }
            isQuestItemOnly |= itemId > 0;
        }

        if (isQuestItemOnly)
            return;

        uint32 goId = go->GetEntry();  // Get the game object entry ID
        uint32 lockId = go->GetGOInfo()->GetLockId();  // Get the lock ID
        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);  // Get the lock info
        if (!lockInfo)
            return;

        // Check the lock types and requirements
        for (uint8 i = 0; i < 8; ++i)
        {
            switch (lockInfo->Type[i])
            {
                case LOCK_KEY_ITEM:
                    if (lockInfo->Index[i] > 0)
                    {
                        reqItem = lockInfo->Index[i];  // Set the required item ID
                        guid = lootGUID;
                    }
                    break;
                case LOCK_KEY_SKILL:
                    if (goId == 13891 || goId == 19535) // Serpentbloom
                    {
                        this->guid = guid;
                    }
                    else if (SkillByLockType(LockType(lockInfo->Index[i])) > 0)
                    {
                        skillId = SkillByLockType(LockType(lockInfo->Index[i]));  // Set the required skill ID
                        reqSkillValue = std::max((uint32)1, lockInfo->Skill[i]);  // Set the required skill value
                        guid = lootGUID;
                    }
                    break;
                case LOCK_KEY_NONE:
                    guid = lootGUID;
                    break;
            }
        }
    }
}

// Check if the itemId is needed for any quest the bot is on
bool LootObject::IsNeededForQuest(Player* bot, uint32 itemId)
{
    // Iterate through the bot's quest log
    for (int qs = 0; qs < MAX_QUEST_LOG_SIZE; ++qs)
    {
        uint32 questId = bot->GetQuestSlotQuestId(qs);
        if (questId == 0)
            continue;

        QuestStatusData& qData = bot->getQuestStatusMap()[questId];
        if (qData.Status != QUEST_STATUS_INCOMPLETE)
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
        if (!qInfo)
            continue;

        // Check the required items for the quest
        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            if (!qInfo->RequiredItemCount[i] || (qInfo->RequiredItemCount[i] - qData.ItemCount[i]) <= 0)
                continue;

            if (qInfo->RequiredItemId[i] != itemId)
                continue;

            return true;  // Return true if the item is needed for the quest
        }
    }

    return false;  // Return false if the item is not needed for any quest
}

// Get the WorldObject corresponding to the loot GUID, refreshing it first
WorldObject* LootObject::GetWorldObject(Player* bot)
{
    Refresh(bot, guid);  // Refresh the object with the given guid

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);  // Get the bot AI
    if (!botAI) {
        return nullptr;  // Return nullptr if bot AI is not available
    }
    Creature* creature = botAI->GetCreature(guid);
    if (creature && creature->getDeathState() == DeathState::Corpse && creature->IsInWorld())
        return creature;  // Return the creature if it is a corpse and in the world

    GameObject* go = botAI->GetGameObject(guid);
    if (go && go->isSpawned() && go->IsInWorld())
        return go;  // Return the game object if it is spawned and in the world

    return nullptr;  // Return nullptr if no valid WorldObject is found
}

// Copy constructor for LootObject
LootObject::LootObject(LootObject const& other)
{
    guid = other.guid;          // Copy the guid from the other object
    skillId = other.skillId;    // Copy the skillId from the other object
    reqSkillValue = other.reqSkillValue;  // Copy the reqSkillValue from the other object
    reqItem = other.reqItem;    // Copy the reqItem from the other object
}

// Check if it is possible to loot the object
bool LootObject::IsLootPossible(Player* bot)
{
    if (IsEmpty() || !GetWorldObject(bot))
        return false;  // Return false if the object is empty or no valid WorldObject is found

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);  // Get the bot AI
    if (!botAI) {
        return false;  // Return false if bot AI is not available
    }
    if (reqItem && !bot->HasItemCount(reqItem, 1))
        return false;  // Return false if the required item is not in the bot's inventory

    if (abs(GetWorldObject(bot)->GetPositionZ() - bot->GetPositionZ()) > INTERACTION_DISTANCE)
        return false;  // Return false if the object is too far away vertically
    
    Creature* creature = botAI->GetCreature(guid);
    if (creature && creature->getDeathState() == DeathState::Corpse)
    {
        if (!bot->isAllowedToLoot(creature) && skillId != SKILL_SKINNING)
            return false;  // Return false if the bot is not allowed to loot the creature and it is not skinning
    }

    if (skillId == SKILL_NONE)
        return true;  // Return true if no specific skill is required

    if (skillId == SKILL_FISHING)
        return false;  // Return false if the required skill is fishing

    if (!botAI->HasSkill((SkillType)skillId))
        return false;  // Return false if the bot does not have the required skill

    if (!reqSkillValue)
        return true;  // Return true if no specific skill value is required

    uint32 skillValue = uint32(bot->GetSkillValue(skillId));  // Get the bot's skill value
    if (reqSkillValue > skillValue)
        return false;  // Return false if the required skill value is higher than the bot's skill value

    // Check if the bot has the required tools for mining or skinning
    if (skillId == SKILL_MINING && !bot->HasItemCount(2901, 1))
        return false;  // Return false if the bot does not have a mining pick

    if (skillId == SKILL_SKINNING && !bot->HasItemCount(7005, 1))
        return false;  // Return false if the bot does not have a skinning knife

    return true;  // Return true if all conditions are met for looting
}

// Add a new loot GUID to the stack, managing the size of the stack
bool LootObjectStack::Add(ObjectGuid guid)
{
    if (availableLoot.size() >= MAX_LOOT_OBJECT_COUNT) {
        availableLoot.shrink(time(nullptr) - 30);  // Shrink the list if it exceeds the maximum count
    }

    if (availableLoot.size() >= MAX_LOOT_OBJECT_COUNT) {
        availableLoot.clear();  // Clear the list if it still exceeds the maximum count
    }

    if (!availableLoot.insert(guid).second)
        return false;  // Return false if the guid is already in the list

    return true;  // Return true if the guid was successfully added
}

// Remove a loot GUID from the stack
void LootObjectStack::Remove(ObjectGuid guid)
{
    LootTargetList::iterator i = availableLoot.find(guid);  // Find the guid in the list
    if (i != availableLoot.end())
        availableLoot.erase(i);  // Erase the guid if found
}

// Clear the loot stack
void LootObjectStack::Clear()
{
    availableLoot.clear();  // Clear the list
}

// Check if there is any lootable object within the specified distance
bool LootObjectStack::CanLoot(float maxDistance)
{
    std::vector<LootObject> ordered = OrderByDistance(maxDistance);  // Order the loot objects by distance
    return !ordered.empty();  // Return true if there are lootable objects within the distance
}

// Get the closest lootable object within the specified distance
LootObject LootObjectStack::GetLoot(float maxDistance)
{
    std::vector<LootObject> ordered = OrderByDistance(maxDistance);  // Order the loot objects by distance
    return ordered.empty() ? LootObject() : *ordered.begin();  // Return the closest lootable object or an empty object if none found
}

// Order lootable objects by distance within the specified maximum distance
std::vector<LootObject> LootObjectStack::OrderByDistance(float maxDistance)
{
    availableLoot.shrink(time(nullptr) - 30);  // Shrink the list to remove old entries

    std::map<float, LootObject> sortedMap;  // Map to store loot objects sorted by distance
    LootTargetList safeCopy(availableLoot);  // Create a safe copy of the available loot list
    for (LootTargetList::iterator i = safeCopy.begin(); i != safeCopy.end(); i++)
    {
        ObjectGuid guid = i->guid;
        LootObject lootObject(bot, guid);  // Create a LootObject for the current guid
        if (!lootObject.IsLootPossible(bot))
            continue;  // Skip if the loot is not possible
        
        float distance = bot->GetDistance(lootObject.GetWorldObject(bot));  // Calculate the distance to the loot object
        if (!maxDistance || distance <= maxDistance)
            sortedMap[distance] = lootObject;  // Add the loot object to the sorted map if within the max distance
    }

    std::vector<LootObject> result;  // Vector to store the ordered loot objects
    for (std::map<float, LootObject>::iterator i = sortedMap.begin(); i != sortedMap.end(); i++)
        result.push_back(i->second);  // Add the loot objects to the result vector

    return result;  // Return the ordered loot objects
}
