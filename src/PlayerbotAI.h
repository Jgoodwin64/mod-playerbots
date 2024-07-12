/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAI_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PLAYERbotAI_H

#include "Chat.h"                   // Include for chat handling
#include "ChatHelper.h"             // Include for chat helper utilities
#include "ChatFilter.h"             // Include for chat filtering functionality
#include "Common.h"                 // Include for common definitions and utilities
#include "Event.h"                  // Include for event handling
#include "Item.h"                   // Include for item handling
#include "PlayerbotAIBase.h"        // Include for base class of Playerbot AI
#include "PlayerbotAIConfig.h"      // Include for Playerbot AI configuration
#include "PlayerbotSecurity.h"      // Include for Playerbot security handling
#include "SpellAuras.h"             // Include for spell auras handling
#include "WorldPacket.h"            // Include for world packet handling
#include "PlayerbotTextMgr.h"       // Include for Playerbot text manager

#include <stack>                    // Include for stack container
#include <queue>                    // Include for queue container

// Forward declarations for various classes used within PlayerbotAI
class AiObjectContext;
class Creature;
class Engine;
class ExternalEventHelper;
class Gameobject;
class Item;
class ObjectGuid;
class Player;
class PlayerbotMgr;
class Spell;
class SpellInfo;
class Unit;
class WorldObject;
class WorldPosition;

// Forward declarations for data structures used within PlayerbotAI
struct CreatureData;
struct GameObjectData;

// Enumeration for different strategy types
enum StrategyType : uint32;

// Enumeration for different healing item display IDs
enum HealingItemDisplayId
{
    HEALTHSTONE_DISPLAYID            = 8026,
    MAJOR_HEALING_POTION             = 24152,
    WHIPPER_ROOT_TUBER               = 21974,
    NIGHT_DRAGON_BREATH              = 21975,
    LIMITED_INVULNERABILITY_POTION   = 24213,
    GREATER_DREAMLESS_SLEEP_POTION   = 17403,
    SUPERIOR_HEALING_POTION          = 15714,
    CRYSTAL_RESTORE                  = 2516,
    DREAMLESS_SLEEP_POTION           = 17403,
    GREATER_HEALING_POTION           = 15713,
    HEALING_POTION                   = 15712,
    LESSER_HEALING_POTION            = 15711,
    DISCOLORED_HEALING_POTION        = 15736,
    MINOR_HEALING_POTION             = 15710,
    VOLATILE_HEALING_POTION          = 24212,
    SUPER_HEALING_POTION             = 37807,
    CRYSTAL_HEALING_POTION           = 47132,
    FEL_REGENERATION_POTION          = 37864,
    MAJOR_DREAMLESS_SLEEP_POTION     = 37845
};

// Enumeration for different bot states
enum BotState
{
    BOT_STATE_COMBAT        = 0,   // Bot is in combat state
    BOT_STATE_NON_COMBAT    = 1,   // Bot is in non-combat state
    BOT_STATE_DEAD          = 2,   // Bot is dead

    BOT_STATE_MAX                  // Maximum value for bot states
};

// Function to check if a given race belongs to the Alliance faction
bool IsAlliance(uint8 race);

// Class for handling chat commands for Playerbot
class PlayerbotChatHandler: protected ChatHandler
{
    public:
        // Constructor that initializes the handler with a master player
        explicit PlayerbotChatHandler(Player* pMasterPlayer);
        // Function to send a system message
        void sysmessage(std::string const str) { SendSysMessage(str.c_str()); }
        // Function to extract a quest ID from a string
        uint32 extractQuestId(std::string const str);
        // Function to extract a spell ID from a string
        uint32 extractSpellId(std::string const str)
        {
            char* source = (char*)str.c_str();
            return extractSpellIdFromLink(source);
        }
};

// Class for calculating the minimum value
class MinValueCalculator
{
    public:
        // Constructor initializing with a default value
        MinValueCalculator(float def = 0.0f) : param(nullptr), minValue(def) { }

        // Function to probe a value and update the minimum value and associated parameter
        void probe(float value, void* p)
        {
            if (!param || minValue >= value)
            {
                minValue = value;
                param = p;
            }
        }

        void* param;  // Pointer to the parameter associated with the minimum value
        float minValue;  // Minimum value
};

// Enumeration for different rogue poison display IDs
enum RoguePoisonDisplayId
{
    DEADLY_POISON_DISPLAYID      = 13707,
    INSTANT_POISON_DISPLAYID     = 13710,
    WOUND_POISON_DISPLAYID       = 37278
};

// Enumeration for different sharpening stone display IDs
enum SharpeningStoneDisplayId
{
    ROUGH_SHARPENING_DISPLAYID       = 24673,
    COARSE_SHARPENING_DISPLAYID      = 24674,
    HEAVY_SHARPENING_DISPLAYID       = 24675,
    SOLID_SHARPENING_DISPLAYID       = 24676,
    DENSE_SHARPENING_DISPLAYID       = 24677,
    CONSECRATED_SHARPENING_DISPLAYID = 24674,    // will not be used because bot can not know if it will face undead targets
    ELEMENTAL_SHARPENING_DISPLAYID   = 21072,
    FEL_SHARPENING_DISPLAYID         = 39192,
    ADAMANTITE_SHARPENING_DISPLAYID  = 39193
};

// Enumeration for different weight stone display IDs
enum WeightStoneDisplayId
{
    ROUGH_WEIGHTSTONE_DISPLAYID      = 24683,
    COARSE_WEIGHTSTONE_DISPLAYID     = 24684,
    HEAVY_WEIGHTSTONE_DISPLAYID      = 24685,
    SOLID_WEIGHTSTONE_DISPLAYID      = 24686,
    DENSE_WEIGHTSTONE_DISPLAYID      = 24687,
    FEL_WEIGHTSTONE_DISPLAYID        = 39548,
    ADAMANTITE_WEIGHTSTONE_DISPLAYID = 39549
};

// Enumeration for different wizard oil display IDs
enum WizardOilDisplayId
{
    MINOR_WIZARD_OIL         = 9731,
    LESSER_WIZARD_OIL        = 47903,
    BRILLIANT_WIZARD_OIL     = 47901,
    WIZARD_OIL               = 47905,
    SUPERIOR_WIZARD_OIL      = 47904,
  /// Blessed Wizard Oil    = 26865 //scourge inv
};

// Enumeration for different mana oil display IDs
enum ManaOilDisplayId
{
    MINOR_MANA_OIL           = 34492,
    LESSER_MANA_OIL          = 47902,
    BRILLIANT_MANA_OIL       = 41488,
    SUPERIOR_MANA_OIL        = 36862
};

// Enumeration for different shield ward display IDs
enum ShieldWardDisplayId
{
    LESSER_WARD_OFSHIELDING  = 38759,
    GREATER_WARD_OFSHIELDING = 38760
};

// Enumeration class for different bot type numbers
enum class BotTypeNumber : uint8
{
    ACTIVITY_TYPE_NUMBER = 1,
    GROUPER_TYPE_NUMBER  = 2,
    GUILDER_TYPE_NUMBER  = 3,
};

// Enumeration class for different grouper types
enum class GrouperType : uint8
{
    SOLO        = 0,  // Solo player
    MEMBER      = 1,  // Group member
    LEADER_2    = 2,  // Leader of a 2-player group
    LEADER_3    = 3,  // Leader of a 3-player group
    LEADER_4    = 4,  // Leader of a 4-player group
    LEADER_5    = 5   // Leader of a 5-player group
};

// Enumeration class for different guilder types
enum class GuilderType : uint8
{
    SOLO   = 0,    // Solo player
    TINY   = 30,   // Tiny guild
    SMALL  = 50,   // Small guild
    MEDIUM = 70,   // Medium guild
    LARGE  = 120,  // Large guild
    VERY_LARGE = 250 // Very large guild
};

// Enumeration for different activity types
enum ActivityType
{
    GRIND_ACTIVITY          = 1,
    RPG_ACTIVITY            = 2,
    TRAVEL_ACTIVITY         = 3,
    OUT_OF_PARTY_ACTIVITY   = 4,
    PACKET_ACTIVITY         = 5,
    DETAILED_MOVE_ACTIVITY  = 6,
    PARTY_ACTIVITY          = 7,
    ALL_ACTIVITY            = 8,

    MAX_ACTIVITY_TYPE            // Maximum value for activity types
};

// Enumeration for different bot roles
enum BotRoles : uint8
{
    BOT_ROLE_NONE   = 0x00, // No role
    BOT_ROLE_TANK   = 0x01, // Tank role
    BOT_ROLE_HEALER = 0x02, // Healer role
    BOT_ROLE_DPS    = 0x04  // DPS role
};

// Enumeration for different hunter talent tabs
enum HUNTER_TABS {
    HUNTER_TAB_BEASTMASTER,
    HUNTER_TAB_MARKSMANSHIP,
    HUNTER_TAB_SURVIVAL,
};

// Enumeration for different rogue talent tabs
enum ROGUE_TABS {
    ROGUE_TAB_ASSASSINATION,
    ROGUE_TAB_COMBAT,
    ROGUE_TAB_SUBTLETY
};

// Enumeration for different priest talent tabs
enum PRIEST_TABS {
    PRIEST_TAB_DISIPLINE,
    PRIEST_TAB_HOLY,
    PRIEST_TAB_SHADOW,
};

// Enumeration for different death knight talent tabs
enum DEATHKNIGT_TABS {
    DEATHKNIGT_TAB_BLOOD,
    DEATHKNIGT_TAB_FROST,
    DEATHKNIGT_TAB_UNHOLY,
};

// Enumeration for different druid talent tabs
enum DRUID_TABS {
    DRUID_TAB_BALANCE,
    DRUID_TAB_FERAL,
    DRUID_TAB_RESTORATION,
};

// Enumeration for different mage talent tabs
enum MAGE_TABS {
    MAGE_TAB_ARCANE,
    MAGE_TAB_FIRE,
    MAGE_TAB_FROST,
};

// Enumeration for different shaman talent tabs
enum SHAMAN_TABS {
    SHAMAN_TAB_ELEMENTAL,
    SHAMAN_TAB_ENHANCEMENT,
    SHAMAN_TAB_RESTORATION,
};

// Enumeration for different paladin talent tabs
enum PALADIN_TABS {
    PALADIN_TAB_HOLY,
    PALADIN_TAB_PROTECTION,
    PALADIN_TAB_RETRIBUTION,
};

// Enumeration for different warlock talent tabs
enum WARLOCK_TABS {
    WARLOCK_TAB_AFFLICATION,
    WARLOCK_TAB_DEMONOLOGY,
    WARLOCK_TAB_DESTRUCTION,
};

// Enumeration for different warrior talent tabs
enum WARRIOR_TABS {
    WARRIOR_TAB_ARMS,
    WARRIOR_TAB_FURY,
    WARRIOR_TAB_PROTECTION,
};

// Class for handling packet operations for Playerbot
class PacketHandlingHelper
{
    public:
        // Add a handler for a specific opcode
        void AddHandler(uint16 opcode, std::string const handler);
        // Handle an external event
        void Handle(ExternalEventHelper &helper);
        // Add a packet to the queue
        void AddPacket(WorldPacket const& packet);

    private:
        std::map<uint16, std::string> handlers;  // Map of handlers by opcode
        std::stack<WorldPacket> queue;           // Stack of packets to be handled
};

// Class for holding chat commands
class ChatCommandHolder
{
    public:
        // Constructor initializing with a command, owner, type, and time
        ChatCommandHolder(std::string const command, Player* owner = nullptr, uint32 type = CHAT_MSG_WHISPER, time_t time = 0) : command(command), owner(owner), type(type), time(time) { }
        // Copy constructor
        ChatCommandHolder(ChatCommandHolder const& other) : command(other.command), owner(other.owner), type(other.type), time(other.time) { }

        // Get the command string
        std::string const GetCommand() { return command; }
        // Get the owner of the command
        Player* GetOwner() { return owner; }
        // Get the type of the command
        uint32 GetType() { return type; }
        // Get the time the command was issued
        time_t GetTime() { return time; }

    private:
        std::string const command;  // Command string
        Player* owner;              // Owner of the command
        uint32 type;                // Type of the command
        time_t time;                // Time the command was issued
};

// Main class for Playerbot AI
class PlayerbotAI : public PlayerbotAIBase
{
    public:
        // Default constructor
        PlayerbotAI();
        // Constructor initializing with a bot player
        PlayerbotAI(Player* bot);
        // Virtual destructor
        virtual ~PlayerbotAI();

        // Update the AI state
        void UpdateAI(uint32 elapsed, bool minimal = false) override;
        // Internal function to update the AI state
        void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;

        // Handle a remote command
        std::string const HandleRemoteCommand(std::string const command);
        // Handle a command from a player
        void HandleCommand(uint32 type, std::string const text, Player* fromPlayer);
        // Queue a chat response
        void QueueChatResponse(uint8 msgtype, ObjectGuid guid1, ObjectGuid guid2, std::string message, std::string chanName, std::string name);
        // Handle an outgoing packet from the bot
        void HandleBotOutgoingPacket(WorldPacket const& packet);
        // Handle an incoming packet from the master
        void HandleMasterIncomingPacket(WorldPacket const& packet);
        // Handle an outgoing packet from the master
        void HandleMasterOutgoingPacket(WorldPacket const& packet);
        // Handle a teleport acknowledgment
        void HandleTeleportAck();
        // Change the current engine based on bot state
        void ChangeEngine(BotState type);
        // Perform the next action in the AI
        void DoNextAction(bool minimal = false);
        // Perform a specific action
        virtual bool DoSpecificAction(std::string const name, Event event = Event(), bool silent = false, std::string const qualifier = "");
        // Change the strategy for a given bot state
        void ChangeStrategy(std::string const name, BotState type);
        // Clear strategies for a given bot state
        void ClearStrategies(BotState type);
        // Get the strategies for a given bot state
        std::vector<std::string> GetStrategies(BotState type);
        // Check if a strategy is contained within the current strategies
        bool ContainsStrategy(StrategyType type);
        // Check if a specific strategy is active for a given bot state
        bool HasStrategy(std::string const name, BotState type);
        // Get the current bot state
        BotState GetState() { return currentState; };
        // Reset the strategies for the bot
        void ResetStrategies(bool load = false);
        // Reinitialize the current engine
        void ReInitCurrentEngine();
        // Reset the bot state
        void Reset(bool full = false);
        // Static function to check if a player is a tank
        static bool IsTank(Player* player);
        // Static function to check if a player is a healer
        static bool IsHeal(Player* player);
        // Static function to check if a player is a DPS
        static bool IsDps(Player* player);
        // Static function to check if a player is ranged
        static bool IsRanged(Player* player);
        // Static function to check if a player is melee
        static bool IsMelee(Player* player);
        // Static function to check if a player is a caster
        static bool IsCaster(Player* player);
        // Static function to check if a player is a combo class
        static bool IsCombo(Player* player);
        // Static function to check if a player is ranged DPS
        static bool IsRangedDps(Player* player);
        // Static function to check if a player is the main tank
        static bool IsMainTank(Player* player);
        // Check if a player is an assist tank
        bool IsAssistTank(Player* player);
        // Check if a player is an assist tank of a specific index
        bool IsAssistTankOfIndex(Player* player, int index);
        // Check if a player is a heal assistant of a specific index
        bool IsHealAssistantOfIndex(Player* player, int index);
        // Check if a player is a ranged DPS assistant of a specific index
        bool IsRangedDpsAssistantOfIndex(Player* player, int index);
        // Check if the bot has aggro from a unit
        bool HasAggro(Unit* unit);
        // Get the group slot index for a player
        int32 GetGroupSlotIndex(Player* player);
        // Get the ranged index for a player
        int32 GetRangedIndex(Player* player);
        // Get the class index for a player
        int32 GetClassIndex(Player* player, uint8_t cls);
        // Get the ranged DPS index for a player
        int32 GetRangedDpsIndex(Player* player);
        // Get the melee index for a player
        int32 GetMeleeIndex(Player* player);

        // Get a creature by GUID
        Creature* GetCreature(ObjectGuid guid);
        // Get a unit by GUID
        Unit* GetUnit(ObjectGuid guid);
        // Get a player by GUID
        Player* GetPlayer(ObjectGuid guid);
        // Static function to get a unit from creature data
        static Unit* GetUnit(CreatureData const* creatureData);
        // Get a game object by GUID
        GameObject* GetGameObject(ObjectGuid guid);
        // static GameObject* GetGameObject(GameObjectData const* gameObjectData);  
        // Get a world object by GUID
        WorldObject* GetWorldObject(ObjectGuid guid);
        // Send a message to the master
        bool TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        // Send a message to the master
        bool TellMaster(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        // Send a message to the master without facing
        bool TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        // Send a message to the master without facing
        bool TellMasterNoFacing(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        // Send an error message
        bool TellError(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        // Handle spell interruption
        void SpellInterrupted(uint32 spellid);
        // Calculate the global cooldown for a spell
        int32 CalculateGlobalCooldown(uint32 spellid);
        // Interrupt the current spell
        void InterruptSpell();
        // Remove an aura by name
        void RemoveAura(std::string const name);
        // Remove the current shapeshift form
        void RemoveShapeshift();
        // Wait for a spell to finish casting
        void WaitForSpellCast(Spell* spell);
        // Play a sound
        bool PlaySound(uint32 emote);
        // Play an emote
        bool PlayEmote(uint32 emote);
        // Ping a location
        void Ping(float x, float y);
        // Find a poison item
        Item* FindPoison() const;
        // Find a bandage item
        Item* FindBandage() const;
        // Find a consumable item by display ID
        Item* FindConsumable(uint32 displayId) const;
        // Find a stone item for a weapon
        Item* FindStoneFor(Item* weapon) const;
        // Find an oil item for a weapon
        Item* FindOilFor(Item* weapon) const;
        // Imbue an item with a target flag and GUID
        void ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID);
        // Imbue an item with a target inventory slot
        void ImbueItem(Item* item, uint8 targetInventorySlot);
        // Imbue an item with a target unit
        void ImbueItem(Item* item, Unit* target);
        // Imbue an item
        void ImbueItem(Item* item);
        // Enchant an item in a specific slot
        void EnchantItemT(uint32 spellid, uint8 slot);
        // Get the count of players buffed with a specific spell
        uint32 GetBuffedCount(Player* player, std::string const spellname);

        // Virtual function to check if a spell can be cast
        virtual bool CanCastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
        // Virtual function to cast a spell
        virtual bool CastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
        // Virtual function to check if a unit has a specific aura
        virtual bool HasAura(std::string const spellName, Unit* player, bool maxStack = false, bool checkIsOwner = false, int maxAmount = -1, bool checkDuration = false);
        // Virtual function to check if a unit has any aura of a specified list
        virtual bool HasAnyAuraOf(Unit* player, ...);

        // Virtual function to check if a spell can be interrupted
        virtual bool IsInterruptableSpellCasting(Unit* player, std::string const spell);
        // Virtual function to check if a unit has an aura to dispel
        virtual bool HasAuraToDispel(Unit* player, uint32 dispelType);
        // Function to check if a spell can be cast with various parameters
        bool CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell = true, Item* itemTarget = nullptr, Item* castItem = nullptr);
        // Function to check if a spell can be cast on a game object
        bool CanCastSpell(uint32 spellid, GameObject* goTarget, uint8 effectMask, bool checkHasSpell = true);
        // Function to check if a spell can be cast at specific coordinates
        bool CanCastSpell(uint32 spellid, float x, float y, float z, uint8 effectMask, bool checkHasSpell = true, Item* itemTarget = nullptr);

        // Function to check if a unit has a specific aura
        bool HasAura(uint32 spellId, Unit const* player);
        // Get an aura by spell name
        Aura* GetAura(std::string const spellName, Unit* unit, bool checkIsOwner = false, bool checkDuration = false, int checkStack = -1);
        // Function to cast a spell on a unit
        bool CastSpell(uint32 spellId, Unit* target, Item* itemTarget = nullptr);
        // Function to cast a spell at specific coordinates
        bool CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget = nullptr);
        // Function to check if a spell can dispel a specific type
        bool canDispel(SpellInfo const* spellInfo, uint32 dispelType);

        // Function to check if a vehicle spell can be cast on a unit
        bool CanCastVehicleSpell(uint32 spellid, Unit* target);
        // Function to cast a vehicle spell on a unit
        bool CastVehicleSpell(uint32 spellId, Unit* target);
        // Function to cast a vehicle spell at specific coordinates
        bool CastVehicleSpell(uint32 spellId, float x, float y, float z);
        // Function to check if the bot is in a vehicle with various conditions
        bool IsInVehicle(bool canControl = false, bool canCast = false, bool canAttack = false, bool canTurn = false, bool fixed = false);

        // Get the equipment gear score for a player
        uint32 GetEquipGearScore(Player* player, bool withBags, bool withBank);
        // Static function to get the mixed gear score for a player
        static uint32 GetMixedGearScore(Player* player, bool withBags, bool withBank, uint32 topN = 0);
        // Check if a player has a specific skill
        bool HasSkill(SkillType skill);
        // Check if a command is allowed
        bool IsAllowedCommand(std::string const text);
        // Get the range for a specific type
        float GetRange(std::string const type);

        // Get the bot player
        Player* GetBot() { return bot; }
        // Get the master player
        Player* GetMaster() { return master; }

        //Checks if the bot is really a player. Players always have themselves as master
        bool IsRealPlayer() { return master ? (master == bot) : false; }
        // Check if the bot has a real player master
        bool HasRealPlayerMaster();
        // Check if the bot has an active player master
        bool HasActivePlayerMaster();
        // Get the group leader or master of the bot
        Player* GetGroupMaster();
        //Checks if the bot is summoned as alt of a player
        bool IsAlt();
        //Returns a semi-random (cycling) number that is fixed for each bot.
        uint32 GetFixedBotNumer(BotTypeNumber typeNumber, uint32 maxNum = 100, float cyclePerMin = 1);
        // Get the grouper type for the bot
        GrouperType GetGrouperType();
        // Get the guilder type for the bot
        GuilderType GetGuilderType();
        // Check if there are players nearby a specific position
        bool HasPlayerNearby(WorldPosition* pos, float range = sPlayerbotAIConfig->reactDistance);
        // Check if there are players nearby
        bool HasPlayerNearby(float range = sPlayerbotAIConfig->reactDistance);
        // Check if there are many players nearby
        bool HasManyPlayersNearby(uint32 trigerrValue = 20, float range = sPlayerbotAIConfig->sightDistance);
        // Check if an activity type is allowed to be active
        bool AllowActive(ActivityType activityType);
        // Check if an activity is allowed
        bool AllowActivity(ActivityType activityType = ALL_ACTIVITY, bool checkNow = false);

        // Check if a specific cheat is enabled
        bool HasCheat(BotCheatMask mask) { return ((uint32)mask & (uint32)cheatMask) != 0 || ((uint32)mask & (uint32)sPlayerbotAIConfig->botCheatMask) != 0; }
        // Get the current cheat mask
        BotCheatMask GetCheat() { return cheatMask; }
        // Set the cheat mask
        void SetCheat(BotCheatMask mask) { cheatMask = mask; }

        // Set the master player
        void SetMaster(Player* newMaster) { master = newMaster; }
        // Get the AI object context
        AiObjectContext* GetAiObjectContext() { return aiObjectContext; }
        // Get the chat helper
        ChatHelper* GetChatHelper() { return &chatHelper; }
        // Check if a player is opposing
        bool IsOpposing(Player* player);
        // Static function to check if two races are opposing
        static bool IsOpposing(uint8 race1, uint8 race2);
        // Get the Playerbot security object
        PlayerbotSecurity* GetSecurity() { return &security; }

        // Get the jump destination position
        Position GetJumpDestination() { return jumpDestination; }
        // Set the jump destination position
        void SetJumpDestination(Position pos) { jumpDestination = pos; }
        // Reset the jump destination position
        void ResetJumpDestination() { jumpDestination = Position(); }

        // Check if the bot can move
        bool CanMove();
        // Check if the bot is in a real guild
        bool IsInRealGuild();
        // Static vector of strings for dispel whitelist
        static std::vector<std::string> dispel_whitelist;
        // Check if two strings are equal ignoring case
        bool EqualLowercaseName(std::string s1, std::string s2);
        // Check if an item can be equipped
        InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading = true) const;
        // Find the equip slot for an item template
        uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const;
    private:
        // Fill gear score data for a player
        static void _fillGearScoreData(Player* player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore, bool mixed = false);
        // Check if telling the master is allowed
        bool IsTellAllowed(PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    protected:
        Player* bot;                            // Pointer to the bot player
        Player* master;                         // Pointer to the master player
        uint32 accountId;                       // Account ID of the bot
        AiObjectContext* aiObjectContext;       // AI object context
        Engine* currentEngine;                  // Current engine
        Engine* engines[BOT_STATE_MAX];         // Array of engines for different bot states
        BotState currentState;                  // Current bot state
        ChatHelper chatHelper;                  // Chat helper object
        std::queue<ChatCommandHolder> chatCommands;  // Queue of chat commands
        std::queue<ChatQueuedReply> chatReplies;     // Queue of chat replies
        PacketHandlingHelper botOutgoingPacketHandlers;  // Helper for handling outgoing packets from the bot
        PacketHandlingHelper masterIncomingPacketHandlers; // Helper for handling incoming packets from the master
        PacketHandlingHelper masterOutgoingPacketHandlers; // Helper for handling outgoing packets from the master
        CompositeChatFilter chatFilter;         // Composite chat filter
        PlayerbotSecurity security;             // Playerbot security object
        std::map<std::string, time_t> whispers; // Map of whispers by string and time
        std::pair<ChatMsg, time_t> currentChat; // Current chat message and time
        static std::set<std::string> unsecuredCommands; // Set of unsecured commands
        bool allowActive[MAX_ACTIVITY_TYPE];    // Array to allow activities
        time_t allowActiveCheckTimer[MAX_ACTIVITY_TYPE]; // Array of timers for activity checks
        bool inCombat = false;                  // Flag for combat state
        BotCheatMask cheatMask = BotCheatMask::none; // Current cheat mask
        Position jumpDestination = Position();  // Jump destination position
};

#endif  // End of header guard
