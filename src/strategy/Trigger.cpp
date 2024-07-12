/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Trigger.h"
#include "Event.h"
#include "Playerbots.h"
#include "Timer.h"

// Constructor for Trigger class
Trigger::Trigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    AiNamedObject(botAI, name), checkInterval(checkInterval == 1 ? 1 : (checkInterval < 100 ? checkInterval * 1000 : checkInterval)), lastCheckTime(0)
{
}

// Method to check if the trigger is active
Event Trigger::Check()
{
    // If the trigger is active, return an event with the trigger's name
	if (IsActive())
	{
		Event event(getName());
        // Return event with the trigger's name
		return event;
	}

    // If the trigger is not active, return a default event
	Event event;
    // Return default event
	return event;
}

// Method to get the target value associated with the trigger
Value<Unit*>* Trigger::GetTargetValue()
{
    // Get target value from context
    return context->GetValue<Unit*>(GetTargetName());
}

// Method to get the target unit
Unit* Trigger::GetTarget()
{
    // Get the target unit from the target value
    return GetTargetValue()->Get();
}

// Method to determine if the trigger needs to be checked
bool Trigger::needCheck()
{
    // If the check interval is less than 2, always check
    if (checkInterval < 2)
        return true;

    // Get the current time in milliseconds
    uint32 now = getMSTime();

    // If last check time is 0 or the interval has passed, update last check time and return true
    if (!lastCheckTime || now - lastCheckTime >= checkInterval)
    {
        lastCheckTime = now;
        // Return true to indicate the check is needed
        return true;
    }

    // Otherwise, do not check
    return false;
}
