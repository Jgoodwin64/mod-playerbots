/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Queue.h"
#include "AiObjectContext.h"
#include "Log.h"
#include "PlayerbotAIConfig.h"

// Function to push an action onto the queue
void Queue::Push(ActionBasket *action)
{
    // Check if the action is valid
    if (action)
    {
        // Iterate through the existing actions in the queue
        for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
        {
            ActionBasket* basket = *iter;
            // If the action already exists in the queue, update its relevance
            if (action->getAction()->getName() == basket->getAction()->getName())
            {
                // Update the relevance if the new action has higher relevance
                if (basket->getRelevance() < action->getRelevance())
                    basket->setRelevance(action->getRelevance());

                // Delete the action node and action to avoid memory leak
                if (ActionNode* actionNode = action->getAction())
                    delete actionNode;

                delete action;

                return;
            }
        }

        // Add the action to the end of the queue
        actions.push_back(action);
    }
}

// Function to pop the highest relevance action from the queue
ActionNode* Queue::Pop()
{
    float max = -1;
    ActionBasket* selection = nullptr;

    // Iterate through the actions in the queue to find the highest relevance action
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }

    // If a selection is found, remove it from the queue and return the action node
    if (selection != nullptr)
    {
        ActionNode* action = selection->getAction();
        actions.remove(selection);
        delete selection;
        return action;
    }

    // Return nullptr if no action is found
    return nullptr;
}

// Function to peek at the highest relevance action in the queue without removing it
ActionBasket* Queue::Peek()
{
    float max = -1;
    ActionBasket* selection = nullptr;

    // Iterate through the actions in the queue to find the highest relevance action
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }

    // Return the selected action basket
    return selection;
}

// Function to get the size of the queue
uint32 Queue::Size()
{
    return actions.size();
}

// Function to remove expired actions from the queue
void Queue::RemoveExpired()
{
    std::list<ActionBasket*> expired;

    // Iterate through the actions in the queue to find expired actions
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        // Check if the action is expired based on the configuration
        if (sPlayerbotAIConfig->expireActionTime && basket->isExpired(sPlayerbotAIConfig->expireActionTime))
            expired.push_back(basket);
    }

    // Remove and delete the expired actions
    for (std::list<ActionBasket*>::iterator iter = expired.begin(); iter != expired.end(); iter++)
    {
        ActionBasket* basket = *iter;
        actions.remove(basket);

        if (ActionNode* action = basket->getAction())
        {
            delete action;
        }

        delete basket;
    }
}
