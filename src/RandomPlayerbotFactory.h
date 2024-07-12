/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H
#define _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H

#include "Common.h"

#include <map>
#include <unordered_map>
#include <vector>

// Forward declarations for Player and WorldSession classes
class Player;
class WorldSession;

class RandomPlayerbotFactory
{
    public:
        // Constructor to initialize the factory with a specific account ID
        RandomPlayerbotFactory(uint32 accountId);
        
        // Virtual destructor for proper cleanup
        virtual ~RandomPlayerbotFactory() { }

        // Method to create a random bot for a given session and class, with a set of possible names
        Player* CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<uint8, std::vector<std::string>> names);
        
        // Static method to initiate the creation of multiple random bots
        static void CreateRandomBots();
        
        // Static method to create random guilds
        static void CreateRandomGuilds();
        
        // Static method to create random arena teams
        static void CreateRandomArenaTeams();
        
        // Static method to generate a random guild name
        static std::string const CreateRandomGuildName();

    private:
        // Method to create a random bot name based on gender
        std::string const CreateRandomBotName(uint8 gender);
        
        // Static method to generate a random arena team name
        static std::string const CreateRandomArenaTeamName();

        // Account ID associated with the factory instance
        uint32 accountId;
        
        // Static map to hold available races for each class
        static std::map<uint8, std::vector<uint8>> availableRaces;
};

#endif
