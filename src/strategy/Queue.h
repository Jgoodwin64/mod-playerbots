/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUEUE_H
#define _PLAYERBOT_QUEUE_H

#include "Common.h"
#include "Action.h"

// Class representing a queue for actions
class Queue
{
    public:
        // Constructor
        Queue(void) { }
        
        // Destructor
        ~Queue(void) { }

        // Function to push an action onto the queue
        void Push(ActionBasket *action);
        
        // Function to pop the highest relevance action from the queue
        ActionNode* Pop();
        
        // Function to peek at the highest relevance action in the queue without removing it
        ActionBasket* Peek();
        
        // Function to get the size of the queue
        uint32 Size();
        
        // Function to remove expired actions from the queue
        void RemoveExpired();

    private:
        std::list<ActionBasket*> actions;  // List to store actions in the queue
};

#endif
