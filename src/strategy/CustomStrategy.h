/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CUSTOMSTRATEGY_H
#define _PLAYERBOT_CUSTOMSTRATEGY_H

#include "Strategy.h"

#include <map>

class PlayerbotAI;

class CustomStrategy : public Strategy, public Qualified
{
    public:
        // Constructor for CustomStrategy
        CustomStrategy(PlayerbotAI* botAI);

        // Initializes the triggers for the strategy
        void InitTriggers(std::vector<TriggerNode*> &triggers) override;
        
        // Gets the name of the strategy
        std::string const getName() override { return std::string("custom::" + qualifier); }
        
        // Resets the strategy
        void Reset();

        // Cache for action lines
        static std::map<std::string, std::string> actionLinesCache;

    private:
        // Stores the action lines for the strategy
        std::vector<std::string> actionLines;
        
        // Loads the action lines for the given owner
        void LoadActionLines(uint32 owner);
};

#endif
