/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATFILTER_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_CHATFILTER_H

#include "Common.h"  // Include common definitions and utilities
#include "PlayerbotAIAware.h"  // Include the PlayerbotAIAware class definition

#include <vector>  // Include the vector container from the standard library

class PlayerbotAI;  // Forward declaration of the PlayerbotAI class

// ChatFilter class, inherits from PlayerbotAIAware
class ChatFilter : public PlayerbotAIAware
{
    public:
        // Constructor that initializes the base class with the provided PlayerbotAI pointer
        ChatFilter(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) { }
        // Virtual destructor
        virtual ~ChatFilter() { }

        // Virtual function to filter a chat message, to be overridden by derived classes
        virtual std::string const Filter(std::string& message);
};

// CompositeChatFilter class, inherits from ChatFilter
class CompositeChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the base class with the provided PlayerbotAI pointer
        CompositeChatFilter(PlayerbotAI* botAI);

        // Virtual destructor
        virtual ~CompositeChatFilter();
        // Override of the Filter function to apply multiple filters
        std::string const Filter(std::string& message) override;

    private:
        // Vector to hold pointers to multiple ChatFilter objects
        std::vector<ChatFilter*> filters;
};

#endif  // End of header guard
