/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"
#include "Playerbots.h"
#include "Timer.h"

// Return the size of the actions array
uint32 NextAction::size(NextAction** actions)
{
    if (!actions) // If actions is null, return 0
        return 0;

    uint32 size = 0;
    for (size = 0; actions[size];) // Increment size until a null element is found
        ++size;

    return size; // Return the size of the array
}

// Clone the actions array
NextAction** NextAction::clone(NextAction** actions)
{
    if (!actions) // If actions is null, return null
        return nullptr;

    uint32 size = NextAction::size(actions); // Get the size of the actions array

    NextAction** res = new NextAction*[size + 1]; // Allocate memory for the cloned array
    for (uint32 i = 0; i < size; i++) // Copy each action
        res[i] = new NextAction(*actions[i]);

    res[size] = nullptr; // Set the last element to null

    return res; // Return the cloned array
}

// Merge two actions arrays
NextAction** NextAction::merge(NextAction** left, NextAction** right)
{
    uint32 leftSize = NextAction::size(left); // Get the size of the left array
    uint32 rightSize = NextAction::size(right); // Get the size of the right array

    NextAction** res = new NextAction*[leftSize + rightSize + 1]; // Allocate memory for the merged array

    for (uint32 i = 0; i < leftSize; i++) // Copy the left array
        res[i] = new NextAction(*left[i]);

    for (uint32 i = 0; i < rightSize; i++) // Copy the right array
        res[leftSize + i] = new NextAction(*right[i]);

    res[leftSize + rightSize] = nullptr; // Set the last element to null

    NextAction::destroy(left); // Destroy the left array
    NextAction::destroy(right); // Destroy the right array

    return res; // Return the merged array
}

// Create an array of actions from a variable argument list
NextAction** NextAction::array(uint32 nil, ...)
{
    va_list vl;
    va_start(vl, nil);

    uint32 size = 0;
    NextAction* cur = nullptr;
    do
    {
        cur = va_arg(vl, NextAction*); // Get the next argument
        ++size; // Increment size
    }
    while (cur); // Continue until a null argument is found

    va_end(vl);

    NextAction** res = new NextAction*[size]; // Allocate memory for the array
    va_start(vl, nil);
    for (uint32 i = 0; i < size; i++) // Copy each argument into the array
        res[i] = va_arg(vl, NextAction*);
    va_end(vl);

    return res; // Return the array
}

// Destroy the actions array
void NextAction::destroy(NextAction** actions)
{
    if (!actions) // If actions is null, return
        return;

    for (uint32 i=0; actions[i]; i++) // Delete each action
        delete actions[i];

    delete[] actions; // Delete the array
}

// Get the target value
Value<Unit*>* Action::GetTargetValue()
{
    return context->GetValue<Unit*>(GetTargetName()); // Return the target value from the context
}

// Get the target unit
Unit* Action::GetTarget()
{
    return GetTargetValue()->Get(); // Return the target unit
}

// Constructor for ActionBasket
ActionBasket::ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event) :
    action(action), relevance(relevance), skipPrerequisites(skipPrerequisites), event(event), created(getMSTime())
{
}

// Check if the action basket is expired
bool ActionBasket::isExpired(uint32 msecs)
{
    return getMSTime() - created >= msecs; // Return true if the time since creation is greater than or equal to msecs
}
