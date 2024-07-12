/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotDbStore.h"  // Include the PlayerbotDbStore class definition
#include "Playerbots.h"  // Include the Playerbots definitions

#include <iostream>  // Include standard I/O stream library

// Function to load the bot AI settings from the database
void PlayerbotDbStore::Load(PlayerbotAI* botAI)
{
    // Get the bot's unique identifier
    ObjectGuid::LowType guid = botAI->GetBot()->GetGUID().GetCounter();

    // Prepare a database statement to select the bot's stored data
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_DB_STORE);
    stmt->SetData(0, guid);  // Set the bot's GUID as a parameter
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        // Clear current strategies for combat and non-combat states
        botAI->ClearStrategies(BOT_STATE_COMBAT);
        botAI->ClearStrategies(BOT_STATE_NON_COMBAT);
        // Set initial chat strategy for both combat and non-combat states
        botAI->ChangeStrategy("+chat", BOT_STATE_COMBAT);
        botAI->ChangeStrategy("+chat", BOT_STATE_NON_COMBAT);

        std::vector<std::string> values;  // Vector to store values from the database
        do
        {
            // Fetch the current row of results
            Field* fields = result->Fetch();
            // Retrieve key and value from the row
            std::string const key = fields[0].Get<std::string>();
            std::string const value = fields[1].Get<std::string>();

            // Check the key and apply the value accordingly
            if (key == "value")
                values.push_back(value);  // Store generic values
            else if (key == "co")
                botAI->ChangeStrategy(value, BOT_STATE_COMBAT);  // Change combat strategy
            else if (key == "nc")
                botAI->ChangeStrategy(value, BOT_STATE_NON_COMBAT);  // Change non-combat strategy
            else if (key == "dead")
                botAI->ChangeStrategy(value, BOT_STATE_DEAD);  // Change dead state strategy
        }
        while (result->NextRow());  // Continue to the next row of results

        // Load the context with the retrieved values
        botAI->GetAiObjectContext()->Load(values);
    }
}

// Function to save the bot AI settings to the database
void PlayerbotDbStore::Save(PlayerbotAI* botAI)
{
    // Get the bot's unique identifier
    ObjectGuid::LowType guid = botAI->GetBot()->GetGUID().GetCounter();

    // Reset the bot's stored data in the database
    Reset(botAI);

    // Get the data to save from the bot's AI context
    std::vector<std::string> data = botAI->GetAiObjectContext()->Save();
    for (std::vector<std::string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        // Save each value in the database
        SaveValue(guid, "value", *i);
    }

    // Save the bot's strategies for different states
    SaveValue(guid, "co", FormatStrategies("co", botAI->GetStrategies(BOT_STATE_COMBAT)));
    SaveValue(guid, "nc", FormatStrategies("nc", botAI->GetStrategies(BOT_STATE_NON_COMBAT)));
    SaveValue(guid, "dead", FormatStrategies("dead", botAI->GetStrategies(BOT_STATE_DEAD)));
}

// Function to format the strategies into a string for storage
std::string const PlayerbotDbStore::FormatStrategies(std::string const type, std::vector<std::string> strategies)
{
    std::ostringstream out;  // String stream to build the formatted string
    for (std::vector<std::string>::iterator i = strategies.begin(); i != strategies.end(); ++i)
        out << "+" << (*i).c_str() << ",";  // Add each strategy prefixed with '+' and followed by ','

    std::string const res = out.str();  // Convert the stream to a string
    return res.substr(0, res.size() - 1);  // Remove the trailing comma and return the result
}

// Function to reset the bot's stored data in the database
void PlayerbotDbStore::Reset(PlayerbotAI* botAI)
{
    // Get the bot's unique identifier
    ObjectGuid::LowType guid = botAI->GetBot()->GetGUID().GetCounter();

    // Prepare a database statement to delete the bot's custom strategies
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_CUSTOM_STRATEGY);
    stmt->SetData(0, guid);  // Set the bot's GUID as a parameter
    PlayerbotsDatabase.Execute(stmt);  // Execute the statement
}

// Function to save a key-value pair to the database
void PlayerbotDbStore::SaveValue(uint32 guid, std::string const key, std::string const value)
{
    // Prepare a database statement to insert the key-value pair
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_DB_STORE);
    stmt->SetData(0, guid);  // Set the bot's GUID as a parameter
    stmt->SetData(1, key);  // Set the key as a parameter
    stmt->SetData(2, value);  // Set the value as a parameter
    PlayerbotsDatabase.Execute(stmt);  // Execute the statement
}
