/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PASSIVEMULTIPLIER_H
#define _PLAYERBOT_PASSIVEMULTIPLIER_H

#include "Multiplier.h"

#include <vector>

// Forward declarations
class Action;
class PlayerbotAI;

// Class representing a passive multiplier for actions
class PassiveMultiplier : public Multiplier
{
    public:
        // Constructor initializing with a bot AI
        PassiveMultiplier(PlayerbotAI* botAI);

        // Function to get the value of the multiplier for a given action
        float GetValue(Action* action) override;

    private:
        static std::vector<std::string> allowedActions;  // Vector of allowed action names
        static std::vector<std::string> allowedParts;    // Vector of allowed action name parts
};

#endif
