/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatFilter.h"         // Include the header for the ChatFilter class
#include "Group.h"              // Include the header for the Group class
#include "Playerbots.h"         // Include the header for the Playerbots class
#include "RtiTargetValue.h"     // Include the header for the RtiTargetValue class

// Method to filter chat messages, removing any part before the first space
std::string const ChatFilter::Filter(std::string& message)
{
    if (message.find("@") == std::string::npos)  // Check if the message contains '@'
        return message;

    return message.substr(message.find(" ") + 1);  // Return the message after the first space
}

// StrategyChatFilter class derived from ChatFilter
class StrategyChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the ChatFilter with bot AI
        StrategyChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) { }

        // Override Filter method to filter messages based on strategy keywords
        std::string const Filter(std::string& message) override
        {
            Player* bot = botAI->GetBot();

            // Check if the message starts with specific strategy keywords
            bool tank = message.find("@tank") == 0;
            if (tank && !botAI->IsTank(bot))
                return "";

            bool dps = message.find("@dps") == 0;
            if (dps && (botAI->IsTank(bot) || botAI->IsHeal(bot)))
                return "";

            bool heal = message.find("@heal") == 0;
            if (heal && !botAI->IsHeal(bot))
                return "";

            bool ranged = message.find("@ranged") == 0;
            if (ranged && !botAI->IsRanged(bot))
                return "";

            bool melee = message.find("@melee") == 0;
            if (melee && botAI->IsRanged(bot))
                return "";

            // If any strategy keyword is found, apply the base filter
            if (tank || dps || heal || ranged || melee)
                return ChatFilter::Filter(message);

            return message;
        }
};

// LevelChatFilter class derived from ChatFilter
class LevelChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the ChatFilter with bot AI
        LevelChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) { }

        // Override Filter method to filter messages based on level range
        std::string const Filter(std::string& message) override
        {
            Player* bot = botAI->GetBot();

            if (message[0] != '@')  // Check if the message starts with '@'
                return message;

            if (message.find("-") != std::string::npos)  // Check if the message contains a level range
            {
                uint32 fromLevel = atoi(message.substr(message.find("@") + 1, message.find("-")).c_str());
                uint32 toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());

                if (bot->getLevel() >= fromLevel && bot->getLevel() <= toLevel)
                    return ChatFilter::Filter(message);

                return message;
            }

		    uint32 level = atoi(message.substr(message.find("@") + 1, message.find(" ")).c_str());
            if (bot->getLevel() == level)
                return ChatFilter::Filter(message);

            return message;
        }
};

// CombatTypeChatFilter class derived from ChatFilter
class CombatTypeChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the ChatFilter with bot AI
        CombatTypeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) { }

        // Override Filter method to filter messages based on combat type
        std::string const Filter(std::string& message) override
        {
            Player* bot = botAI->GetBot();

            bool melee = message.find("@melee") == 0;
            bool ranged = message.find("@ranged") == 0;

            if (!melee && !ranged)
                return message;

            // Check if the combat type matches the bot's class
            switch (bot->getClass())
            {
                case CLASS_WARRIOR:
                case CLASS_PALADIN:
                case CLASS_ROGUE:
                case CLASS_DEATH_KNIGHT:
                    if (ranged)
                        return "";
                    break;
                case CLASS_HUNTER:
                case CLASS_PRIEST:
                case CLASS_MAGE:
                case CLASS_WARLOCK:
                    if (melee)
                        return "";
                    break;
                case CLASS_DRUID:
                    if (ranged && botAI->IsTank(bot))
                        return "";
                    if (melee && !botAI->IsTank(bot))
                        return "";
                    break;
                case CLASS_SHAMAN:
                    if (melee && botAI->IsHeal(bot))
                        return "";
                    if (ranged && !botAI->IsHeal(bot))
                        return "";
                    break;
            }

            return ChatFilter::Filter(message);  // Apply the base filter
        }
};

// RtiChatFilter class derived from ChatFilter
class RtiChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the ChatFilter with bot AI and sets up RTI targets
        RtiChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
        {
            rtis.push_back("@star");
            rtis.push_back("@circle");
            rtis.push_back("@diamond");
            rtis.push_back("@triangle");
            rtis.push_back("@moon");
            rtis.push_back("@square");
            rtis.push_back("@cross");
            rtis.push_back("@skull");
        }

        // Override Filter method to filter messages based on RTI targets
        std::string const Filter(std::string& message) override
        {
            Player* bot = botAI->GetBot();
            Group* group = bot->GetGroup();
            if (!group)
                return message;

            bool found = false;
            bool isRti = false;
            for (std::vector<std::string>::iterator i = rtis.begin(); i != rtis.end(); i++)
            {
                std::string const rti = *i;

                bool isRti = message.find(rti) == 0;
                if (!isRti)
                    continue;

                ObjectGuid rtiTarget = group->GetTargetIcon(RtiTargetValue::GetRtiIndex(rti.substr(1)));
                if (bot->GetGUID() == rtiTarget)
                    return ChatFilter::Filter(message);

                Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
                if (!target)
                    return "";

                if (target->GetGUID() != rtiTarget)
                    return "";

                found |= isRti;
                if (found)
                    break;
            }

            if (found)
                return ChatFilter::Filter(message);

            return message;
        }

    private:
        std::vector<std::string> rtis;  // Vector to store RTI targets
};

// ClassChatFilter class derived from ChatFilter
class ClassChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the ChatFilter with bot AI and sets up class names
        ClassChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
        {
            classNames["@death_knight"] = CLASS_DEATH_KNIGHT;
            classNames["@druid"] = CLASS_DRUID;
            classNames["@hunter"] = CLASS_HUNTER;
            classNames["@mage"] = CLASS_MAGE;
            classNames["@paladin"] = CLASS_PALADIN;
            classNames["@priest"] = CLASS_PRIEST;
            classNames["@rogue"] = CLASS_ROGUE;
            classNames["@shaman"] = CLASS_SHAMAN;
            classNames["@warlock"] = CLASS_WARLOCK;
            classNames["@warrior"] = CLASS_WARRIOR;
        }

        // Override Filter method to filter messages based on class
        std::string const Filter(std::string& message) override
        {
            Player* bot = botAI->GetBot();

            bool found = false;
            bool isClass = false;
            for (std::map<std::string, uint8>::iterator i = classNames.begin(); i != classNames.end(); i++)
            {
                bool isClass = message.find(i->first) == 0;
                if (isClass && bot->getClass() != i->second)
                    return "";

                found |= isClass;
                if (found)
                    break;
            }

            if (found)
                return ChatFilter::Filter(message);

            return message;
        }

    private:
        std::map<std::string, uint8> classNames;  // Map to store class names and their corresponding class IDs
};

// SubGroupChatFilter class derived from ChatFilter
class SubGroupChatFilter : public ChatFilter
{
    public:
        // Constructor that initializes the ChatFilter with bot AI
        SubGroupChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) { }

        // Override Filter method to filter messages based on sub-group
        std::string const Filter(std::string& message) override
        {
            Player* bot = botAI->GetBot();

            if (message.find("@group") == 0)
            {
                std::string const pnum = message.substr(6, message.find(" "));
                uint32 from = atoi(pnum.c_str());
                uint32 to = from;
                if (pnum.find("-") != std::string::npos)
                {
                    from = atoi(pnum.substr(pnum.find("@") + 1, pnum.find("-")).c_str());
                    to = atoi(pnum.substr(pnum.find("-") + 1, pnum.find(" ")).c_str());
                }

                if (!bot->GetGroup())
                    return message;

                uint32 sg = bot->GetSubGroup() + 1;
                if (sg >= from && sg <= to)
                    return ChatFilter::Filter(message);
            }

            return message;
        }
};

// CompositeChatFilter class derived from ChatFilter
CompositeChatFilter::CompositeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
{
    filters.push_back(new StrategyChatFilter(botAI));  // Add StrategyChatFilter to the list
    filters.push_back(new ClassChatFilter(botAI));     // Add ClassChatFilter to the list
    filters.push_back(new RtiChatFilter(botAI));       // Add RtiChatFilter to the list
    filters.push_back(new CombatTypeChatFilter(botAI));// Add CombatTypeChatFilter to the list
    filters.push_back(new LevelChatFilter(botAI));     // Add LevelChatFilter to the list
    filters.push_back(new SubGroupChatFilter(botAI));  // Add SubGroupChatFilter to the list
}

// Destructor to clean up the filters
CompositeChatFilter::~CompositeChatFilter()
{
    for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        delete (*i);  // Delete each filter in the list
}

// Override Filter method to apply all filters in the list to the message
std::string const CompositeChatFilter::Filter(std::string& message)
{
    for (uint32 j = 0; j < filters.size(); ++j)
    {
        for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        {
            message = (*i)->Filter(message);  // Apply each filter to the message
            if (message.empty())
                break;
        }
    }

    return message;  // Return the filtered message
}
