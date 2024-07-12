/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CustomStrategy.h"
#include "Playerbots.h"

#include <regex>

// Initialize the static member actionLinesCache
std::map<std::string, std::string> CustomStrategy::actionLinesCache;

// Converts a string action to a NextAction object
NextAction* toNextAction(std::string const action)
{
    std::vector<std::string> tokens = split(action, '!');
    if (tokens.size() == 2 && !tokens[0].empty())
        return new NextAction(tokens[0], atof(tokens[1].c_str()));
    else if (tokens.size() == 1 && !tokens[0].empty())
        return new NextAction(tokens[0], ACTION_NORMAL);

    // Log error if the action string is invalid
    LOG_ERROR("playerbots", "Invalid action {}", action.c_str());
    return nullptr;
}

// Converts a string of actions to an array of NextAction pointers
NextAction** toNextActionArray(std::string const actions)
{
    std::vector<std::string> tokens = split(actions, ',');
    NextAction** res = new NextAction*[tokens.size() + 1];

    uint32 index = 0;
    for (std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        if (NextAction* na = toNextAction(*i))
            res[index++] = na;
    }

    // Mark the end of the array with a nullptr
	res[index++] = nullptr;
    return res;
}

// Converts a string action line to a TriggerNode object
TriggerNode* toTriggerNode(std::string const actionLine)
{
    std::vector<std::string> tokens = split(actionLine, '>');
    if (tokens.size() == 2)
        return new TriggerNode(tokens[0], toNextActionArray(tokens[1]));

    // Log error if the action line string is invalid
    LOG_ERROR("playerbots", "Invalid action line {}", actionLine.c_str());
    return nullptr;
}

// Constructor for CustomStrategy
CustomStrategy::CustomStrategy(PlayerbotAI* botAI) : Strategy(botAI), Qualified()
{
}

// Initializes the triggers for the strategy
void CustomStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    if (actionLines.empty())
    {
        if (actionLinesCache[qualifier].empty())
        {
            // Load action lines from the database for the bot
            LoadActionLines((uint32)botAI->GetBot()->GetGUID().GetCounter());
            if (actionLines.empty())
                LoadActionLines(0); // Load default action lines if specific ones are not found
        }
        else
        {
            std::vector<std::string> tokens = split(actionLinesCache[qualifier], '\n');
            std::regex tpl("\\(nullptr,\\s*'.+',\\s*'(.+)'\\)(,|;)");
            for (std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
            {
                std::string const line = *i;
                for (std::sregex_iterator j = std::sregex_iterator(line.begin(), line.end(), tpl); j != std::sregex_iterator(); ++j)
                {
                    std::smatch match = *j;
                    std::string const actionLine = match[1].str();
                    if (!actionLine.empty())
                        actionLines.push_back(actionLine);
                }
            }
        }
    }

    for (std::vector<std::string>::iterator i = actionLines.begin(); i != actionLines.end(); ++i)
    {
        if (TriggerNode* tn = toTriggerNode(*i))
            triggers.push_back(tn);
    }
}

// Loads the action lines for the given owner from the database
void CustomStrategy::LoadActionLines(uint32 owner)
{
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER_AND_NAME);
    stmt->SetData(0, owner);
    stmt->SetData(1, qualifier);
    PreparedQueryResult result = PlayerbotsDatabase.Query(stmt);
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            std::string const action = fields[1].Get<std::string>();
            actionLines.push_back(action);
        }
        while (result->NextRow());
    }
}

// Resets the strategy by clearing action lines and cache
void CustomStrategy::Reset()
{
    actionLines.clear();
    actionLinesCache[qualifier].clear();
}
