/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAIAWARE_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERbotAIAWARE_H

class PlayerbotAI;  // Forward declaration of PlayerbotAI class

class PlayerbotAIAware
{
    public:
        // Constructor that initializes the botAI member variable with the provided PlayerbotAI pointer
        PlayerbotAIAware(PlayerbotAI* botAI) : botAI(botAI) { }

    protected:
        PlayerbotAI* botAI;  // Protected member variable to hold the PlayerbotAI pointer
};

#endif  // End of header guard
