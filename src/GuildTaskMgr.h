/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDTASKMGR_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_GUILDTASKMGR_H

#include "Common.h"     // Include common definitions and utilities
#include "Transaction.h" // Include transaction handling

#include <map>          // Include the map container from the standard library

// Forward declarations of classes used in the GuildTaskMgr class
class ChatHandler;
class Player;
class Unit;

// GuildTaskMgr class definition
class GuildTaskMgr
{
    public:
        // Constructor
        GuildTaskMgr() { };
        // Virtual destructor
        virtual ~GuildTaskMgr() { };

        // Static instance method for singleton pattern
        static GuildTaskMgr* instance()
        {
            static GuildTaskMgr instance; // Static instance of GuildTaskMgr
            return &instance;             // Return the static instance
        }

        // Method to update guild tasks for the owner and guild master
        void Update(Player* owner, Player* guildMaster);

        // Static method to handle console commands
        static bool HandleConsoleCommand(ChatHandler* handler, char const* args);

        // Method to check if an item is a guild task item
        bool IsGuildTaskItem(uint32 itemId, uint32 guildId);

        // Method to check item task completion
        bool CheckItemTask(uint32 itemId, uint32 obtained, Player* owner, Player* bot, bool byMail = false);

        // Method to check kill task completion
        void CheckKillTask(Player* owner, Unit* victim);

        // Internal method to handle kill task checks
        void CheckKillTaskInternal(Player* owner, Unit* victim);

        // Method to check if a text contains a task transfer
        bool CheckTaskTransfer(std::string const text, Player* owner, Player* bot);

    private:
        // Method to get task values for a specific owner and task type
        std::map<uint32, uint32> GetTaskValues(uint32 owner, std::string const type, uint32* validIn = nullptr);

        // Method to get a specific task value for an owner and guild
        uint32 GetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32* validIn = nullptr);

        // Method to set a specific task value for an owner and guild
        uint32 SetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32 value, uint32 validIn);

        // Method to create a task for an owner and guild
        uint32 CreateTask(Player* owner, uint32 guildId);

        // Method to send an advertisement for a task
        bool SendAdvertisement(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId);

        // Method to send an advertisement for an item task
        bool SendItemAdvertisement(CharacterDatabaseTransaction& trans, uint32 itemId, uint32 owner, uint32 guildId, uint32 validIn);

        // Method to send an advertisement for a kill task
        bool SendKillAdvertisement(CharacterDatabaseTransaction& trans, uint32 creatureId, uint32 owner, uint32 guildId, uint32 validIn);

        // Method to send a thank you message
        bool SendThanks(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId, uint32 payment);

        // Method to reward a player for task completion
        bool Reward(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId);

        // Method to create an item task for a player and guild
        bool CreateItemTask(Player* owner, uint32 guildId);

        // Method to create a kill task for a player and guild
        bool CreateKillTask(Player* owner, uint32 guildId);

        // Method to get the maximum count for an item task
        uint32 GetMaxItemTaskCount(uint32 itemId);

        // Method to clean up advertisements
        void CleanupAdverts();

        // Method to remove duplicated advertisements
        void RemoveDuplicatedAdverts();

        // Method to delete mail by buffer of IDs
        void DeleteMail(std::vector<uint32> buffer);

        // Method to send a completion message to a player
        void SendCompletionMessage(Player* player, std::string const verb);
};

// Macro to get the singleton instance of GuildTaskMgr
#define sGuildTaskMgr GuildTaskMgr::instance()

#endif  // End of header guard
