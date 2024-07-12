/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AIFACTORY_H    // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_AIFACTORY_H

#include "Common.h"               // Include common definitions and utilities

#include <map>                    // Include the map container from the standard library

// Forward declarations of classes used in the AiFactory class
class AiObjectContext;
class Engine;
class Player;
class PlayerbotAI;

// Enumeration for bot roles
enum BotRoles : uint8;

// AiFactory class definition
class AiFactory
{
    public:
        // Static method to create an AiObjectContext based on the player's class and bot AI
        static AiObjectContext* createAiObjectContext(Player* player, PlayerbotAI* botAI);

        // Static method to create a combat engine for the player
	    static Engine* createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext);

        // Static method to create a non-combat engine for the player
	    static Engine* createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext);

        // Static method to create a dead engine for the player
        static Engine* createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aibjectContext);

        // Static method to add default non-combat strategies to the engine
        static void AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine);

        // Static method to add default dead strategies to the engine
        static void AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine);

        // Static method to add default combat strategies to the engine
        static void AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine);

        // Static method to get the player's specialization tab
        static uint8 GetPlayerSpecTab(Player* player);

        // Static method to get the player's spec tabs and their points
        static std::map<uint8, uint32> GetPlayerSpecTabs(Player* player);

        // Static method to get the player's roles based on their class and spec
        static BotRoles GetPlayerRoles(Player* player);

        // Static method to get the player's spec name based on their class and spec tab
        static std::string GetPlayerSpecName(Player* player);
};

#endif  // End of header guard
