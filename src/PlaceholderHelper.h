/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLACEHOLDERHELPER_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLACEHOLDERHELPER_H

#include "Common.h"  // Include common definitions and utilities
#include "Player.h"  // Include Player class definition
#include "PlayerbotDungeonSuggestionMgr.h"  // Include Dungeon Suggestion Manager

#include <map>  // Include the map container from the standard library

// Define a type alias for a map that holds placeholder strings
typedef std::map<std::string, std::string> PlaceholderMap;

// PlaceholderHelper class to assist with placeholder mappings
class PlaceholderHelper
{
    public:
        // Static function to map role-related placeholders for a given bot
        static void MapRole(PlaceholderMap& placeholders, Player* bot);

        // Static function to map dungeon-related placeholders for a given dungeon suggestion and bot
        static void MapDungeon(
            PlaceholderMap& placeholders,
            DungeonSuggestion const* dungeonSuggestion,
            Player* bot
        );

    private:
        // Insertion struct to help with inserting dungeon information
        struct Insertion
        {
            std::ostringstream& out;  // Output stream to write formatted text
            DungeonSuggestion const* dungeonSuggestion;  // Pointer to the dungeon suggestion
        };

        // Static function to insert the dungeon name into the output stream
        static void InsertDungeonName(Insertion& insertion);

        // Static function to insert the dungeon strategy into the output stream
        static void InsertDungeonStrategy(Insertion& insertion);

        // Static function to insert the dungeon difficulty into the output stream based on the bot
        static void InsertDifficulty(Insertion& insertion, Player* bot);
};

#endif  // End of header guard
