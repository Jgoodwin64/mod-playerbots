/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATHELPER_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_CHATHELPER_H

#include "Common.h"  // Include common definitions and utilities
#include "ObjectGuid.h"  // Include ObjectGuid definitions
#include "PlayerbotAIAware.h"  // Include the PlayerbotAIAware class definition
#include "SharedDefines.h"  // Include shared definitions

#include <map>  // Include the map container from the standard library

// Forward declarations of various classes
class GameObject;
class Quest;
class Player;
class PlayerbotAI;
class SpellInfo;
class WorldObject;

// Forward declaration of ItemTemplate struct
struct ItemTemplate;

// Typedefs for sets of item and spell IDs
typedef std::set<uint32> ItemIds;
typedef std::set<uint32> SpellIds;

// ChatHelper class, inherits from PlayerbotAIAware
class ChatHelper : public PlayerbotAIAware
{
    public:
        // Constructor that initializes the base class with the provided PlayerbotAI pointer
        ChatHelper(PlayerbotAI* botAI);

        // Static methods for formatting and parsing various game-related data
        static std::string const formatMoney(uint32 copper);
        static uint32 parseMoney(std::string const text);
        static ItemIds parseItems(std::string const text);
        uint32 parseSpell(std::string const text);
        static std::string const FormatQuest(Quest const* quest);
        static std::string const FormatItem(ItemTemplate const* proto, uint32 count = 0, uint32 total = 0);
        static std::string const FormatQItem(uint32 itemId);
        static std::string const FormatSpell(SpellInfo const* spellInfo);
        static std::string const FormatGameobject(GameObject* go);
        static std::string const FormatWorldobject(WorldObject* wo);
        static std::string const FormatWorldEntry(int32 entry);
        static std::string const FormatQuestObjective(std::string const name, uint32 available, uint32 required);
        static GuidVector parseGameobjects(std::string const text);

        // Static methods for parsing and formatting chat messages
        static ChatMsg parseChat(std::string const text);
        static std::string const FormatChat(ChatMsg chat);

        // Static methods for formatting player classes, races, skills, and boolean flags
        static std::string const FormatClass(Player* player, int8 spec);
        static std::string const FormatClass(uint8 cls);
        static std::string const FormatRace(uint8 race);
        static std::string const FormatSkill(uint32 skill);
        static std::string const FormatBoolean(bool flag);

        // Static methods for parsing item qualities, classes, and equipment slots
        static uint32 parseItemQuality(std::string const text);
        static bool parseItemClass(std::string const text, uint32* itemClass, uint32* itemSubClass);
        static uint32 parseSlot(std::string const text);
        uint32 parseSkill(std::string const text);

        // Static method to check if a string is parseable
        static bool parseable(std::string const text);

        // Method to erase all occurrences of a substring from a string
        void eraseAllSubStr(std::string& mainStr, std::string const toErase);

    private:
        // Static member variables for various mappings
        static std::map<std::string, uint32> consumableSubClasses;
        static std::map<std::string, uint32> tradeSubClasses;
        static std::map<std::string, uint32> itemQualities;
        static std::map<std::string, uint32> projectileSubClasses;
        static std::map<std::string, uint32> slots;
        static std::map<std::string, uint32> skills;
        static std::map<std::string, ChatMsg> chats;
        static std::map<uint8, std::string> classes;
        static std::map<uint8, std::string> races;
        static std::map<uint8, std::map<uint8, std::string>> specs;
};

#endif  // End of header guard
