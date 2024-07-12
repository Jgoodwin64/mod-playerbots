/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTAIBASE_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERBOTAIBASE_H

#include "Define.h"  // Include common definitions and utilities

// Declaration of the PlayerbotAIBase class
class PlayerbotAIBase
{
    public:
        // Constructor that initializes the class with a boolean indicating if it is a bot AI
        PlayerbotAIBase(bool isBotAI);

        // Method to check if the AI can be updated
        bool CanUpdateAI();
        // Method to set the delay for the next AI check
        void SetNextCheckDelay(uint32 const delay);
        // Method to increase the delay for the next AI check
        void IncreaseNextCheckDelay(uint32 delay);
        // Method to yield the thread, optionally with a delay
        void YieldThread(bool delay = false);
        // Virtual method to update the AI, can be overridden by derived classes
        virtual void UpdateAI(uint32 elapsed, bool minimal = false);
        // Pure virtual method to be implemented by derived classes for internal AI updates
        virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false) = 0;
        // Method to check if the AI is active
        bool IsActive();
        // Method to check if the instance is a bot AI
        bool IsBotAI() const;

    protected:
        uint32 nextAICheckDelay;  // Protected member variable for the next AI check delay

    private:
        bool _isBotAI;  // Private member variable indicating if this instance is a bot AI
};

#endif  // End of header guard
