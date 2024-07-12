/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTSECURITY_H
#define _PLAYERBOT_PLAYERBOTSECURITY_H

#include "Common.h"
#include "ObjectGuid.h"

#include <map>

class Player;

// Enum representing different levels of security for player bot interactions
enum PlayerbotSecurityLevel : uint32
{
    PLAYERBOT_SECURITY_DENY_ALL     = 0,  // Deny all actions
    PLAYERBOT_SECURITY_TALK         = 1,  // Allow limited actions (e.g., talking)
    PLAYERBOT_SECURITY_INVITE       = 2,  // Allow invitations to groups
    PLAYERBOT_SECURITY_ALLOW_ALL    = 3   // Allow all actions
};

// Enum representing reasons for denying an action
enum DenyReason
{
    PLAYERBOT_DENY_NONE,
    PLAYERBOT_DENY_LOW_LEVEL,
    PLAYERBOT_DENY_GEARSCORE,
    PLAYERBOT_DENY_NOT_YOURS,
    PLAYERBOT_DENY_IS_BOT,
    PLAYERBOT_DENY_OPPOSING,
    PLAYERBOT_DENY_DEAD,
    PLAYERBOT_DENY_FAR,
    PLAYERBOT_DENY_INVITE,
    PLAYERBOT_DENY_FULL_GROUP,
    PLAYERBOT_DENY_NOT_LEADER,
    PLAYERBOT_DENY_IS_LEADER,
    PLAYERBOT_DENY_BG,
    PLAYERBOT_DENY_LFG
};

// Class handling the security levels for player bots
class PlayerbotSecurity
{
    public:
        // Constructor
        PlayerbotSecurity(Player* const bot);

        // Method to determine the security level for interactions from a specific player
        PlayerbotSecurityLevel LevelFor(Player* from, DenyReason* reason = nullptr, bool ignoreGroup = false);
        
        // Method to check if a player meets the required security level for an action
        bool CheckLevelFor(PlayerbotSecurityLevel level, bool silent, Player* from, bool ignoreGroup = false);

    private:
        Player* const bot;  // Pointer to the bot player
        uint32 account;     // Account ID of the bot player
        std::map<ObjectGuid, std::map<std::string, time_t> > whispers;  // Map to track whisper messages and their timestamps
};

#endif
