/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACTION_H
#define _PLAYERBOT_ACTION_H

#include "AiObject.h"
#include "Common.h"
#include "Event.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class NextAction
{
    public:
        // Constructor with name and relevance
        NextAction(std::string const name, float relevance = 0.0f) : relevance(relevance), name(name) { } // Name after relevance - whipowill
        // Copy constructor
        NextAction(NextAction const& o) : relevance(o.relevance), name(o.name) { } // Name after relevance - whipowill

        // Get the name of the action
        std::string const getName() { return name; }
        // Get the relevance of the action
        float getRelevance() {return relevance;}

        // Get the size of the actions array
        static uint32 size(NextAction** actions);
        // Clone the actions array
        static NextAction** clone(NextAction** actions);
        // Merge two actions arrays
        static NextAction** merge(NextAction** what, NextAction** with);
        // Create an array of actions from a variable argument list
        static NextAction** array(uint32 nil,...);
        // Destroy the actions array
        static void destroy(NextAction** actions);

    private:
        float relevance;
        std::string const name;
};

class Action : public AiNamedObject
{
    public:
        // Enum for action threat types
        enum class ActionThreatType
        {
            None      = 0,
            Single    = 1,
            Aoe       = 2
        };

        // Constructor with botAI and name
        Action(PlayerbotAI* botAI, std::string const name = "action") : AiNamedObject(botAI, name), verbose(false) { } // Verbose after AiNamedObject - whipowill
        // Destructor
        virtual ~Action(void) { }

        // Execute the action
        virtual bool Execute([[maybe_unused]] Event event) { return true; }
        // Check if the action is possible
        virtual bool isPossible() { return true; }
        // Check if the action is useful
        virtual bool isUseful() { return true; }
        // Get the prerequisites of the action
        virtual NextAction** getPrerequisites() { return nullptr; }
        // Get the alternatives of the action
        virtual NextAction** getAlternatives() { return nullptr; }
        // Get the continuers of the action
        virtual NextAction** getContinuers() { return nullptr; }
        // Get the threat type of the action
        virtual ActionThreatType getThreatType() { return ActionThreatType::None; }
        // Update the action
        void Update() { }
        // Reset the action
        void Reset() { }
        // Get the target unit
        virtual Unit* GetTarget();
        // Get the target value
        virtual Value<Unit*>* GetTargetValue();
        // Get the target name
        virtual std::string const GetTargetName() { return "self target"; }
        // Make the action verbose
        void MakeVerbose() { verbose = true; }
        // Set the relevance of the action
        void setRelevance(uint32 relevance1) { relevance = relevance1; };
        // Get the relevance of the action
        virtual float getRelevance() { return relevance; }

    protected:
        bool verbose;
        float relevance = 0;
};

class ActionNode
{
    public:
        // Constructor with name, prerequisites, alternatives, and continuers
        ActionNode(std::string const name, NextAction** prerequisites = nullptr, NextAction** alternatives = nullptr, NextAction** continuers = nullptr) :
            name(name), action(nullptr), continuers(continuers), alternatives(alternatives), prerequisites(prerequisites) { } // Reorder arguments - whipowill

        // Destructor
        virtual ~ActionNode()
        {
            NextAction::destroy(prerequisites);
            NextAction::destroy(alternatives);
            NextAction::destroy(continuers);
        }

        // Get the action
        Action* getAction() { return action; }
        // Set the action
        void setAction(Action* action) { this->action = action; }
        // Get the name of the action node
        std::string const getName() { return name; }

        // Get the continuers of the action node
        NextAction** getContinuers() { return NextAction::merge(NextAction::clone(continuers), action->getContinuers()); }
        // Get the alternatives of the action node
        NextAction** getAlternatives() { return NextAction::merge(NextAction::clone(alternatives), action->getAlternatives()); }
        // Get the prerequisites of the action node
        NextAction** getPrerequisites() { return NextAction::merge(NextAction::clone(prerequisites), action->getPrerequisites()); }

    private:
        std::string const name;
        Action* action;
        NextAction** continuers;
        NextAction** alternatives;
        NextAction** prerequisites;
};

class ActionBasket
{
    public:
        // Constructor for ActionBasket
        ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event);

        // Destructor
        virtual ~ActionBasket(void) { }

        // Get the relevance of the action basket
        float getRelevance() {return relevance;}
        // Get the action node
        ActionNode* getAction() {return action;}
        // Get the event of the action basket
        Event getEvent() { return event; }
        // Check if prerequisites should be skipped
        bool isSkipPrerequisites() { return skipPrerequisites; }
        // Amend the relevance by a factor
        void AmendRelevance(float k) { relevance *= k; }
        // Set the relevance of the action basket
        void setRelevance(float relevance) { this->relevance = relevance; }
        // Check if the action basket is expired
        bool isExpired(uint32 msecs);

    private:
        ActionNode* action;
        float relevance;
        bool skipPrerequisites;
        Event event;
        uint32 created;
};

#endif
