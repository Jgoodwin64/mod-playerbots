/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STRATEGY_H
#define _PLAYERBOT_STRATEGY_H

// Include necessary headers
#include "Action.h"
#include "NamedObjectContext.h"
#include "Multiplier.h"
#include "Trigger.h"
#include "PlayerbotAIAware.h"

// Define different types of strategies using an enumeration
enum StrategyType : uint32
{
    STRATEGY_TYPE_GENERIC   = 0,    // Generic strategy
    STRATEGY_TYPE_COMBAT    = 1,    // Combat strategy
    STRATEGY_TYPE_NONCOMBAT = 2,    // Non-combat strategy
    STRATEGY_TYPE_TANK      = 4,    // Tank strategy
    STRATEGY_TYPE_DPS       = 8,    // DPS (Damage Per Second) strategy
    STRATEGY_TYPE_HEAL      = 16,   // Healing strategy
    STRATEGY_TYPE_RANGED    = 32,   // Ranged strategy
    STRATEGY_TYPE_MELEE     = 64    // Melee strategy
};

// Priority levels for actions, these values were commented out but retained for reference
// enum ActionPriority
// {
// 	ACTION_IDLE = 0,
// 	ACTION_DEFAULT = 5,
// 	ACTION_NORMAL = 10,
// 	ACTION_HIGH = 20,
// 	ACTION_MOVE = 30,
// 	ACTION_INTERRUPT = 40,
// 	ACTION_DISPEL = 50,
// 	ACTION_RAID = 60,
// 	ACTION_LIGHT_HEAL = 10,
// 	ACTION_MEDIUM_HEAL = 20,
// 	ACTION_CRITICAL_HEAL = 30,
// 	ACTION_EMERGENCY = 90
// };

// Action priorities as static float values
static float ACTION_IDLE = 0.0f;
static float ACTION_DEFAULT = 5.0f;
static float ACTION_NORMAL = 10.0f;
static float ACTION_HIGH = 20.0f;
static float ACTION_MOVE = 30.0f;
static float ACTION_INTERRUPT = 40.0f;
static float ACTION_DISPEL = 50.0f;
static float ACTION_RAID = 60.0f;
static float ACTION_LIGHT_HEAL = 10.0f;
static float ACTION_MEDIUM_HEAL = 20.0f;
static float ACTION_CRITICAL_HEAL = 30.0f;
static float ACTION_EMERGENCY = 90.0f;

// The Strategy class, derived from PlayerbotAIAware
class Strategy : public PlayerbotAIAware
{
    public:
        // Constructor
        Strategy(PlayerbotAI* botAI);
        
        // Virtual destructor
        virtual ~Strategy() { }

        // Virtual function to get default actions, can be overridden in derived classes
        virtual NextAction** getDefaultActions() { return nullptr; }

        // Virtual function to initialize triggers, can be overridden in derived classes
        virtual void InitTriggers([[maybe_unused]] std::vector<TriggerNode*> &triggers) { }

        // Virtual function to initialize multipliers, can be overridden in derived classes
        virtual void InitMultipliers([[maybe_unused]] std::vector<Multiplier*> &multipliers) { }

        // Pure virtual function to get the name of the strategy, must be implemented in derived classes
        virtual std::string const getName() = 0;

        // Virtual function to get the type of strategy, can be overridden in derived classes
        virtual uint32 GetType() const { return STRATEGY_TYPE_GENERIC; }

        // Function to get a specific action node by name
        virtual ActionNode* GetAction(std::string const name);

        // Function to update the strategy, can be expanded in derived classes
        void Update() { }

        // Function to reset the strategy, can be expanded in derived classes
        void Reset() { }

    protected:
        // List of action node factories
        NamedObjectFactoryList<ActionNode> actionNodeFactories;
};

#endif
