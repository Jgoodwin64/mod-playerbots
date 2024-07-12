/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SERVERFACADE_H
#define _PLAYERBOT_SERVERFACADE_H

#include "Common.h"

// Forward declarations of classes used within ServerFacade
class Player;
class Unit;
class WorldObject;

// ServerFacade class provides various utility functions related to distances and facing directions
class ServerFacade
{
    public:
        // Default constructor
        ServerFacade() { };

        // Destructor
        virtual ~ServerFacade() { };

        // Singleton instance getter
        static ServerFacade* instance()
        {
            static ServerFacade instance;
            return &instance;
        }

    public:
        // Get the 2D distance between a Unit and a WorldObject
        float GetDistance2d(Unit* unit, WorldObject* wo);

        // Get the 2D distance between a Unit and a specified (x, y) coordinate
        float GetDistance2d(Unit* unit, float x, float y);

        // Check if the first distance is less than the second distance
        bool IsDistanceLessThan(float dist1, float dist2);

        // Check if the first distance is greater than the second distance
        bool IsDistanceGreaterThan(float dist1, float dist2);

        // Check if the first distance is greater than or equal to the second distance
        bool IsDistanceGreaterOrEqualThan(float dist1, float dist2);

        // Check if the first distance is less than or equal to the second distance
        bool IsDistanceLessOrEqualThan(float dist1, float dist2);

        // Set the facing direction of a Player to a WorldObject
        void SetFacingTo(Player* bot, WorldObject* wo, bool force = false);

        // Get the chase target of a Unit
        Unit* GetChaseTarget(Unit* target);
};

// Macro to access the singleton instance of ServerFacade
#define sServerFacade ServerFacade::instance()

#endif
