/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRIGGER_H
#define _PLAYERBOT_TRIGGER_H

#include "Common.h"
#include "Action.h"

class PlayerbotAI;
class Unit;

// Class definition for Trigger, inheriting from AiNamedObject
class Trigger : public AiNamedObject
{
    public:
        // Constructor for Trigger class
        Trigger(PlayerbotAI* botAI, std::string const name = "trigger", int32 checkInterval = 1);

        // Virtual destructor for Trigger class
        virtual ~Trigger() { }

        // Method to check if the trigger is active
        virtual Event Check();
        // Virtual method to handle external events with parameters
        virtual void ExternalEvent([[maybe_unused]] std::string const param, [[maybe_unused]] Player* owner = nullptr) { }
        // Virtual method to handle external events with packets
        virtual void ExternalEvent([[maybe_unused]] WorldPacket& packet, [[maybe_unused]] Player* owner = nullptr) { }
        // Virtual method to determine if the trigger is active
        virtual bool IsActive() { return false; }
        // Virtual method to get handlers for the trigger
        virtual NextAction** getHandlers() { return nullptr; }
        // Method to update the trigger
        void Update() { }
        // Virtual method to reset the trigger
        virtual void Reset() { }
        // Virtual method to get the target unit
        virtual Unit* GetTarget();
        // Virtual method to get the target value
        virtual Value<Unit*>* GetTargetValue();
        // Virtual method to get the target name
        virtual std::string const GetTargetName() { return "self target"; }

        // Method to determine if the trigger needs to be checked
        bool needCheck();

    protected:
        int32 checkInterval; // Interval between checks
        uint32 lastCheckTime; // Time of the last check
};

// Class definition for TriggerNode
class TriggerNode
{
    public:
        // Constructor for TriggerNode class
        TriggerNode(std::string const name, NextAction** handlers = nullptr) : trigger(nullptr), handlers(handlers), name(name) { } // reorder args - whipowill

        // Destructor for TriggerNode class
        virtual ~TriggerNode()
        {
            NextAction::destroy(handlers); // Destroy handlers
        }

        // Method to get the trigger
        Trigger* getTrigger() { return trigger; }
        // Method to set the trigger
        void setTrigger(Trigger* trigger) { this->trigger = trigger; }
        // Method to get the name of the trigger
        std::string const getName() { return name; }

        // Method to get handlers for the trigger
        NextAction** getHandlers()
        {
            return NextAction::merge(NextAction::clone(handlers), trigger->getHandlers());
        }

        // Method to get the relevance of the first handler
        float getFirstRelevance()
        {
            return handlers[0] ? handlers[0]->getRelevance() : -1;
        }

    private:
        Trigger* trigger; // Pointer to the trigger
        NextAction** handlers; // Array of handlers
        std::string const name; // Name of the trigger
};

#endif
