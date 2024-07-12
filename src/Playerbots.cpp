/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "cs_playerbots.h"
#include "Channel.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "GuildTaskMgr.h"
#include "Metric.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

// Database script for playerbots
class PlayerbotsDatabaseScript : public DatabaseScript
{
    public:
        // Constructor
        PlayerbotsDatabaseScript() : DatabaseScript("PlayerbotsDatabaseScript") { }

        // Function called when databases are loading
        bool OnDatabasesLoading() override
        {
            DatabaseLoader playerbotLoader("server.playerbots");
            playerbotLoader.SetUpdateFlags(sConfigMgr->GetOption<bool>("Playerbots.Updates.EnableDatabases", true) ? DatabaseLoader::DATABASE_PLAYERBOTS : 0);
            playerbotLoader.AddDatabase(PlayerbotsDatabase, "Playerbots");

            return playerbotLoader.Load();
        }

        // Function to keep the database connection alive
        void OnDatabasesKeepAlive() override
        {
            PlayerbotsDatabase.KeepAlive();
        }

        // Function called when databases are closing
        void OnDatabasesClosing() override
        {
            PlayerbotsDatabase.Close();
        }

        // Function to warn about sync queries
        void OnDatabaseWarnAboutSyncQueries(bool apply) override
        {
            PlayerbotsDatabase.WarnAboutSyncQueries(apply);
        }

        // Function called when selecting database index on logout
        void OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam) override
        {
            statementIndex = CHAR_UPD_CHAR_ONLINE;
            statementParam = player->GetGUID().GetCounter();
        }

        // Function to get the database revision
        void OnDatabaseGetDBRevision(std::string& revision) override
        {
            if (QueryResult resultPlayerbot = PlayerbotsDatabase.Query("SELECT date FROM version_db_playerbots ORDER BY date DESC LIMIT 1"))
            {
                Field* fields = resultPlayerbot->Fetch();
                revision = fields[0].Get<std::string>();
            }

            if (revision.empty())
            {
                revision = "Unknown Playerbots Database Revision";
            }
        }
};

// Metric script for playerbots
class PlayerbotsMetricScript : public MetricScript
{
    public:
        // Constructor
        PlayerbotsMetricScript() : MetricScript("PlayerbotsMetricScript") { }

        // Function called for metric logging
        void OnMetricLogging() override
        {
            if (sMetric->IsEnabled())
            {
                sMetric->LogValue("db_queue_playerbots", uint64(PlayerbotsDatabase.QueueSize()), {});
            }
        }
};

// Player script for handling player-related events with playerbots
class PlayerbotsPlayerScript : public PlayerScript
{
    public:
        // Constructor
        PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript") { }

        // Function called when a player logs in
        void OnLogin(Player* player) override
        {
            if (!player->GetSession()->IsBot())
            {
                sPlayerbotsMgr->AddPlayerbotData(player, false);
                sRandomPlayerbotMgr->OnPlayerLogin(player);
            }
        }

        // Function called after player update
        void OnAfterUpdate(Player* player, uint32 diff) override
        {
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            {
                botAI->UpdateAI(diff);
            }

            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                playerbotMgr->UpdateAI(diff);
            }
        }

        // Function to determine if a player can use chat
        bool CanPlayerUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Player* receiver) override
        {
            if (type == CHAT_MSG_WHISPER)
            {
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(receiver))
                {
                    botAI->HandleCommand(type, msg, player);

                    return false;
                }
            }

            return true;
        }

        // Function called when a player chats in a group
        void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group) override
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                if (Player* member = itr->GetSource())
                {
                    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member))
                    {
                        botAI->HandleCommand(type, msg, player);
                    }
                }
            }
        }

        // Function called when a player chats
        void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg) override
        {
            if (type == CHAT_MSG_GUILD)
            {
                if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                {
                    for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin(); it != playerbotMgr->GetPlayerBotsEnd(); ++it)
                    {
                        if (Player* const bot = it->second)
                        {
                            if (bot->GetGuildId() == player->GetGuildId())
                            {
                                GET_PLAYERBOT_AI(bot)->HandleCommand(type, msg, player);
                            }
                        }
                    }
                }
            }
        }

        // Function called when a player chats in a channel
        void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Channel* channel) override
        {
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                if (channel->GetFlags() & 0x18)
                {
                    playerbotMgr->HandleCommand(type, msg);
                }
            }

            sRandomPlayerbotMgr->HandleCommand(type, msg, player);
        }

        // Function called before criteria progress is updated
        bool OnBeforeCriteriaProgress(Player* player, AchievementCriteriaEntry const* /*criteria*/) override
        {
            if (sRandomPlayerbotMgr->IsRandomBot(player))
            {
                return false;
            }
            return true;
        }

        // Function called before achievement completion
        bool OnBeforeAchiComplete(Player* player, AchievementEntry const* /*achievement*/) override
        {
            if (sRandomPlayerbotMgr->IsRandomBot(player))
            {
                return false;
            }
            return true;
        }
};

// Misc script for playerbots
class PlayerbotsMiscScript : public MiscScript
{
    public:
        // Constructor
        PlayerbotsMiscScript() : MiscScript("PlayerbotsMiscScript", {MISCHOOK_ON_DESTRUCT_PLAYER}) { }

        // Function called when a player is destructed
        void OnDestructPlayer(Player* player) override
        {
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            {
                delete botAI;
            }

            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                delete playerbotMgr;
            }
        }
};

// Server script for playerbots
class PlayerbotsServerScript : public ServerScript
{
    public:
        // Constructor
        PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript") { }

        // Function called when a packet is received
        void OnPacketReceived(WorldSession* session, WorldPacket const& packet) override
        {
            if (Player* player = session->GetPlayer())
                if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                    playerbotMgr->HandleMasterIncomingPacket(packet);
        }
};

// World script for playerbots
class PlayerbotsWorldScript : public WorldScript
{
    public:
        // Constructor
        PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript") { }

        // Function called before the world is initialized
        void OnBeforeWorldInitialized() override
        {
            uint32 oldMSTime = getMSTime();

            LOG_INFO("server.loading", " ");
            LOG_INFO("server.loading", "Load Playerbots Config...");

            sPlayerbotAIConfig->Initialize();

            LOG_INFO("server.loading", ">> Loaded playerbots config in {} ms", GetMSTimeDiffToNow(oldMSTime));
            LOG_INFO("server.loading", " ");
        }
};

// Script for handling various playerbot events
class PlayerbotsScript : public PlayerbotScript
{
    public:
        // Constructor
        PlayerbotsScript() : PlayerbotScript("PlayerbotsScript") { }

        // Function to check LFG queue for non-bots
        bool OnPlayerbotCheckLFGQueue(lfg::Lfg5Guids const& guidsList) override
        {
            bool nonBotFound = false;
            for (ObjectGuid const& guid : guidsList.guids)
            {
                Player* player = ObjectAccessor::FindPlayer(guid);
                if (guid.IsGroup() || (player && !GET_PLAYERBOT_AI(player)))
                {
                    nonBotFound = true;
                    break;
                }
            }

            return nonBotFound;
        }

        // Function to check kill tasks for a playerbot
        void OnPlayerbotCheckKillTask(Player* player, Unit* victim) override
        {
            if (player)
                sGuildTaskMgr->CheckKillTask(player, victim);
        }

        // Function to check petition account for a playerbot
        void OnPlayerbotCheckPetitionAccount(Player* player, bool& found) override
        {
            if (found && GET_PLAYERBOT_AI(player))
                found = false;
        }

        // Function to check updates to send for a playerbot
        bool OnPlayerbotCheckUpdatesToSend(Player* player) override
        {
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
                return botAI->IsRealPlayer();

            return true;
        }

        // Function called when a packet is sent from a playerbot
        void OnPlayerbotPacketSent(Player* player, WorldPacket const* packet) override
        {
            if (!player)
                return;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            {
                botAI->HandleBotOutgoingPacket(*packet);
            }
            else if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                playerbotMgr->HandleMasterOutgoingPacket(*packet);
            }
        }

        // Function called to update playerbot logic
        void OnPlayerbotUpdate(uint32 diff) override
        {
            sRandomPlayerbotMgr->UpdateAI(diff);
            sRandomPlayerbotMgr->UpdateSessions();
        }

        // Function called to update playerbot sessions
        void OnPlayerbotUpdateSessions(Player* player) override
        {
            if (player)
                if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                    playerbotMgr->UpdateSessions();
        }

        // Function called when a playerbot logs out
        void OnPlayerbotLogout(Player* player) override
        {
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
                if (!botAI || botAI->IsRealPlayer())
                {
                    playerbotMgr->LogoutAllBots();
                }
            }

            sRandomPlayerbotMgr->OnPlayerLogout(player);
        }

        // Function called to log out all playerbots
        void OnPlayerbotLogoutBots() override
        {
            sRandomPlayerbotMgr->LogoutAllBots();
        }
};

// Function to add all playerbot scripts to the script manager
void AddPlayerbotsScripts()
{
    new PlayerbotsDatabaseScript();
    new PlayerbotsMetricScript();
    new PlayerbotsPlayerScript();
    new PlayerbotsMiscScript();
    new PlayerbotsServerScript();
    new PlayerbotsWorldScript();
    new PlayerbotsScript();

    AddSC_playerbots_commandscript();
}
