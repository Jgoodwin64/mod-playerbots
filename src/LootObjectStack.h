/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTOBJECTSTACK_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_LOOTOBJECTSTACK_H

#include "ObjectGuid.h"  // Include the ObjectGuid header file

class AiObjectContext;  // Forward declaration of AiObjectContext class
class Player;  // Forward declaration of Player class
class WorldObject;  // Forward declaration of WorldObject class

struct ItemTemplate;  // Forward declaration of ItemTemplate struct

// Abstract base class defining the strategy for looting
class LootStrategy
{
    public:
        LootStrategy() { }  // Default constructor
        virtual ~LootStrategy() { };  // Virtual destructor
        // Pure virtual function to determine if an item can be looted
        virtual bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) = 0;
        // Pure virtual function to get the name of the loot strategy
        virtual std::string const GetName() = 0;
};

// Class representing a lootable object
class LootObject
{
    public:
        LootObject() : skillId(0), reqSkillValue(0), reqItem(0) { }  // Default constructor initializing member variables
        LootObject(Player* bot, ObjectGuid guid);  // Constructor initializing with a player and an object GUID
        LootObject(LootObject const& other);  // Copy constructor

        bool IsEmpty() { return !guid; }  // Check if the loot object is empty (no GUID)
        bool IsLootPossible(Player* bot);  // Check if looting is possible for the player
        void Refresh(Player* bot, ObjectGuid guid);  // Refresh the loot object with a new GUID
        WorldObject* GetWorldObject(Player* bot);  // Get the WorldObject corresponding to the GUID
        ObjectGuid guid;  // GUID of the loot object

        uint32 skillId;  // Skill ID required to loot the object
        uint32 reqSkillValue;  // Skill value required to loot the object
        uint32 reqItem;  // Item required to loot the object

    private:
        // Check if the item is needed for a quest
        static bool IsNeededForQuest(Player* bot, uint32 itemId);
};

// Class representing a target for looting
class LootTarget
{
    public:
        LootTarget(ObjectGuid guid);  // Constructor initializing with an object GUID
        LootTarget(LootTarget const& other);  // Copy constructor

    public:
        LootTarget& operator=(LootTarget const& other);  // Assignment operator
        bool operator<(LootTarget const& other) const;  // Less-than operator for sorting

    public:
        ObjectGuid guid;  // GUID of the loot target
        time_t asOfTime;  // Timestamp when the loot target was added
};

// Class representing a list of loot targets, inheriting from std::set
class LootTargetList : public std::set<LootTarget>
{
    public:
        // Remove loot targets that were added before a specified time
        void shrink(time_t fromTime);
};

// Class representing a stack of lootable objects
class LootObjectStack
{
    public:
        LootObjectStack(Player* bot) : bot(bot) { }  // Constructor initializing with a player pointer

        // Add a new loot object to the stack by GUID
        bool Add(ObjectGuid guid);
        // Remove a loot object from the stack by GUID
        void Remove(ObjectGuid guid);
        // Clear all loot objects from the stack
        void Clear();
        // Check if there are any lootable objects within a specified distance
        bool CanLoot(float maxDistance);
        // Get a lootable object within a specified distance
        LootObject GetLoot(float maxDistance = 0);

    private:
        // Order loot objects by distance from the player, optionally within a specified maximum distance
        std::vector<LootObject> OrderByDistance(float maxDistance = 0);

        Player* bot;  // Pointer to the player (bot) associated with this loot stack
        LootTargetList availableLoot;  // List of available loot targets
};

#endif  // End of header guard
