/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ENGINE_H
#define _PLAYERBOT_ENGINE_H

#include "Multiplier.h"
#include "Queue.h"
#include "PlayerbotAIAware.h"
#include "Strategy.h"
#include "Trigger.h"

#include <map>

class Action; // Forward declaration of the Action class
class ActionNode; // Forward declaration of the ActionNode class
class AiObjectContext; // Forward declaration of the AiObjectContext class
class Event; // Forward declaration of the Event class
class NextAction; // Forward declaration of the NextAction class
class PlayerbotAI; // Forward declaration of the PlayerbotAI class

enum ActionResult
{
    ACTION_RESULT_UNKNOWN, // Action result is unknown
    ACTION_RESULT_OK, // Action was successful
    ACTION_RESULT_IMPOSSIBLE, // Action is impossible
    ACTION_RESULT_USELESS, // Action is useless
    ACTION_RESULT_FAILED // Action failed
};

class ActionExecutionListener
{
    public:
        virtual ~ActionExecutionListener() { }; // Virtual destructor

        virtual bool Before(Action* action, Event event) = 0; // Pure virtual method executed before action
        virtual bool AllowExecution(Action* action, Event event) = 0; // Pure virtual method to allow action execution
        virtual void After(Action* action, bool executed, Event event) = 0; // Pure virtual method executed after action
        virtual bool OverrideResult(Action* action, bool executed, Event event) = 0; // Pure virtual method to override action result
};

class ActionExecutionListeners : public ActionExecutionListener
{
    public:
        virtual ~ActionExecutionListeners(); // Virtual destructor

        bool Before(Action* action, Event event) override; // Method executed before action
        bool AllowExecution(Action* action, Event event) override; // Method to allow action execution
        void After(Action* action, bool executed, Event event) override; // Method executed after action
        bool OverrideResult(Action* action, bool executed, Event event) override; // Method to override action result

        void Add(ActionExecutionListener* listener) // Add a listener
        {
            listeners.push_back(listener);
        }

        void Remove(ActionExecutionListener* listener) // Remove a listener
        {
            listeners.remove(listener);
        }

    private:
        std::list<ActionExecutionListener*> listeners; // List of listeners
};

class Engine : public PlayerbotAIAware
{
    public:
        Engine(PlayerbotAI* botAI, AiObjectContext* factory); // Constructor

	    void Init(); // Initialize the engine
        void addStrategy(std::string const name); // Add a strategy
		void addStrategies(std::string first, ...); // Add multiple strategies
        bool removeStrategy(std::string const name); // Remove a strategy
        bool HasStrategy(std::string const name); // Check if a strategy exists
        void removeAllStrategies(); // Remove all strategies
        void toggleStrategy(std::string const name); // Toggle a strategy
        std::string const ListStrategies(); // List all strategies
        std::vector<std::string> GetStrategies(); // Get all strategies
		bool ContainsStrategy(StrategyType type); // Check if strategy of a type exists
		void ChangeStrategy(std::string const names); // Change strategy
        std::string const GetLastAction() { return lastAction; } // Get last action

	    virtual bool DoNextAction(Unit*, uint32 depth = 0, bool minimal = false); // Perform next action
	    ActionResult ExecuteAction(std::string const name, Event event = Event(), std::string const qualifier = ""); // Execute an action

        void AddActionExecutionListener(ActionExecutionListener* listener) // Add an action execution listener
        {
            actionExecutionListeners.Add(listener);
        }

        void removeActionExecutionListener(ActionExecutionListener* listener) // Remove an action execution listener
        {
            actionExecutionListeners.Remove(listener);
        }

	    virtual ~Engine(void); // Destructor

        bool testMode; // Test mode flag

    private:
        bool MultiplyAndPush(NextAction** actions, float forceRelevance, bool skipPrerequisites, Event event, const char* pushType); // Multiply and push actions
        void Reset(); // Reset the engine
        void ProcessTriggers(bool minimal); // Process triggers
        void PushDefaultActions(); // Push default actions
        void PushAgain(ActionNode* actionNode, float relevance, Event event); // Push action again
        ActionNode* CreateActionNode(std::string const name); // Create action node
        Action* InitializeAction(ActionNode* actionNode); // Initialize action
        bool ListenAndExecute(Action* action, Event event); // Listen and execute action

        void LogAction(char const* format, ...); // Log action
        void LogValues(); // Log values

        ActionExecutionListeners actionExecutionListeners; // Action execution listeners

    protected:
	    Queue queue; // Action queue
	    std::vector<TriggerNode*> triggers; // List of triggers
        std::vector<Multiplier*> multipliers; // List of multipliers
        AiObjectContext* aiObjectContext; // AI object context
        std::map<std::string, Strategy*> strategies; // Map of strategies
        float lastRelevance; // Last relevance value
        std::string lastAction; // Last action performed
};

#endif
