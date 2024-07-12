/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTDUNGEONSUGGESTIONMGR_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERBOTDUNGEONSUGGESTIONMGR_H

#include "Common.h"  // Include common definitions and utilities
#include "DBCEnums.h"  // Include enumeration definitions for DBC (Database Client)

#include <map>  // Include the map container from the standard library
#include <vector>  // Include the vector container from the standard library

// Structure to hold information about a dungeon suggestion
struct DungeonSuggestion
{
    std::string name;  // Name of the dungeon
    Difficulty difficulty;  // Difficulty level of the dungeon
    uint8 min_level;  // Minimum player level for the dungeon
    uint8 max_level;  // Maximum player level for the dungeon
    std::string abbrevation;  // Abbreviation of the dungeon name
    std::string strategy;  // Suggested strategy for the dungeon
};

// Manager class for handling dungeon suggestions
class PlayerbotDungeonSuggestionMgr
{
    public:
        // Constructor
        PlayerbotDungeonSuggestionMgr() { };
        // Destructor
         ~PlayerbotDungeonSuggestionMgr() { };
        // Static instance method for singleton pattern
        static PlayerbotDungeonSuggestionMgr* instance()
        {
            static PlayerbotDungeonSuggestionMgr instance;  // Static instance of the manager
            return &instance;  // Return pointer to the static instance
        }

        // Method to load dungeon suggestions from some source
        void LoadDungeonSuggestions();
        // Method to get the list of dungeon suggestions
        std::vector<DungeonSuggestion> const GetDungeonSuggestions();

    private:
        // Vector to hold the loaded dungeon suggestions
        std::vector<DungeonSuggestion> m_dungeonSuggestions;
};

// Macro to access the singleton instance of the dungeon suggestion manager
#define sPlayerbotDungeonSuggestionMgr PlayerbotDungeonSuggestionMgr::instance()

#endif  // End of header guard
