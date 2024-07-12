/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTDBSTORE_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERBOTDBSTORE_H

#include "Common.h"  // Include common definitions and utilities

#include <vector>  // Include the vector container from the standard library

class PlayerbotAI;  // Forward declaration of the PlayerbotAI class

// PlayerbotDbStore class handles database operations for player bots
class PlayerbotDbStore
{
    public:
        // Constructor
        PlayerbotDbStore() { }
        // Virtual destructor
        virtual ~PlayerbotDbStore() { }

        // Static method to get the singleton instance of PlayerbotDbStore
        static PlayerbotDbStore* instance()
        {
            static PlayerbotDbStore instance;  // Singleton instance
            return &instance;  // Return the address of the singleton instance
        }

        // Save the state of the given PlayerbotAI instance to the database
        void Save(PlayerbotAI* botAI);
        // Load the state of the given PlayerbotAI instance from the database
        void Load(PlayerbotAI* botAI);
        // Reset the state of the given PlayerbotAI instance in the database
        void Reset(PlayerbotAI* botAI);

    private:
        // Helper method to save a key-value pair to the database for a given bot identified by guid
        void SaveValue(uint32 guid, std::string const key, std::string const value);
        // Helper method to format strategies as a string based on the type and a list of strategies
        std::string const FormatStrategies(std::string const type, std::vector<std::string> strategies);
};

// Define a macro for easier access to the singleton instance of PlayerbotDbStore
#define sPlayerbotDbStore PlayerbotDbStore::instance()

#endif  // End of header guard
