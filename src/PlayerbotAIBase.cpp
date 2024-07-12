/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotAIBase.h"  // Include the header for PlayerbotAIBase class
#include "Playerbots.h"  // Include the Playerbots header, which might contain related configurations and utilities

// Constructor for PlayerbotAIBase
// Initializes nextAICheckDelay to 0 and sets _isBotAI to the provided value
PlayerbotAIBase::PlayerbotAIBase(bool isBotAI) : nextAICheckDelay(0), _isBotAI(isBotAI)
{
}

// Method to update the AI
// elapsed - the time elapsed since the last update
// minimal - flag indicating if only minimal updates should be performed
void PlayerbotAIBase::UpdateAI(uint32 elapsed, bool minimal)
{
    // Reduce the nextAICheckDelay by the elapsed time, but ensure it doesn't go below 0
    if (nextAICheckDelay > elapsed)
        nextAICheckDelay -= elapsed;
    else
        nextAICheckDelay = 0;

    // Check if the AI can be updated, if not, return early
    if (!CanUpdateAI())
        return;

    // Perform the internal AI update logic
    UpdateAIInternal(elapsed, minimal);
    // Yield the thread to possibly delay further updates
    YieldThread();
}

// Method to set the next AI check delay
// delay - the delay to set
void PlayerbotAIBase::SetNextCheckDelay(uint32 const delay)
{
    // if (nextAICheckDelay < delay)
    
    // Set the nextAICheckDelay to the provided delay
        // LOG_DEBUG("playerbots", "Setting lesser delay {} -> {}", nextAICheckDelay, delay);
    nextAICheckDelay = delay;
    
    // if (nextAICheckDelay > sPlayerbotAIConfig->globalCoolDown)
        // LOG_DEBUG("playerbots",  "std::set next check delay: {}", nextAICheckDelay);
}

// Method to increase the next AI check delay
// delay - the amount of delay to add
void PlayerbotAIBase::IncreaseNextCheckDelay(uint32 delay)
{
    // Add the provided delay to the current nextAICheckDelay
    nextAICheckDelay += delay;
     // if (nextAICheckDelay > sPlayerbotAIConfig->globalCoolDown)
    //     LOG_DEBUG("playerbots",  "increase next check delay: {}", nextAICheckDelay);
}

// Method to check if the AI can be updated
// Returns true if nextAICheckDelay is 0, indicating the AI can be updated
bool PlayerbotAIBase::CanUpdateAI()
{
    return nextAICheckDelay == 0;
}

// Method to yield the thread and possibly set a delay for the next update
// delay - flag indicating if a longer delay should be set
void PlayerbotAIBase::YieldThread(bool delay)
{
    // Set nextAICheckDelay to a default or increased value based on the delay flag
    if (nextAICheckDelay < sPlayerbotAIConfig->reactDelay)
        nextAICheckDelay = delay ? sPlayerbotAIConfig->reactDelay * 10 : sPlayerbotAIConfig->reactDelay;
}

// Method to check if the AI is active
// Returns true if nextAICheckDelay is less than the maximum wait time for a move
bool PlayerbotAIBase::IsActive()
{
    return nextAICheckDelay < sPlayerbotAIConfig->maxWaitForMove;
}

// Method to check if the AI instance is a bot AI
// Returns the value of _isBotAI
bool PlayerbotAIBase::IsBotAI() const
{
    return _isBotAI;
}
