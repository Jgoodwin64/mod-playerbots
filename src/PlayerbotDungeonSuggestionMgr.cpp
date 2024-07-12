/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotDungeonSuggestionMgr.h"  // Include the header for the PlayerbotDungeonSuggestionMgr class
#include "Playerbots.h"  // Include the Playerbots header for additional functionalities and dependencies

// Function to retrieve dungeon suggestions
std::vector<DungeonSuggestion> const PlayerbotDungeonSuggestionMgr::GetDungeonSuggestions()
{
    return m_dungeonSuggestions;  // Return the vector of dungeon suggestions stored in the manager
}

// Function to load dungeon suggestions from the database
void PlayerbotDungeonSuggestionMgr::LoadDungeonSuggestions()
{
    LOG_INFO("server.loading", "Loading playerbots dungeon suggestions...");  // Log the start of the loading process
    uint32 oldMSTime = getMSTime();  // Record the current time in milliseconds for performance tracking

    uint32 count = 0;  // Initialize a counter for the number of loaded dungeon suggestions
    auto statement = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_DUNGEON_SUGGESTION);  // Prepare a statement to query dungeon suggestions from the database
    uint8 const expansion = sWorld->getIntConfig(CONFIG_EXPANSION);  // Retrieve the current expansion configuration
    statement->SetData(0, expansion);  // Bind the expansion value to the prepared statement

    PreparedQueryResult result = PlayerbotsDatabase.Query(statement);  // Execute the query and store the result
    if (result)  // Check if the query returned any results
    {
        do
        {
            Field* fields = result->Fetch();  // Fetch the current row of the result set
            std::string const name = fields[0].Get<std::string>();  // Get the dungeon name from the first column
            uint8 const difficulty = fields[1].Get<uint8>();  // Get the dungeon difficulty from the second column
            uint8 const min_level = fields[2].Get<uint8>();  // Get the minimum level requirement from the third column
            uint8 const max_level = fields[3].Get<uint8>();  // Get the maximum level requirement from the fourth column
            std::string const abbrevation = fields[4].Get<std::string>();  // Get the dungeon abbreviation from the fifth column
            std::string const strategy = fields[5].Get<std::string>();  // Get the strategy for the dungeon from the sixth column

            // Create a DungeonSuggestion object with the retrieved values
            DungeonSuggestion const row =
            {
                name,
                static_cast<Difficulty>(difficulty),  // Cast the difficulty value to the Difficulty enum type
                min_level,
                max_level,
                abbrevation,
                strategy
            };

            m_dungeonSuggestions.push_back(row);  // Add the dungeon suggestion to the vector
            ++count;  // Increment the counter
        }
        while (result->NextRow());  // Continue to the next row in the result set
    }

    // Log the number of loaded dungeon suggestions and the time taken to load them
    LOG_INFO("server.loading", "{} playerbots dungeon suggestions loaded in {} ms",
        count, GetMSTimeDiffToNow(oldMSTime));
}
