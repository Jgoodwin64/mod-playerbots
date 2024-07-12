/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PassiveMultiplier.h"
#include "Action.h"
#include "AiObjectContext.h"

// Initialize static members for allowed actions and parts
std::vector<std::string> PassiveMultiplier::allowedActions;
std::vector<std::string> PassiveMultiplier::allowedParts;

// Constructor initializing the passive multiplier with a bot AI
PassiveMultiplier::PassiveMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "passive")
{
    // Populate the allowed actions if the list is empty
    if (allowedActions.empty())
    {
        allowedActions.push_back("co");
        allowedActions.push_back("nc");
        allowedActions.push_back("reset botAI");
        allowedActions.push_back("check mount state");
    }

    // Populate the allowed parts if the list is empty
    if (allowedParts.empty())
    {
        allowedParts.push_back("follow");
        allowedParts.push_back("stay");
        allowedParts.push_back("chat shortcut");
    }
}

// Function to get the value of the passive multiplier for a given action
float PassiveMultiplier::GetValue(Action* action)
{
    // Return default value if the action is null
    if (!action)
        return 1.0f;

    // Get the name of the action
    std::string const name = action->getName();

    // Check if the action name is in the allowed actions list
    for (std::vector<std::string>::iterator i = allowedActions.begin(); i != allowedActions.end(); i++)
    {
        if (name == *i)
            return 1.0f;
    }

    // Check if the action name contains any allowed parts
    for (std::vector<std::string>::iterator i = allowedParts.begin(); i != allowedParts.end(); i++)
    {
        if (name.find(*i) != std::string::npos)
            return 1.0f;
    }

    // Return 0 if the action is not allowed
    return 0;
}
