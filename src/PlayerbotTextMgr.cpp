/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

// Function to replace all occurrences of a substring with another substring in a given string
void PlayerbotTextMgr::replaceAll(std::string & str, const std::string & from, const std::string & to) {
    if (from.empty())
        return; // If the 'from' string is empty, do nothing
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to); // Replace 'from' with 'to'
        start_pos += to.length(); // Move past the replaced part to avoid infinite loop if 'to' contains 'from'
    }
}

// Function to load bot texts from the database
void PlayerbotTextMgr::LoadBotTexts()
{
    LOG_INFO("playerbots", "Loading playerbots texts...");

    uint32 count = 0; // Counter for the number of texts loaded
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TEXT)))
    {
        do
        {
            std::map<uint32, std::string> text; // Map to hold text entries for different locales
            Field* fields = result->Fetch(); // Fetch the current row
            std::string name = fields[0].Get<std::string>(); // Get the bot text name
            text[0] = fields[1].Get<std::string>(); // Get the default locale text
            uint32 sayType = fields[2].Get<uint32>(); // Get the say type
            uint32 replyType = fields[3].Get<uint32>(); // Get the reply type
            for (uint8 i = 1; i < MAX_LOCALES; ++i)
            {
                text[i] = fields[i + 3].Get<std::string>(); // Get texts for other locales
            }
            botTexts[name].push_back(BotTextEntry(name, text, sayType, replyType)); // Store the text entry
            ++count; // Increment the count of loaded texts
        }
        while (result->NextRow());
    }

    LOG_INFO("playerbots", "{} playerbots texts loaded", count); // Log the number of texts loaded
}

// Function to load the probability for bot text occurrences
void PlayerbotTextMgr::LoadBotTextChance()
{
    if (botTextChance.empty()) // Check if bot text chances are not already loaded
    {
        QueryResult results = PlayerbotsDatabase.Query("SELECT name, probability FROM ai_playerbot_texts_chance");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch(); // Fetch the current row
                std::string name = fields[0].Get<std::string>(); // Get the text name
                uint32 probability = fields[1].Get<uint32>(); // Get the probability

                botTextChance[name] = probability; // Store the probability for the text name
            } while (results->NextRow());
        }
    }
}

// General function to get a bot text by name
std::string PlayerbotTextMgr::GetBotText(std::string name)
{
    if (botTexts.empty()) // Check if any bot texts are loaded
    {
        LOG_ERROR("playerbots", "Can't get bot text {}! No bots texts loaded!", name);
        return ""; // Return an empty string if no texts are loaded
    }

    if (botTexts[name].empty()) // Check if there are texts for the specified name
    {
        LOG_ERROR("playerbots", "Can't get bot text {}! No bots texts for this name!", name);
        return ""; // Return an empty string if no texts are found for the name
    }

    std::vector<BotTextEntry>& list = botTexts[name]; // Get the list of text entries for the name
    BotTextEntry textEntry = list[urand(0, list.size() - 1)]; // Randomly select a text entry
    return !textEntry.m_text[GetLocalePriority()].empty() ? textEntry.m_text[GetLocalePriority()] : textEntry.m_text[0]; // Return the text for the highest priority locale or the default text
}

// Overloaded function to get a bot text by name and replace placeholders in the text
std::string PlayerbotTextMgr::GetBotText(std::string name, std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name); // Get the bot text by name
    if (botText.empty())
        return ""; // Return an empty string if no text is found

    for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        replaceAll(botText, i->first, i->second); // Replace each placeholder with its corresponding value

    return botText; // Return the modified text
}

// Function to get a bot text reply by ChatReplyType and replace placeholders in the text
std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders)
{
    if (botTexts.empty()) // Check if any bot texts are loaded
    {
        LOG_ERROR("playerbots", "Can't get bot text reply {}! No bots texts loaded!", replyType);
        return ""; // Return an empty string if no texts are loaded
    }
    if (botTexts["reply"].empty()) // Check if there are reply texts loaded
    {
        LOG_ERROR("playerbots", "Can't get bot text reply {}! No bots texts replies!", replyType);
        return ""; // Return an empty string if no reply texts are found
    }

    std::vector<BotTextEntry>& list = botTexts["reply"]; // Get the list of reply text entries
    std::vector<BotTextEntry> proper_list;
    for (auto text : list)
    {
        if (text.m_replyType == replyType)
            proper_list.push_back(text); // Collect texts that match the reply type
    }

    if (proper_list.empty())
        return ""; // Return an empty string if no matching texts are found

    BotTextEntry textEntry = proper_list[urand(0, proper_list.size() - 1)]; // Randomly select a matching text entry
    std::string botText = !textEntry.m_text[GetLocalePriority()].empty() ? textEntry.m_text[GetLocalePriority()] : textEntry.m_text[0]; // Get the text for the highest priority locale or the default text
    for (auto & placeholder : placeholders)
        replaceAll(botText, placeholder.first, placeholder.second); // Replace each placeholder with its corresponding value

    return botText; // Return the modified text
}

// Function to get a bot text reply by ChatReplyType and a specific placeholder name
std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::string name)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%s"] = name; // Create a placeholder map with the given name

    return GetBotText(replyType, placeholders); // Get the bot text reply with the placeholders replaced
}

// Function to roll for a chance to use a bot text by name
bool PlayerbotTextMgr::rollTextChance(std::string name)
{
    if (!botTextChance[name])
        return true; // If no specific chance is set, always use the text

    return urand(0, 100) < botTextChance[name]; // Roll a random number and compare it to the chance
}

// Overloaded function to get a bot text by name and store it in a reference variable if it passes the chance roll
bool PlayerbotTextMgr::GetBotText(std::string name, std::string &text)
{
    if (!rollTextChance(name))
        return false; // Return false if the text does not pass the chance roll

    text = GetBotText(name); // Get the bot text by name
    return !text.empty(); // Return true if the text is not empty
}

// Overloaded function to get a bot text by name with placeholders and store it in a reference variable if it passes the chance roll
bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders)
{
    if (!rollTextChance(name))
        return false; // Return false if the text does not pass the chance roll

    text = GetBotText(name, placeholders); // Get the bot text by name with placeholders replaced
    return !text.empty(); // Return true if the text is not empty
}

// Function to add locale priority, incrementing the count for the specified locale
void PlayerbotTextMgr::AddLocalePriority(uint32 locale)
{
    if (!locale)
        return; // If the locale is 0, do nothing

    botTextLocalePriority[locale]++; // Increment the priority count for the locale
}

// Function to get the highest priority locale currently set
uint32 PlayerbotTextMgr::GetLocalePriority()
{
    uint32 topLocale = 0;

    // If no real players online, reset top locale
    if (!sWorld->GetActiveSessionCount())
    {
        ResetLocalePriority();
        return 0; // Return 0 if no players are online
    }

    for (uint8 i = 0; i < MAX_LOCALES; ++i)
    {
        if (botTextLocalePriority[i] > topLocale)
            topLocale = i; // Find the locale with the highest priority count
    }
    return topLocale; // Return the highest priority locale
}

// Function to reset the locale priority counts
void PlayerbotTextMgr::ResetLocalePriority()
{
    for (uint8 i = 0; i < MAX_LOCALES; ++i)
    {
        botTextLocalePriority[i] = 0; // Reset the priority count for each locale
    }
}
