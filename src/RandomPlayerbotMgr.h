/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTMGR_H
#define _PLAYERBOT_RANDOMPLAYERBOTMGR_H

#include "PlayerbotMgr.h"

class ChatHandler;
class PerformanceMonitorOperation;
class WorldLocation;

class CachedEvent
{
    public:
        CachedEvent() : value(0), lastChangeTime(0), validIn(0), data("") { }
        CachedEvent(const CachedEvent& other) : value(other.value), lastChangeTime(other.lastChangeTime), validIn(other.validIn), data(other.data) { }
        CachedEvent(uint32 value, uint32 lastChangeTime, uint32 validIn, std::string const data = "") : value(value), lastChangeTime(lastChangeTime), validIn(validIn), data(data) { }

        bool IsEmpty() { return !lastChangeTime; }

    public:
        uint32 value;              // Cached value
        uint32 lastChangeTime;     // Timestamp of the last change
        uint32 validIn;            // Validity duration of the cached data
        std::string data;          // Additional data as a string
};

//https://gist.github.com/bradley219/5373998

class botPIDImpl;
class botPID
{
public:
    // Kp -  proportional gain
    // Ki -  Integral gain
    // Kd -  derivative gain
    // dt -  loop interval time
    // max - maximum value of manipulated variable
    // min - minimum value of manipulated variable
    botPID(double dt, double max, double min, double Kp, double Ki, double Kd);
    void adjust(double Kp, double Ki, double Kd);
    void reset();

    double calculate(double setpoint, double pv);
    ~botPID();

private:
    botPIDImpl* pimpl;  // Pointer to implementation class
};

class RandomPlayerbotMgr : public PlayerbotHolder
{
    public:
        RandomPlayerbotMgr();
        virtual ~RandomPlayerbotMgr();
        static RandomPlayerbotMgr* instance()
        {
            static RandomPlayerbotMgr instance;
            return &instance;
        }

        void LogPlayerLocation();  // Log the location of players
        void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;  // Update AI internals

    private:
        void ScaleBotActivity();   // Scale the activity of bots

    public:
        uint32 activeBots = 0;  // Number of active bots
        static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args);  // Handle console commands
        bool IsRandomBot(Player* bot);  // Check if a bot is a random bot
        bool IsRandomBot(ObjectGuid::LowType bot);  // Check if a bot by its GUID is a random bot
        void Randomize(Player* bot);  // Randomize bot attributes
        void Clear(Player* bot);  // Clear bot data
        void RandomizeFirst(Player* bot);  // Randomize first bot
        void RandomizeMin(Player* bot);  // Randomize minimum bot attributes
        void IncreaseLevel(Player* bot);  // Increase bot level
        void ScheduleTeleport(uint32 bot, uint32 time = 0);  // Schedule teleport for a bot
        void ScheduleChangeStrategy(uint32 bot, uint32 time = 0);  // Schedule strategy change for a bot
        void HandleCommand(uint32 type, std::string const text, Player* fromPlayer, std::string channelName = "");  // Handle commands from players
        std::string const HandleRemoteCommand(std::string const request);  // Handle remote commands
        void OnPlayerLogout(Player* player);  // Handle player logout
        void OnPlayerLogin(Player* player);  // Handle player login
        void OnPlayerLoginError(uint32 bot);  // Handle player login errors
        Player* GetRandomPlayer();  // Get a random player
        std::vector<Player*> GetPlayers() { return players; };  // Get all players
        PlayerBotMap GetAllBots() { return playerBots; };  // Get all bots
        void PrintStats();  // Print statistics
        double GetBuyMultiplier(Player* bot);  // Get buy multiplier for a bot
        double GetSellMultiplier(Player* bot);  // Get sell multiplier for a bot
        void AddTradeDiscount(Player* bot, Player* master, int32 value);  // Add trade discount
        void SetTradeDiscount(Player* bot, Player* master, uint32 value);  // Set trade discount
        uint32 GetTradeDiscount(Player* bot, Player* master);  // Get trade discount
        void Refresh(Player* bot);  // Refresh bot
        void RandomTeleportForLevel(Player* bot);  // Random teleport for leveling
        void RandomTeleportGrindForLevel(Player* bot);  // Random teleport for grinding
        void RandomTeleportForRpg(Player* bot);  // Random teleport for RPG
        uint32 GetMaxAllowedBotCount();  // Get maximum allowed bot count
        bool ProcessBot(Player* player);  // Process a bot
        void Revive(Player* player);  // Revive a player
        void ChangeStrategy(Player* player);  // Change bot strategy
        void ChangeStrategyOnce(Player* player);  // Change bot strategy once
        uint32 GetValue(Player* bot, std::string const type);  // Get a value for a bot
        uint32 GetValue(uint32 bot, std::string const type);  // Get a value by bot ID
        std::string const GetData(uint32 bot, std::string const type);  // Get data for a bot
        void SetValue(uint32 bot, std::string const type, uint32 value, std::string const data = "");  // Set a value for a bot
        void SetValue(Player* bot, std::string const type, uint32 value, std::string const data = "");  // Set a value for a player bot
        void Remove(Player* bot);  // Remove a bot
        ObjectGuid const GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId);  // Get BattleMaster GUID
        CreatureData const* GetCreatureDataByEntry(uint32 entry);  // Get creature data by entry
        void LoadBattleMastersCache();  // Load BattleMasters cache
        std::map<uint32, std::map<uint32, std::map<TeamId, bool>>> NeedBots;  // Bots needed for each team and battleground
        std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> BgBots;  // Bots in battlegrounds
        std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> VisualBots;  // Visual bots for battlegrounds
        std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> BgPlayers;  // Players in battlegrounds
        std::map<uint32, std::map<uint32, std::map<TeamId, std::map<TeamId, uint32>>>> ArenaBots;  // Bots in arenas
        std::map<uint32, std::map<uint32, std::map<uint32, uint32>>> Rating;  // Ratings for bots
        std::map<uint32, std::map<uint32, std::map<uint32, uint32>>> Supporters;  // Supporters for bots
        std::map<TeamId, std::vector<uint32>> LfgDungeons;  // LFG dungeons
        void CheckBgQueue();  // Check battleground queue
        void CheckLfgQueue();  // Check LFG queue
        void CheckPlayers();  // Check players

        std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> getBattleMastersCache() { return BattleMastersCache; }

        float getActivityMod() { return activityMod; }
        float getActivityPercentage() { return activityMod * 100.0f; }
        void setActivityPercentage(float percentage) { activityMod = percentage / 100.0f; }

    protected:
        void OnBotLoginInternal(Player* const bot) override;  // Internal bot login handler

    private:
        botPID pid = botPID(1, 50, -50, 0, 0, 0);  // PID controller instance with initial values
        float activityMod = 0.25;  // Activity modifier for bots
        uint32 GetEventValue(uint32 bot, std::string const event);  // Get event value for a bot
        std::string const GetEventData(uint32 bot, std::string const event);  // Get event data for a bot
        uint32 SetEventValue(uint32 bot, std::string const event, uint32 value, uint32 validIn, std::string const data = "");  // Set event value for a bot
        void GetBots();  // Get all bots
        std::vector<uint32> GetBgBots(uint32 bracket);  // Get bots in a battleground bracket
        time_t BgCheckTimer;  // Timer for battleground check
        time_t LfgCheckTimer;  // Timer for LFG check
        time_t PlayersCheckTimer;  // Timer for players check
        uint32 AddRandomBots();  // Add random bots
        bool ProcessBot(uint32 bot);  // Process a bot by its ID
        void ScheduleRandomize(uint32 bot, uint32 time);  // Schedule randomization for a bot
        void RandomTeleport(Player* bot);  // Randomly teleport a bot
        void RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth = false);  // Randomly teleport a bot to specific locations
        uint32 GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ);  // Get the level of a zone
        void PrepareTeleportCache();  // Prepare teleport cache
        typedef void(RandomPlayerbotMgr::*ConsoleCommandHandler)(Player*);  // Typedef for console command handler

        std::vector<Player*> players;  // Vector of players
        uint32 processTicks;  // Process ticks for updates
        std::map<uint8, std::vector<WorldLocation>> locsPerLevelCache;  // Location cache per level
        std::map<uint8, std::vector<WorldLocation>> bankerLocsPerLevelCache;  // Banker location cache per level

        // std::map<uint32, std::vector<WorldLocation>> rpgLocsCache;
        std::map<uint32, std::map<uint32, std::vector<WorldLocation>>> rpgLocsCacheLevel;  // RPG locations cache per level
        std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> BattleMastersCache;  // BattleMasters cache
        std::map<uint32, std::map<std::string, CachedEvent>> eventCache;  // Event cache for bots
        std::list<uint32> currentBots;  // List of current bots
        uint32 bgBotsCount;  // Count of bots in battlegrounds
        uint32 playersLevel;  // Players' level
        PerformanceMonitorOperation* totalPmo;  // Total performance monitor operation
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif

