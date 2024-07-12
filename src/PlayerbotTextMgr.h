/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTTEXTMGR_H
#define _PLAYERBOT_PLAYERBOTTEXTMGR_H

#include "Common.h"

#include <map>
#include <vector>

// Macro to get a bot text by name
#define BOT_TEXT1(name) sPlayerbotTextMgr->GetBotText(name)
// Macro to get a bot text by name with placeholders replaced
#define BOT_TEXT2(name, replace) sPlayerbotTextMgr->GetBotText(name, replace)

// Struct to store bot text entry details
struct BotTextEntry
{
    BotTextEntry(std::string name, std::map<uint32, std::string> text, uint32 say_type, uint32 reply_type)
        : m_name(name), m_text(text), m_sayType(say_type), m_replyType(reply_type) {}
    std::string m_name; // Name of the text entry
    std::map<uint32, std::string> m_text; // Texts for different locales
    uint32 m_sayType; // Type of the text (say type)
    uint32 m_replyType; // Type of the reply (reply type)
};

// Struct to store chat reply data
struct ChatReplyData
{
    ChatReplyData(uint32 guid, uint32 type, std::string chat)
        : m_type(type), m_guid(guid), m_chat(chat) {}
    uint32 m_type; // Type of the chat reply
    uint32 m_guid = 0; // GUID of the chat reply
    std::string m_chat = ""; // Chat message
};

// Struct to store queued chat reply data
struct ChatQueuedReply
{
    ChatQueuedReply(uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name, time_t time)
        : m_type(type), m_guid1(guid1), m_guid2(guid2), m_msg(msg), m_chanName(chanName), m_name(name), m_time(time) {}
    uint32 m_type; // Type of the queued reply
    uint32 m_guid1; // First GUID associated with the reply
    uint32 m_guid2; // Second GUID associated with the reply
    std::string m_msg; // Message of the reply
    std::string m_chanName; // Channel name where the reply was sent
    std::string m_name; // Name associated with the reply
    time_t m_time; // Time when the reply was queued
};

// Enum to define different types of chat replies
enum ChatReplyType
{
    REPLY_NOT_UNDERSTAND,
    REPLY_GRUDGE,
    REPLY_VICTIM,
    REPLY_ATTACKER,
    REPLY_HELLO,
    REPLY_NAME,
    REPLY_ADMIN_ABUSE
};

// Class to manage player bot texts
class PlayerbotTextMgr
{
    public:
        PlayerbotTextMgr() {
            for (uint8 i = 0; i < MAX_LOCALES; ++i)
            {
                botTextLocalePriority[i] = 0; // Initialize locale priorities to 0
            }
        };
        virtual ~PlayerbotTextMgr() { }; // Destructor
        static PlayerbotTextMgr* instance()
        {
            static PlayerbotTextMgr instance;
            return &instance; // Return a singleton instance of PlayerbotTextMgr
        }

        std::string GetBotText(std::string name, std::map<std::string, std::string> placeholders); // Get a bot text by name with placeholders replaced
        std::string GetBotText(std::string name); // Get a bot text by name
        std::string GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders); // Get a bot text reply by type with placeholders replaced
        std::string GetBotText(ChatReplyType replyType, std::string name); // Get a bot text reply by type and name
        bool GetBotText(std::string name, std::string& text); // Get a bot text by name and store in reference if chance roll passes
        bool GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders); // Get a bot text by name with placeholders and store in reference if chance roll passes
        void LoadBotTexts(); // Load bot texts from the database
        void LoadBotTextChance(); // Load the probability for bot text occurrences from the database
        static void replaceAll(std::string& str, const std::string& from, const std::string& to); // Replace all occurrences of a substring in a string
        bool rollTextChance(std::string text); // Roll for a chance to use a bot text

        uint32 GetLocalePriority(); // Get the highest priority locale currently set
        void AddLocalePriority(uint32 locale); // Add locale priority, incrementing the count for the specified locale
        void ResetLocalePriority(); // Reset the locale priority counts

    private:
        std::map<std::string, std::vector<BotTextEntry>> botTexts; // Map to store bot texts by name
        std::map<std::string, uint32 > botTextChance; // Map to store bot text chances by name
        uint32 botTextLocalePriority[MAX_LOCALES]; // Array to store locale priorities
};

// Macro to get the singleton instance of PlayerbotTextMgr
#define sPlayerbotTextMgr PlayerbotTextMgr::instance()

#endif
