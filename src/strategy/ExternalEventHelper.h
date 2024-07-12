/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_EXTERNALEVENTHELPER_H
#define _PLAYERBOT_EXTERNALEVENTHELPER_H

#include "Common.h"

#include <map>

class AiObjectContext;
class Player;
class WorldPacket;

class ExternalEventHelper
{
    public:
        ExternalEventHelper(AiObjectContext* aiObjectContext) : aiObjectContext(aiObjectContext) { } // Constructor for ExternalEventHelper

        bool ParseChatCommand(std::string const command, Player* owner = nullptr); // Parse a chat command
        void HandlePacket(std::map<uint16, std::string>& handlers, WorldPacket const& packet, Player* owner = nullptr); // Handle a packet
        bool HandleCommand(std::string const name, std::string const param, Player* owner = nullptr); // Handle a command

    private:
        AiObjectContext* aiObjectContext; // AI object context
};

#endif
