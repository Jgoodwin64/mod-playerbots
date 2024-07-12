/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Strategy.h"
#include "Playerbots.h"

// Class definition for ActionNodeFactoryInternal, inheriting from NamedObjectFactory<ActionNode>
class ActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
    public:
        // Constructor for ActionNodeFactoryInternal
        ActionNodeFactoryInternal()
        {
            // Initializing action node creators
            creators["melee"] = &melee; // Assigning melee action node creator
            creators["healthstone"] = &healthstone; // Assigning healthstone action node creator
            creators["be near"] = &follow_master_random; // Assigning follow master random action node creator
            creators["attack anything"] = &attack_anything; // Assigning attack anything action node creator
            creators["move random"] = &move_random; // Assigning move random action node creator
            creators["move to loot"] = &move_to_loot; // Assigning move to loot action node creator
            creators["food"] = &food; // Assigning food action node creator
            creators["drink"] = &drink; // Assigning drink action node creator
            creators["mana potion"] = &mana_potion; // Assigning mana potion action node creator
            creators["healing potion"] = &healing_potion; // Assigning healing potion action node creator
            creators["flee"] = &flee; // Assigning flee action node creator
        }

    private:
        // Static method to create melee action node
        static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("melee",
                /*P*/ nullptr, // No predecessor action
                /*A*/ nullptr, // No alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create healthstone action node
        static ActionNode* healthstone([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("healthstone",
                /*P*/ nullptr, // No predecessor action
                /*A*/ NextAction::array(0, new NextAction("healing potion"), nullptr), // Healing potion as an alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create follow master random action node
        static ActionNode* follow_master_random([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("be near",
                /*P*/ nullptr, // No predecessor action
                /*A*/ NextAction::array(0, new NextAction("follow"), nullptr), // Follow as an alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create attack anything action node
        static ActionNode* attack_anything([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("attack anything",
                /*P*/ nullptr, // No predecessor action
                /*A*/ nullptr, // No alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create move random action node
        static ActionNode* move_random([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("move random",
                /*P*/ nullptr, // No predecessor action
                /*A*/ NextAction::array(0, new NextAction("stay line"), nullptr), // Stay line as an alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create move to loot action node
        static ActionNode* move_to_loot([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("move to loot",
                /*P*/ nullptr, // No predecessor action
                /*A*/ nullptr, // No alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create food action node
        static ActionNode* food([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("food",
                /*P*/ nullptr, // No predecessor action
                /*A*/ nullptr, // No alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create drink action node
        static ActionNode* drink([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("drink",
                /*P*/ nullptr, // No predecessor action
                /*A*/ nullptr, // No alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create mana potion action node
        static ActionNode* mana_potion([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("mana potion",
                /*P*/ nullptr, // No predecessor action
                /*A*/ NextAction::array(0, new NextAction("drink"), nullptr), // Drink as an alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create healing potion action node
        static ActionNode* healing_potion([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("healing potion",
                /*P*/ nullptr, // No predecessor action
                /*A*/ NextAction::array(0, new NextAction("food"), nullptr), // Food as an alternate action
                /*C*/ nullptr); // No condition
        }

        // Static method to create flee action node
        static ActionNode* flee([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("flee",
                /*P*/ nullptr, // No predecessor action
                /*A*/ nullptr, // No alternate action
                /*C*/ nullptr); // No condition
        }
};

// Constructor for Strategy class
Strategy::Strategy(PlayerbotAI* botAI) : PlayerbotAIAware(botAI)
{
    // Adding action node factory to the list of factories
    actionNodeFactories.Add(new ActionNodeFactoryInternal());
}

// Method to get action node by name
ActionNode* Strategy::GetAction(std::string const name)
{
    // Returning action node from context by name
    return actionNodeFactories.GetContextObject(name, botAI);
}
