/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AIOBJECTCONTEXT_H
#define _PLAYERBOT_AIOBJECTCONTEXT_H

#include "Common.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIAware.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Value.h"

#include <sstream>
#include <string>

class PlayerbotAI;

class AiObjectContext : public PlayerbotAIAware
{
    public:
        // Constructor for AiObjectContext
        AiObjectContext(PlayerbotAI* botAI);
        // Destructor for AiObjectContext
        virtual ~AiObjectContext() { }

        // Get strategy by name
        virtual Strategy* GetStrategy(std::string const name);
        // Get sibling strategies by name
        virtual std::set<std::string> GetSiblingStrategy(std::string const name);
        // Get trigger by name
        virtual Trigger* GetTrigger(std::string const name);
        // Get action by name
        virtual Action* GetAction(std::string const name);
        // Get untyped value by name
        virtual UntypedValue* GetUntypedValue(std::string const name);

        // Get typed value by name
        template<class T>
        Value<T>* GetValue(std::string const name)
        {
            return dynamic_cast<Value<T>*>(GetUntypedValue(name));
        }

        // Get typed value by name and parameter
        template<class T>
        Value<T>* GetValue(std::string const name, std::string const param)
        {
            return GetValue<T>((std::string(name) + "::" + param));
        }

        // Get typed value by name and integer parameter
        template<class T>
        Value<T>* GetValue(std::string const name, int32 param)
        {
            std::ostringstream out;
            out << param;
            return GetValue<T>(name, out.str());
        }

        // Get all created values
        std::set<std::string> GetValues();
        // Get supported strategies
        std::set<std::string> GetSupportedStrategies();
        // Get supported actions
        std::set<std::string> GetSupportedActions();
        // Format values into a string
        std::string const FormatValues();

        // Update all contexts
        virtual void Update();
        // Reset all contexts
        virtual void Reset();
        // Add shared values context
        virtual void AddShared(NamedObjectContext<UntypedValue>* sharedValues);

        // Save all created values
        std::vector<std::string> Save();
        // Load values from data
        void Load(std::vector<std::string> data);

        std::vector<std::string> performanceStack;

    protected:
        NamedObjectContextList<Strategy> strategyContexts; // List of strategy contexts
        NamedObjectContextList<Action> actionContexts; // List of action contexts
        NamedObjectContextList<Trigger> triggerContexts; // List of trigger contexts
        NamedObjectContextList<UntypedValue> valueContexts; // List of value contexts
};

#endif
