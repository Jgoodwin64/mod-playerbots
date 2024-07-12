/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTCOMMANDSERVER_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERBOTCOMMANDSERVER_H

// PlayerbotCommandServer class definition
class PlayerbotCommandServer
{
    public:
        // Constructor
        PlayerbotCommandServer() { }
        
        // Virtual destructor
        virtual ~PlayerbotCommandServer() { }
        
        // Static method to get the singleton instance of the class
        static PlayerbotCommandServer* instance()
        {
            // Static local variable to hold the singleton instance
            static PlayerbotCommandServer instance;
            // Return a pointer to the singleton instance
            return &instance;
        }

        // Method to start the command server
        void Start();
};

// Macro definition to access the singleton instance of PlayerbotCommandServer
#define sPlayerbotCommandServer PlayerbotCommandServer::instance()

#endif  // End of header guard
