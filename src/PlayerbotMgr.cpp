/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CharacterCache.h"
#include "CharacterPackets.h"
#include "Common.h"
#include "Define.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "PlayerbotDbStore.h"
#include "PlayerbotFactory.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "ChannelMgr.h"
#include <cstdio>
#include <cstring>
#include <string>

// Constructor for PlayerbotHolder class, which initializes the base class PlayerbotAIBase with false
PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase(false)
{
}

// PlayerbotLoginQueryHolder is a class that holds queries related to player bot logins
class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
    private:
        uint32 masterAccountId;          // ID of the master account
        PlayerbotHolder* playerbotHolder; // Pointer to the PlayerbotHolder instance

    public:
        // Constructor for PlayerbotLoginQueryHolder, initializes base class and member variables
        PlayerbotLoginQueryHolder(PlayerbotHolder* playerbotHolder, uint32 masterAccount, uint32 accountId, ObjectGuid guid)
            : LoginQueryHolder(accountId, guid), masterAccountId(masterAccount), playerbotHolder(playerbotHolder) { }

        // Getter for masterAccountId
        uint32 GetMasterAccountId() const { return masterAccountId; }
        // Getter for playerbotHolder
        PlayerbotHolder* GetPlayerbotHolder() { return playerbotHolder; }
};

// Adds a player bot to the PlayerbotHolder
void PlayerbotHolder::AddPlayerBot(ObjectGuid playerGuid, uint32 masterAccountId)
{
    // Check if the bot is already added and is in the world
    Player* bot = ObjectAccessor::FindConnectedPlayer(playerGuid);
    if (bot && bot->IsInWorld())
        return;

    // Get the account ID associated with the player's GUID
    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(playerGuid);
    if (!accountId)
        return;

    // Create a PlayerbotLoginQueryHolder instance to handle login queries
    std::shared_ptr<PlayerbotLoginQueryHolder> holder = std::make_shared<PlayerbotLoginQueryHolder>(this, masterAccountId, accountId, playerGuid);
    if (!holder->Initialize())
    {
        return;
    }

    // Find the master session associated with the master account ID
    if (WorldSession* masterSession = sWorld->FindSession(masterAccountId))
    {
        // Add a query holder callback for when the database query completes
        masterSession->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete([this](SQLQueryHolderBase const& holder)
        {
            HandlePlayerBotLoginCallback(static_cast<PlayerbotLoginQueryHolder const&>(holder));
        });
    }
    else
    {
        // If master session is not found, add a callback to the world session
        sWorld->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete([this](SQLQueryHolderBase const& holder)
        {
            HandlePlayerBotLoginCallback(static_cast<PlayerbotLoginQueryHolder const&>(holder));
        });
    }
}

// Handles the callback after the player bot login query completes
void PlayerbotHolder::HandlePlayerBotLoginCallback(PlayerbotLoginQueryHolder const& holder)
{
    uint32 botAccountId = holder.GetAccountId();

    // Create a new WorldSession for the bot
    WorldSession* botSession = new WorldSession(botAccountId, "", nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), LOCALE_enUS, 0, false, false, 0, true);

    // Handle player login from database using the holder
    botSession->HandlePlayerLoginFromDB(holder); // will delete lqh

    Player* bot = botSession->GetPlayer();
    if (!bot)
    {
        // If bot is not found, log out the player bot and log an error
        LogoutPlayerBot(holder.GetGuid());
        // LOG_ERROR("playerbots", "Error logging in bot {}, please try to reset all random bots", holder.GetGuid().ToString().c_str());
        return;
    }

    // Notify the random player bot manager of the player login
    sRandomPlayerbotMgr->OnPlayerLogin(bot);

    uint32 masterAccount = holder.GetMasterAccountId();
    WorldSession* masterSession = masterAccount ? sWorld->FindSession(masterAccount) : nullptr;
    bool allowed = false;

    // Check if the bot is allowed to be controlled by the master account
    if (botAccountId == masterAccount)
        allowed = true;
    else if (masterSession && sPlayerbotAIConfig->allowGuildBots && bot->GetGuildId() != 0 && bot->GetGuildId() == masterSession->GetPlayer()->GetGuildId())
        allowed = true;
    else if (sPlayerbotAIConfig->IsInRandomAccountList(botAccountId))
        allowed = true;

    if (allowed)
    {
        // If allowed, handle bot login
        OnBotLogin(bot);
    }
    else
    {
        // If not allowed, send a message to the master session and log out the bot
        if (masterSession)
        {
            ChatHandler ch(masterSession);
            ch.PSendSysMessage("You are not allowed to control bot %s", bot->GetName());
        }
        OnBotLogin(bot);
        LogoutPlayerBot(bot->GetGUID());

        // LOG_ERROR("playerbots", "Attempt to add not allowed bot {}, please try to reset all random bots", bot->GetName());
    }
}

// Updates the sessions for all player bots
void PlayerbotHolder::UpdateSessions()
{
    // Iterate through all player bots
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->IsBeingTeleported())
        {
            // Handle teleport acknowledgment if the bot is being teleported
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI) {
                botAI->HandleTeleportAck();
            }
        }
        else if (bot->IsInWorld())
        {
            // Handle bot packets if the bot is in the world
            HandleBotPackets(bot->GetSession());
        }
    }
}

// Handles the packets for the bot session
void PlayerbotHolder::HandleBotPackets(WorldSession* session)
{
    WorldPacket* packet;
    // Process each packet in the session's packet queue
    while (session->GetPacketQueue().next(packet))
    {
        OpcodeClient opcode = static_cast<OpcodeClient>(packet->GetOpcode());
        ClientOpcodeHandler const* opHandle = opcodeTable[opcode];
        opHandle->Call(session, *packet);
        delete packet;
    }
}

// Logs out all player bots
void PlayerbotHolder::LogoutAllBots()
{
    /*
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd())
            break;

        Player* bot= itr->second;
        if (!GET_PLAYERBOT_AI(bot)->IsRealPlayer())
            LogoutPlayerBot(bot->GetGUID());
    }
    */

    // Iterate through all player bots and log them out
    PlayerBotMap bots = playerBots;
    for (auto& itr : bots)
    {
        Player* bot = itr.second;
        if (!bot)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        LogoutPlayerBot(bot->GetGUID());
    }
}

// Cancels the logout process for all player bots
void PlayerbotMgr::CancelLogout()
{
    Player* master = GetMaster();
    if (!master)
        return;

    // Iterate through all player bots and cancel their logout
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        if (bot->GetSession()->isLogingOut())
        {
            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
            botAI->TellMaster("Logout cancelled!");
        }
    }

    // Iterate through random player bots and cancel their logout if they have the same master
    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        if (botAI->GetMaster() != master)
            continue;

        if (bot->GetSession()->isLogingOut())
        {
            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
        }
    }
}

// Logs out a specific player bot by their GUID
void PlayerbotHolder::LogoutPlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleground() && !bot->InBattlegroundQueue() && botAI->HasActivePlayerMaster())
        {
            // Save the bot to the database before logging out
            sPlayerbotDbStore->Save(botAI);
        }

        LOG_INFO("playerbots", "Bot {} logging out", bot->GetName().c_str());
        bot->SaveToDB(false, false);

        WorldSession* botWorldSessionPtr = bot->GetSession();
        WorldSession* masterWorldSessionPtr = nullptr;

        if (botWorldSessionPtr->isLogingOut())
            return;

        Player* master = botAI->GetMaster();
        if (master)
            masterWorldSessionPtr = master->GetSession();

        // Determine if an instant logout is possible
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

        if (masterWorldSessionPtr && masterWorldSessionPtr->ShouldLogOut(time(nullptr)))
            logout = true;

        if (masterWorldSessionPtr && !masterWorldSessionPtr->GetPlayer())
            logout = true;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            botWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))
        {
            logout = true;
        }

        if (master && (master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            (masterWorldSessionPtr && masterWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))))
        {
            logout = true;
        }

        TravelTarget* target = nullptr;
        if (botAI->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        }

        // Peiru: Allow bots to always instant logout to see if this resolves logout crashes
        logout = true;

        // If no instant logout, request normal logout
        if (!logout)
        {
            if (bot->GetSession()->isLogingOut())
                return;
            else if (bot)
            {
                botAI->TellMaster("I'm logging out!");
                WorldPackets::Character::LogoutRequest data = WorldPacket(CMSG_LOGOUT_REQUEST);
                botWorldSessionPtr->HandleLogoutRequestOpcode(data);
                if (!bot)
                {
                    playerBots.erase(guid);
                    delete botWorldSessionPtr;
                    if (target)
                        delete target;
                }
                return;
            }
            else
            {
                playerBots.erase(guid);    // Deletes bot player pointer inside this WorldSession PlayerBotMap
                delete botWorldSessionPtr;  // Finally delete the bot's WorldSession
                if (target)
                    delete target;
            }
            return;
        }
        // If instant logout possible, do it
        else if (bot && (logout || !botWorldSessionPtr->isLogingOut()))
        {
            botAI->TellMaster("Goodbye!");
            playerBots.erase(guid);                 // Deletes bot player pointer inside this WorldSession PlayerBotMap
            botWorldSessionPtr->LogoutPlayer(true); // This will delete the bot Player object and PlayerbotAI object
            delete botWorldSessionPtr;              // Finally delete the bot's WorldSession
        }
    }
}

// Disables a specific player bot by their GUID
void PlayerbotHolder::DisablePlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI) {
            return;
        }
        botAI->TellMaster("Goodbye!");
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleground() && !bot->InBattlegroundQueue() && botAI->HasActivePlayerMaster())
        {
            // Save the bot to the database before logging out
            sPlayerbotDbStore->Save(botAI);
        }

        LOG_DEBUG("playerbots", "Bot {} logged out", bot->GetName().c_str());

        bot->SaveToDB(false, false);

        if (botAI->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
            if (target)
                delete target;
        }

        playerBots.erase(guid);    // Deletes bot player pointer inside this WorldSession PlayerBotMap

        delete botAI;
    }
}

// Retrieves a player bot by their GUID
Player* PlayerbotHolder::GetPlayerBot(ObjectGuid playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

// Retrieves a player bot by their low GUID
Player* PlayerbotHolder::GetPlayerBot(ObjectGuid::LowType lowGuid) const
{
    ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(lowGuid);
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

// Handles the login process for a bot
void PlayerbotHolder::OnBotLogin(Player* const bot)
{
    // Prevent duplicate login
    if (playerBots.find(bot->GetGUID()) != playerBots.end()) {
        return;
    }

    // Add the bot to the Playerbots manager and update the playerBots map
    sPlayerbotsMgr->AddPlayerbotData(bot, true);
    playerBots[bot->GetGUID()] = bot;
    OnBotLoginInternal(bot);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI) {
        return;
    }
    Player* master = botAI->GetMaster();
    if (master)
    {
        ObjectGuid masterGuid = master->GetGUID();
        if (master->GetGroup() && !master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            if (master)
            {
                if (master->GetGUID() == member)
                {
                    groupValid = true;
                    break;
                }
            }
            else
            {
                uint32 account = sCharacterCache->GetCharacterAccountIdByGuid(member);
                if (!sPlayerbotAIConfig->IsInRandomAccountList(account))
                {
                    groupValid = true;
                    break;
                }
            }
        }

        if (!groupValid)
        {
            WorldPacket p;
            std::string const member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
        }
    }

    group = bot->GetGroup();
    if (group)
    {
        botAI->ResetStrategies();
    }
    else
    {
        // botAI->ResetStrategies(!sRandomPlayerbotMgr->IsRandomBot(bot));
        botAI->ResetStrategies();
    }

    if (master && !master->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        bot->GetMotionMaster()->MovementExpired();
        bot->CleanupAfterTaxiFlight();
    }

    // Check activity
    botAI->AllowActivity(ALL_ACTIVITY, true);

    // Set delay on login
    botAI->SetNextCheckDelay(urand(2000, 4000));

    botAI->TellMaster("Hello!", PLAYERBOT_SECURITY_TALK);

    if (master && master->GetGroup() && !group) {
        master->GetGroup()->AddMember(bot);
    }

    uint32 accountId = bot->GetSession()->GetAccountId();
    bool isRandomAccount = sPlayerbotAIConfig->IsInRandomAccountList(accountId);

    if (isRandomAccount && sPlayerbotAIConfig->randomBotFixedLevel) {
        bot->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    } else if (isRandomAccount && !sPlayerbotAIConfig->randomBotFixedLevel) {
        bot->RemovePlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }

    bot->SaveToDB(false, false);
    if (master && isRandomAccount && master->GetLevel() < bot->GetLevel()) {
        // PlayerbotFactory factory(bot, master->getLevel());
        // factory.Randomize(false);
        uint32 mixedGearScore = PlayerbotAI::GetMixedGearScore(master, true, false, 12) * sPlayerbotAIConfig->autoInitEquipLevelLimitRatio;
        PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY, mixedGearScore);
        factory.Randomize(false);
    }

    // Bots join World chat if not solo oriented
    if (bot->getLevel() >= 10 && sRandomPlayerbotMgr->IsRandomBot(bot) && GET_PLAYERBOT_AI(bot) && GET_PLAYERBOT_AI(bot)->GetGrouperType() != GrouperType::SOLO)
    {
        // TODO make action/config
        // Make the bot join the world channel for chat
        WorldPacket pkt(CMSG_JOIN_CHANNEL);
        pkt << uint32(0) << uint8(0) << uint8(0);
        pkt << std::string("World");
        pkt << ""; // Pass
        bot->GetSession()->HandleJoinChannel(pkt);
    }
    // Join standard channels
    AreaTableEntry const* current_zone = sAreaTableStore.LookupEntry(bot->GetAreaId());
    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    std::string current_zone_name = current_zone ? current_zone->area_name[0] : "";

    if (current_zone && cMgr)
    {
        for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
        {
            ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i);
            if (!channel) continue;

            bool isLfg = (channel->flags & CHANNEL_DBC_FLAG_LFG) != 0;

            // Skip non built-in channels or global channel without zone name in pattern
            if (!isLfg && (!channel || (channel->flags & 4) == 4))
                continue;

            // New channel
            Channel* new_channel = nullptr;
            if (isLfg)
            {
                std::string lfgChannelName = channel->pattern[0];
                new_channel = cMgr->GetJoinChannel("LookingForGroup", channel->ChannelID);
            }
            else
            {
                char new_channel_name_buf[100];
                snprintf(new_channel_name_buf, 100, channel->pattern[0], current_zone_name.c_str());
                new_channel = cMgr->GetJoinChannel(new_channel_name_buf, channel->ChannelID);
            }
            if (new_channel && new_channel->GetName().length() > 0)
                new_channel->JoinChannel(bot, "");
        }
    }
}

// Processes a bot command and returns the result as a string
std::string const PlayerbotHolder::ProcessBotCommand(std::string const cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig->enabled || guid.IsEmpty())
        return "bot system is disabled";

    uint32 botAccount = sCharacterCache->GetCharacterAccountIdByGuid(guid);
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(guid.GetCounter());
    bool isRandomAccount = sPlayerbotAIConfig->IsInRandomAccountList(botAccount);
    bool isMasterAccount = (masterAccountId == botAccount);

    if (!isRandomAccount && !isMasterAccount && !admin && masterguid)
    {
        Player* master = ObjectAccessor::FindConnectedPlayer(masterguid);
        if (master && (!sPlayerbotAIConfig->allowGuildBots || !masterGuildId || (masterGuildId && sCharacterCache->GetCharacterGuildIdByGuid(guid) != masterGuildId)))
            return "not in your guild or account";
    }

    if (cmd == "add" || cmd == "login")
    {
        if (ObjectAccessor::FindPlayer(guid))
            return "player already logged in";

        if (!sPlayerbotAIConfig->allowPlayerBots && !isRandomAccount && !isMasterAccount)
            return "You cannot login another player's character as bot.";

        AddPlayerBot(guid, masterAccountId);
        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        if (!ObjectAccessor::FindPlayer(guid))
            return "player is offline";

        if (!GetPlayerBot(guid))
            return "not your bot";

        LogoutPlayerBot(guid);
        return "ok";
    }

    // if (admin)
    // {
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        bot = sRandomPlayerbotMgr->GetPlayerBot(guid);

    if (!bot)
        return "bot not found";

    if (!isRandomAccount || isRandomBot) {
        return "ERROR: You can not use this command on non-summoned random bot.";
    }

    if (GET_PLAYERBOT_AI(bot)) {
        if (Player* master = GET_PLAYERBOT_AI(bot)->GetMaster())
        {
            if (master->GetSession()->GetSecurity() <= SEC_PLAYER && sPlayerbotAIConfig->autoInitOnly && cmd != "init=auto") {
                return "The command is not allowed, use init=auto instead.";
            }
            int gs;
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=legendary" || cmd == "init=yellow")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=auto")
            {
                uint32 mixedGearScore = PlayerbotAI::GetMixedGearScore(master, true, false, 12) * sPlayerbotAIConfig->autoInitEquipLevelLimitRatio;
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY, mixedGearScore);
                factory.Randomize(false);
                return "ok, gear score limit: " + std::to_string(mixedGearScore / (ITEM_QUALITY_EPIC + 1)) + "(for epic)";
            }
            else if (cmd.starts_with("init=") && sscanf(cmd.c_str(), "init=%d", &gs) != -1)
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY, gs);
                factory.Randomize(false);
                return "ok, gear score limit: " + std::to_string(gs / (ITEM_QUALITY_EPIC + 1)) + "(for epic)";
            }
        }

        if (cmd == "refresh=raid")
        {   // TODO: This function is not perfect yet. If you are already in a raid, 
            // after the command is executed, the AI ​​needs to go back online or exit the raid and re-enter.
            PlayerbotFactory factory(bot, bot->getLevel());
            factory.UnbindInstance();
            return "ok";
        }
    }

    if (cmd == "levelup" || cmd == "level")
    {
        PlayerbotFactory factory(bot, bot->getLevel());
        factory.Randomize(true);
        return "ok";
    }
    else if (cmd == "refresh")
    {
        PlayerbotFactory factory(bot, bot->getLevel());
        factory.Refresh();
        return "ok";
    }
    else if (cmd == "random")
    {
        sRandomPlayerbotMgr->Randomize(bot);
        return "ok";
    }
    else if (cmd == "quests"){
        PlayerbotFactory factory(bot, bot->getLevel());
        factory.InitInstanceQuests();
        return "Initialization quests";
    }
    // }

    return "unknown command";
}

// Handles the Playerbot Manager command
bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->enabled)
    {
        handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return false;
    }

    WorldSession* m_session = handler->GetSession();
    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(player);
    if (!mgr)
    {
        handler->PSendSysMessage("You cannot control bots yet");
        return false;
    }

    // Handle the player bot command and send the response messages
    std::vector<std::string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("%s", i->c_str());
    }

    return true;
}
// Handles the player bot command
std::vector<std::string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    std::vector<std::string> messages; // Vector to store messages that will be returned

    if (!*args) // Check if no arguments are passed
    {
        // Provide usage information if no arguments are passed
        messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME");
        messages.push_back("       addclass CLASSNAME");
        return messages; // Return the usage messages
    }

    char* cmd = strtok((char*)args, " "); // Extract the first command from the arguments
    char* charname = strtok(nullptr, " "); // Extract the second part (character name) from the arguments
    if (!cmd) // Check if no command is provided
    {
        // Provide usage information if no command is found
        messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME or addclass CLASSNAME");
        return messages; // Return the usage message
    }

    if (!strcmp(cmd, "initself")) { // Check if the command is 'initself'
        if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) { // Check if the player has GM security level
            // Uncomment this to execute bot login process
            // OnBotLogin(master);

            // Initialize the player bot with epic quality items
            PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_EPIC);
            factory.Randomize(false);
            messages.push_back("initself ok"); // Confirm the command execution
            return messages;
        } else {
            messages.push_back("ERROR: Only GM can use this command."); // Error message for insufficient permissions
            return messages;
        }
    }

    if (!strncmp(cmd, "initself=", 9)) { // Check if the command starts with 'initself='
        if (!strcmp(cmd, "initself=rare")) { // Initialize with rare items
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            } else {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        if (!strcmp(cmd, "initself=epic")) { // Initialize with epic items
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            } else {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        int32 gs;
        if (sscanf(cmd, "initself=%d", &gs) != -1) { // Initialize with a specific gear score
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_LEGENDARY, gs);
                factory.Randomize(false);
                messages.push_back("initself ok, gs = " + std::to_string(gs));
                return messages;
            } else {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
    }

    if (!strcmp(cmd, "list")) // Handle 'list' command to list bots
    {
        messages.push_back(ListBots(master)); // Call ListBots function and add the result to messages
        return messages;
    }

    if (!strcmp(cmd, "reload")) // Handle 'reload' command to reload the configuration
    {
        messages.push_back("Reloading config"); // Notify about config reload
        sPlayerbotAIConfig->Initialize(); // Reload the configuration
        return messages;
    }

    if (!strcmp(cmd, "tweak")) // Handle 'tweak' command to tweak the configuration
    {
        sPlayerbotAIConfig->tweakValue = sPlayerbotAIConfig->tweakValue++; // Increment tweakValue
        if (sPlayerbotAIConfig->tweakValue > 2) // Reset tweakValue if it exceeds 2
            sPlayerbotAIConfig->tweakValue = 0;

        messages.push_back("Set tweakvalue to " + std::to_string(sPlayerbotAIConfig->tweakValue)); // Notify about new tweakValue
        return messages;
    }

    if (!strcmp(cmd, "self")) // Handle 'self' command for self-bot actions
    {
        if (GET_PLAYERBOT_AI(master)) // Check if bot AI is enabled for the master
        {
            messages.push_back("Disable player botAI"); // Notify about disabling bot AI
            DisablePlayerBot(master->GetGUID()); // Disable the bot AI
        }
        else if (sPlayerbotAIConfig->selfBotLevel == 0)
            messages.push_back("Self-bot is disabled"); // Notify if self-bot is disabled
        else if (sPlayerbotAIConfig->selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            messages.push_back("You do not have permission to enable player botAI"); // Permission error message
        else
        {
            messages.push_back("Enable player botAI"); // Notify about enabling bot AI
            OnBotLogin(master); // Enable the bot AI
        }

        return messages;
    }

    if (!strcmp(cmd, "lookup")) // Handle 'lookup' command to lookup bots
    {
        messages.push_back(LookupBots(master)); // Call LookupBots function and add the result to messages
        return messages;
    }

    if (!strcmp(cmd, "addclass")) // Handle 'addclass' command to add a class bot
    {
        if (sPlayerbotAIConfig->addClassCommand == 0 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER) {
            messages.push_back("You do not have permission to create bot by addclass command"); // Permission error message
            return messages;
        }
        if (!charname) { // Check if class name is provided
            messages.push_back("addclass: invalid CLASSNAME(warrior/paladin/hunter/rogue/priest/shaman/mage/warlock/druid/dk)"); // Error message for invalid class name
            return messages;
        }
        uint8 claz;
        // Map class names to class IDs
        if (!strcmp(charname, "warrior"))
        {
            claz = 1;
        }
        else if (!strcmp(charname, "paladin"))
        {
            claz = 2;
        }
        else if (!strcmp(charname, "hunter"))
        {
            claz = 3;
        }
        else if (!strcmp(charname, "rogue"))
        {
            claz = 4;
        }
        else if (!strcmp(charname, "priest"))
        {
            claz = 5;
        }
        else if (!strcmp(charname, "shaman"))
        {
            claz = 7;
        }
        else if (!strcmp(charname, "mage"))
        {
            claz = 8;
        }
        else if (!strcmp(charname, "warlock"))
        {
            claz = 9;
        }
        else if (!strcmp(charname, "druid"))
        {
            claz = 11;
        }
        else if (!strcmp(charname, "dk"))
        {
            claz = 6;
        }
        else
        {
            messages.push_back("Error: Invalid Class. Try again."); // Error message for invalid class
            return messages;
        }
        uint8 master_race = master->getRace(); // Get the race of the master
        std::string race_limit; // Define race limit based on the master's race
        switch (master_race)
        {
            case 1: // Human
            case 3: // Dwarf
            case 4: // Night Elf
            case 7: // Gnome
            case 11: // Draenei
                race_limit = "1, 3, 4, 7, 11";
                break;
            case 2: // Orc
            case 5: // Undead
            case 6: // Tauren
            case 8: // Troll
            case 10: // Blood Elf
                race_limit = "2, 5, 6, 8, 10";
                break;
        }
        // Find a bot that fits the conditions and is not in any guild
        QueryResult results = CharacterDatabase.Query("SELECT guid FROM characters "
            "WHERE name IN (SELECT name FROM playerbots_names) AND class = '{}' AND online = 0 AND race IN ({}) AND guid NOT IN ( SELECT guid FROM guild_member ) "
            "ORDER BY account DESC LIMIT 1", claz, race_limit);
        if (results) // If a suitable bot is found
        {
            Field* fields = results->Fetch();
            ObjectGuid guid = ObjectGuid(HighGuid::Player, fields[0].Get<uint32>());
            AddPlayerBot(guid, master->GetSession()->GetAccountId()); // Add the bot to the player's account

            messages.push_back("addclass " + std::string(charname) + " ok"); // Notify about successful addition
            return messages;
        }
        messages.push_back("addclass failed."); // Notify about failure to add bot
        return messages;
    }

    std::string charnameStr; // Variable to hold character name

    if (!charname) // Check if no character name is provided
    {
        std::string name;
        bool isPlayer = sCharacterCache->GetCharacterNameByGuid(master->GetTarget(), name); // Get the name of the target character
        if (isPlayer) {
            charnameStr = name; // Set character name if the target is a player
        } else {
            messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME"); // Usage message for invalid target
            return messages;
        }
    } else {
        charnameStr = charname; // Set character name from the provided argument
    }

    std::string const cmdStr = cmd; // Store the command as a constant string

    std::unordered_set<std::string> bots; // Set to store bot names
    if (charnameStr == "*" && master) // If character name is '*', add all group members
    {
        Group* group = master->GetGroup(); // Get the group of the master
        if (!group)
        {
            messages.push_back("you must be in group"); // Error message if the master is not in a group
            return messages;
        }

        Group::MemberSlotList slots = group->GetMemberSlots(); // Get the group members
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
            ObjectGuid member = i->guid;

            if (member == master->GetGUID()) // Skip the master
                continue;

            std::string bot;
            if (sCharacterCache->GetCharacterNameByGuid(member, bot)) // Get the name of each group member
                bots.insert(bot); // Add the name to the set of bots
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER) // If character name is '!' and the master has GM permissions
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            if (Player* bot = i->second)
                if (bot->IsInWorld())
                    bots.insert(bot->GetName()); // Add all online bots to the set
        }
    }

    std::vector<std::string> chars = split(charnameStr, ','); // Split character names by comma
    for (std::vector<std::string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        std::string const s = *i;

        uint32 accountId = GetAccountId(s); // Get the account ID for each character name
        if (!accountId)
        {
            bots.insert(s); // Add the character name to the set if account ID is not found
            continue;
        }

        QueryResult results = CharacterDatabase.Query("SELECT name FROM characters WHERE account = {}", accountId); // Query all characters for the account
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string const charName = fields[0].Get<std::string>();
                bots.insert(charName); // Add each character name to the set
            } while (results->NextRow());
        }
    }

    for (auto i = bots.begin(); i != bots.end(); ++i)
    {
        std::string const bot = *i;

        std::ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sCharacterCache->GetCharacterGuidByName(bot); // Get the GUID of the bot by name
        if (!member)
        {
            out << "character not found"; // Error message if the bot is not found
        }
        else if (master && member != master->GetGUID())
        {
            out << ProcessBotCommand(cmdStr, member, master->GetGUID(), master->GetSession()->GetSecurity() >= SEC_GAMEMASTER, master->GetSession()->GetAccountId(), master->GetGuildId()); // Process the bot command
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, ObjectGuid::Empty, true, -1, -1); // Process the bot command without a master
        }

        messages.push_back(out.str()); // Add the command output to messages
    }

    return messages; // Return all messages
}

uint32 PlayerbotHolder::GetAccountId(std::string const name)
{
    return AccountMgr::GetId(name); // Get account ID by character name
}

uint32 PlayerbotHolder::GetAccountId(ObjectGuid guid)
{
    if (!guid.IsPlayer()) // Check if the GUID is not a player
        return 0;

    // Prevent DB access for online player
    if (Player* player = ObjectAccessor::FindConnectedPlayer(guid))
        return player->GetSession()->GetAccountId(); // Return account ID if the player is online

    ObjectGuid::LowType lowguid = guid.GetCounter();

    if (QueryResult result = CharacterDatabase.Query("SELECT account FROM characters WHERE guid = {}", lowguid))
    {
        uint32 acc = (*result)[0].Get<uint32>(); // Get account ID from the database
        return acc;
    }

    return 0; // Return 0 if account ID is not found
}

std::string const PlayerbotHolder::ListBots(Player* master)
{
    std::set<std::string> bots; // Set to store bot names
    std::map<uint8, std::string> classNames; // Map to store class names by class ID

    // Initialize class names
    classNames[CLASS_DEATH_KNIGHT] = "Death Knight";
    classNames[CLASS_DRUID] = "Druid";
    classNames[CLASS_HUNTER] = "Hunter";
    classNames[CLASS_MAGE] = "Mage";
    classNames[CLASS_PALADIN] = "Paladin";
    classNames[CLASS_PRIEST] = "Priest";
    classNames[CLASS_ROGUE] = "Rogue";
    classNames[CLASS_SHAMAN] = "Shaman";
    classNames[CLASS_WARLOCK] = "Warlock";
    classNames[CLASS_WARRIOR] = "Warrior";
    classNames[CLASS_DEATH_KNIGHT] = "DeathKnight";

    std::map<std::string, std::string> online; // Map to store online status of bots
    std::vector<std::string> names; // Vector to store bot names
    std::map<std::string, std::string> classes; // Map to store class names of bots

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        std::string const name = bot->GetName();
        bots.insert(name);

        names.push_back(name);
        online[name] = "+"; // Mark bot as online
        classes[name] = classNames[bot->getClass()]; // Get the class name of the bot
    }

    if (master)
    {
        QueryResult results = CharacterDatabase.Query("SELECT class, name FROM characters WHERE account = {}", master->GetSession()->GetAccountId());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint8 cls = fields[0].Get<uint8>();
                std::string const name = fields[1].Get<std::string>();
                if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
                {
                    names.push_back(name);
                    online[name] = "-"; // Mark bot as offline
                    classes[name] = classNames[cls];
                }
            } while (results->NextRow());
        }
    }

    std::sort(names.begin(), names.end()); // Sort bot names

    if (Group* group = master->GetGroup()) // If the master is in a group, add group members
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* member = ObjectAccessor::FindPlayer(itr->guid);
            if (member && sRandomPlayerbotMgr->IsRandomBot(member))
            {
                std::string const name = member->GetName();

                names.push_back(name);
                online[name] = "+"; // Mark bot as online
                classes[name] = classNames[member->getClass()]; // Get the class name of the bot
            }
        }
    }

    std::ostringstream out;
    bool first = true;
    out << "Bot roster: ";
    for (std::vector<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        if (first)
            first = false;
        else
            out << ", ";

        std::string const name = *i;
        out << online[name] << name << " " << classes[name]; // Format the bot roster output
    }

    return out.str(); // Return the bot roster as a string
}

std::string const PlayerbotHolder::LookupBots(Player* master)
{
    std::list<std::string> messages; // List to store lookup messages
    messages.push_back("Classes Available:");
    messages.push_back("|TInterface\\icons\\INV_Sword_27.png:25:25:0:-1|t Warrior");
    messages.push_back("|TInterface\\icons\\INV_Hammer_01.png:25:25:0:-1|t Paladin");
    messages.push_back("|TInterface\\icons\\INV_Weapon_Bow_07.png:25:25:0:-1|t Hunter");
    messages.push_back("|TInterface\\icons\\INV_ThrowingKnife_04.png:25:25:0:-1|t Rogue");
    messages.push_back("|TInterface\\icons\\INV_Staff_30.png:25:25:0:-1|t Priest");
    messages.push_back("|TInterface\\icons\\inv_jewelry_talisman_04.png:25:25:0:-1|t Shaman");
    messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Mage");
    messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Warlock");
    messages.push_back("|TInterface\\icons\\Ability_Druid_Maul.png:25:25:0:-1|t Druid");
    messages.push_back("DK");
    messages.push_back("(Usage: .bot lookup CLASS)");
    std::string ret_msg;
    for (std::string msg: messages) {
        ret_msg += msg + "\n";
    }
    return ret_msg;
}

PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(),  master(master), lastErrorTell(0)
{
}

PlayerbotMgr::~PlayerbotMgr()
{
    if (master)
        sPlayerbotsMgr->RemovePlayerBotData(master->GetGUID());
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool /*minimal*/)
{
    SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
    CheckTellErrors(elapsed);
}

void PlayerbotMgr::HandleCommand(uint32 type, std::string const text)
{
    Player* master = GetMaster();
    if (!master)
        return;

    if (text.find(sPlayerbotAIConfig->commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, text, sPlayerbotAIConfig->commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i);
        }

        return;
    }

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleCommand(type, text, master);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == master)
            botAI->HandleCommand(type, text, master);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == GetMaster())
            botAI->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            break;
        }
        // if master cancelled logout, cancel too
        case CMSG_LOGOUT_CANCEL:
        {
            CancelLogout();
            break;
        }
    }
}

void PlayerbotMgr::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleMasterOutgoingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == GetMaster())
            botAI->HandleMasterOutgoingPacket(packet);
    }
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB(false, false);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (GET_PLAYERBOT_AI(bot) && GET_PLAYERBOT_AI(bot)->GetMaster() == GetMaster())
            bot->SaveToDB(false, false);
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI) {
        return;
    }
    botAI->SetMaster(master);
    botAI->ResetStrategies();

    LOG_INFO("playerbots", "Bot {} logged in", bot->GetName().c_str());
}

void PlayerbotMgr::OnPlayerLogin(Player* player)
{
    // set locale priority for bot texts
    sPlayerbotTextMgr->AddLocalePriority(player->GetSession()->GetSessionDbcLocale());

    if (sPlayerbotAIConfig->selfBotLevel > 2)
        HandlePlayerbotCommand("self", player);

    if (!sPlayerbotAIConfig->botAutologin)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.Query("SELECT name FROM characters WHERE account = {}", accountId);
    if (results)
    {
        std::ostringstream out;
        out << "add ";
        bool first = true;
        do
        {
            Field* fields = results->Fetch();

            if (first)
                first = false;
            else
                out << ",";

            out << fields[0].Get<std::string>();
        } while (results->NextRow());

        HandlePlayerbotCommand(out.str().c_str(), player);
    }
}

void PlayerbotMgr::TellError(std::string const botName, std::string const text)
{
    std::set<std::string> names = errors[text];
    if (names.find(botName) == names.end())
    {
        names.insert(botName);
    }

    errors[text] = names;
}

void PlayerbotMgr::CheckTellErrors(uint32 elapsed)
{
    time_t now = time(nullptr);
    if ((now - lastErrorTell) < sPlayerbotAIConfig->errorDelay / 1000)
        return;

    lastErrorTell = now;

    for (PlayerBotErrorMap::iterator i = errors.begin(); i != errors.end(); ++i)
    {
        std::string const text = i->first;
        std::set<std::string> names = i->second;

        std::ostringstream out;
        bool first = true;
        for (std::set<std::string>::iterator j = names.begin(); j != names.end(); ++j)
        {
            if (!first)
                out << ", ";
            else
                first = false;

            out << *j;
        }

        out << "|cfff00000: " << text;

        ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }

    errors.clear();
}

void PlayerbotsMgr::AddPlayerbotData(Player* player, bool isBotAI)
{
    if (!player)
    {
        return;
    }
    // If the guid already exists in the map, remove it
    std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsMap.find(player->GetGUID());
    if (itr != _playerbotsMap.end())
    {
        _playerbotsMap.erase(itr);
    }
    if (!isBotAI)
    {
        PlayerbotMgr* playerbotMgr = new PlayerbotMgr(player);
        ASSERT(_playerbotsMap.emplace(player->GetGUID(), playerbotMgr).second);

        playerbotMgr->OnPlayerLogin(player);
    }
    else
    {
        PlayerbotAI* botAI = new PlayerbotAI(player);
        ASSERT(_playerbotsMap.emplace(player->GetGUID(), botAI).second);
    }
}

void PlayerbotsMgr::RemovePlayerBotData(ObjectGuid const& guid)
{
    std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsMap.find(guid);
    if (itr != _playerbotsMap.end())
    {
        _playerbotsMap.erase(itr);
    }
}

PlayerbotAI* PlayerbotsMgr::GetPlayerbotAI(Player* player)
{
    if (!(sPlayerbotAIConfig->enabled) || !player)
    {
        return nullptr;
    }
    // if (player->GetSession()->isLogingOut() || player->IsDuringRemoveFromWorld()) {
    //     return nullptr;
    // }
    auto itr = _playerbotsMap.find(player->GetGUID());
    if (itr != _playerbotsMap.end())
    {
        if (itr->second->IsBotAI())
            return reinterpret_cast<PlayerbotAI*>(itr->second);
    }

    return nullptr;
}

PlayerbotMgr* PlayerbotsMgr::GetPlayerbotMgr(Player* player)
{
    if (!(sPlayerbotAIConfig->enabled) || !player)
    {
        return nullptr;
    }
    auto itr = _playerbotsMap.find(player->GetGUID());
    if (itr != _playerbotsMap.end())
    {
        if (!itr->second->IsBotAI())
            return reinterpret_cast<PlayerbotMgr*>(itr->second);
    }

    return nullptr;
}
