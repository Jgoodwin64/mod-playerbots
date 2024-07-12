/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlaceholderHelper.h"  // Include the PlaceholderHelper header
#include "AiFactory.h"  // Include the AiFactory header
#include "Playerbots.h"  // Include the Playerbots header
#include "PlayerbotTextMgr.h"  // Include the PlayerbotTextMgr header
#include "Util.h"  // Include the Util header

// Function to map dungeon information to placeholders
void PlaceholderHelper::MapDungeon(
    PlaceholderMap& placeholders,  // Map to store placeholder values
    DungeonSuggestion const* dungeonSuggestion,  // Pointer to the dungeon suggestion data
    Player* bot  // Pointer to the bot player
)
{
    std::ostringstream out;  // Output string stream to build the dungeon string
    Insertion insertion = {out, dungeonSuggestion};  // Create an insertion object with the stream and dungeon suggestion
    InsertDungeonName(insertion);  // Insert the dungeon name into the stream
    InsertDungeonStrategy(insertion);  // Insert the dungeon strategy into the stream
    InsertDifficulty(insertion, bot);  // Insert the dungeon difficulty into the stream

    placeholders["%dungeon"] = out.str();  // Add the final dungeon string to the placeholders map
}

// Function to map the role of the bot player to placeholders
void PlaceholderHelper::MapRole(PlaceholderMap& placeholders, Player* bot)
{
    BotRoles const role = AiFactory::GetPlayerRoles(bot);  // Get the role of the bot player
    std::string roleText;  // String to hold the role text
    switch (role)  // Determine the role and set the corresponding text
    {
        case BOT_ROLE_TANK:
            roleText = "Tank";
            break;
        case BOT_ROLE_HEALER:
            roleText = "Healer";
            break;
        case BOT_ROLE_DPS:
            roleText = "DPS";
            break;
        case BOT_ROLE_NONE:
        default:
            return;  // If the role is none or unknown, exit the function
    }

    bool const hasRole = !roleText.empty();  // Check if the role text is not empty
    if (hasRole)
    {
        placeholders["%role"] = roleText;  // Add the role text to the placeholders map
    }
}

// Function to insert the dungeon name into the output stream
void PlaceholderHelper::InsertDungeonName(Insertion& insertion)
{
    std::string name = insertion.dungeonSuggestion->name;  // Get the dungeon name
    bool const hasAbbrevation = !insertion.dungeonSuggestion->abbrevation.empty();  // Check if there is an abbreviation
    if (hasAbbrevation)
    {
        name = insertion.dungeonSuggestion->abbrevation;  // Use the abbreviation if available
    }

    insertion.out << "|c00b000b0" << name << "|r";  // Insert the name into the stream with color formatting
}

// Function to insert the dungeon strategy into the output stream
void PlaceholderHelper::InsertDungeonStrategy(Insertion& insertion)
{
    bool const hasStrategy = !insertion.dungeonSuggestion->strategy.empty();  // Check if there is a strategy
    bool const isRandomlyUsingStrategy = urand(0, 1);  // Randomly decide whether to use the strategy
    if (hasStrategy && isRandomlyUsingStrategy)
    {
        std::string strategy = insertion.dungeonSuggestion->strategy;  // Get the strategy text
        insertion.out << " " + strategy;  // Insert the strategy into the stream
    }
}

// Function to insert the dungeon difficulty into the output stream
void PlaceholderHelper::InsertDifficulty(Insertion& insertion, [[maybe_unused]] Player* bot)
{
    bool const hasHeroic = insertion.dungeonSuggestion->difficulty == DUNGEON_DIFFICULTY_HEROIC;  // Check if the dungeon is heroic
    std::string difficultyText;  // String to hold the difficulty text
    if (hasHeroic)
    {
        bool const isRandomlyNormal = urand(0, 1);  // Randomly decide whether to use "Normal"
        bool const isRandomlyHeroic = urand(0, 1);  // Randomly decide whether to use "Heroic"
        std::vector<std::string> normalAbbrevations = {"Normal", "N"};  // List of normal difficulty abbreviations
        std::vector<std::string> heroicAbbrevations = {"Heroic", "HC", "H"};  // List of heroic difficulty abbreviations
        uint32 const randomAbbrevationIndex = urand(0, 1);  // Random index for the abbreviations
        if (isRandomlyNormal)
        {
            difficultyText = normalAbbrevations[randomAbbrevationIndex];  // Choose a normal abbreviation
        }
        else if (isRandomlyHeroic)
        {
            difficultyText = heroicAbbrevations[randomAbbrevationIndex];  // Choose a heroic abbreviation
        }

        insertion.out << " " << difficultyText;  // Insert the difficulty text into the stream
    }
}
