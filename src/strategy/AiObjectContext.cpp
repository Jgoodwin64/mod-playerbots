/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AiObjectContext.h"
#include "StrategyContext.h"
#include "ActionContext.h"
#include "ChatActionContext.h"
#include "WorldPacketActionContext.h"
#include "ChatTriggerContext.h"
#include "TriggerContext.h"
#include "SharedValueContext.h"
#include "WorldPacketTriggerContext.h"
#include "ValueContext.h"
#include "Playerbots.h"
#include "raids/RaidTriggerContext.h"
#include "raids/RaidActionContext.h"
#include "raids/RaidStrategyContext.h"
#include "raids/naxxramas/RaidNaxxActionContext.h"
#include "raids/naxxramas/RaidNaxxTriggerContext.h"

// Constructor for AiObjectContext
AiObjectContext::AiObjectContext(PlayerbotAI* botAI) : PlayerbotAIAware(botAI)
{
    strategyContexts.Add(new StrategyContext()); // Add strategy context
    strategyContexts.Add(new MovementStrategyContext()); // Add movement strategy context
    strategyContexts.Add(new AssistStrategyContext()); // Add assist strategy context
    strategyContexts.Add(new QuestStrategyContext()); // Add quest strategy context
    strategyContexts.Add(new RaidStrategyContext()); // Add raid strategy context
    
    actionContexts.Add(new ActionContext()); // Add action context
    actionContexts.Add(new ChatActionContext()); // Add chat action context
    actionContexts.Add(new WorldPacketActionContext()); // Add world packet action context
    actionContexts.Add(new RaidActionContext()); // Add raid action context
    actionContexts.Add(new RaidNaxxActionContext()); // Add raid Naxxramas action context

    triggerContexts.Add(new TriggerContext()); // Add trigger context
    triggerContexts.Add(new ChatTriggerContext()); // Add chat trigger context
    triggerContexts.Add(new WorldPacketTriggerContext()); // Add world packet trigger context
    triggerContexts.Add(new RaidTriggerContext()); // Add raid trigger context
    triggerContexts.Add(new RaidNaxxTriggerContext()); // Add raid Naxxramas trigger context

    valueContexts.Add(new ValueContext()); // Add value context

    valueContexts.Add(sSharedValueContext); // Add shared value context
}

// Update all contexts
void AiObjectContext::Update()
{
    strategyContexts.Update(); // Update strategy contexts
    triggerContexts.Update(); // Update trigger contexts
    actionContexts.Update(); // Update action contexts
    valueContexts.Update(); // Update value contexts
}

// Reset all contexts
void AiObjectContext::Reset()
{
    strategyContexts.Reset(); // Reset strategy contexts
    triggerContexts.Reset(); // Reset trigger contexts
    actionContexts.Reset(); // Reset action contexts
    valueContexts.Reset(); // Reset value contexts
}

// Save all created values
std::vector<std::string> AiObjectContext::Save()
{
    std::vector<std::string> result;

    std::set<std::string> names = valueContexts.GetCreated(); // Get created values
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i); // Get untyped value
        if (!value) // Skip if value is null
            continue;

        std::string const data = value->Save(); // Save value
        if (data == "?") // Skip if data is "?"
            continue;

        std::string const name = *i;
        std::ostringstream out;
        out << name;

        out << ">" << data; // Format value for saving
        result.push_back(out.str()); // Add to result
    }

    return result; // Return saved values
}

// Load values from data
void AiObjectContext::Load(std::vector<std::string> data)
{
    for (std::vector<std::string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        std::string const row = *i;
        std::vector<std::string> parts = split(row, '>'); // Split row into parts
        if (parts.size() != 2) // Skip if parts size is not 2
            continue;

        std::string const name = parts[0];
        std::string const text = parts[1];

        UntypedValue* value = GetUntypedValue(name); // Get untyped value
        if (!value) // Skip if value is null
            continue;

        value->Load(text); // Load value
    }
}

// Get strategy by name
Strategy* AiObjectContext::GetStrategy(std::string const name)
{
    return strategyContexts.GetContextObject(name, botAI); // Return strategy context object
}

// Get sibling strategies by name
std::set<std::string> AiObjectContext::GetSiblingStrategy(std::string const name)
{
    return strategyContexts.GetSiblings(name); // Return sibling strategies
}

// Get trigger by name
Trigger* AiObjectContext::GetTrigger(std::string const name)
{
    return triggerContexts.GetContextObject(name, botAI); // Return trigger context object
}

// Get action by name
Action* AiObjectContext::GetAction(std::string const name)
{
    return actionContexts.GetContextObject(name, botAI); // Return action context object
}

// Get untyped value by name
UntypedValue* AiObjectContext::GetUntypedValue(std::string const name)
{
    return valueContexts.GetContextObject(name, botAI); // Return untyped value context object
}

// Get created values
std::set<std::string> AiObjectContext::GetValues()
{
    return valueContexts.GetCreated(); // Return created values
}

// Get supported strategies
std::set<std::string> AiObjectContext::GetSupportedStrategies()
{
    return strategyContexts.supports(); // Return supported strategies
}

// Get supported actions
std::set<std::string> AiObjectContext::GetSupportedActions()
{
    return actionContexts.supports(); // Return supported actions
}

// Format values into a string
std::string const AiObjectContext::FormatValues()
{
    std::ostringstream out;
    std::set<std::string> names = valueContexts.GetCreated(); // Get created values
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i, out << "|")
    {
        UntypedValue* value = GetUntypedValue(*i); // Get untyped value
        if (!value) // Skip if value is null
            continue;

        std::string const text = value->Format(); // Format value
        if (text == "?") // Skip if text is "?"
            continue;

        out << "{" << *i << "=" << text << "}"; // Add formatted value to output
    }

    return out.str(); // Return formatted values
}

// Add shared values context
void AiObjectContext::AddShared(NamedObjectContext<UntypedValue>* sharedValues)
{
    valueContexts.Add(sharedValues); // Add shared values context
}
