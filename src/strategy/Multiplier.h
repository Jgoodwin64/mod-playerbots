/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MULTIPLIER_H
#define _PLAYERBOT_MULTIPLIER_H

#include "AiObject.h"

// Forward declarations
class Action;
class PlayerbotAI;

// Class representing a multiplier used in AI calculations
class Multiplier : public AiNamedObject
{
    public:
        // Constructor initializing the multiplier with a bot AI and a name
        Multiplier(PlayerbotAI* botAI, std::string const name) : AiNamedObject(botAI, name)  {}
        
        // Virtual destructor for the multiplier class
        virtual ~Multiplier() { }

        // Virtual function to get the value of the multiplier, defaulting to 1.0
        virtual float GetValue([[maybe_unused]] Action* action) { return 1.0f; }
};

#endif
