/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FLEEMANAGER_H  // Header guard start to prevent multiple inclusions
#define _PLAYERBOT_FLEEMANAGER_H

#include "Common.h"    // Include common definitions and utilities
#include "TravelMgr.h" // Include TravelMgr header file

#include <vector> // Include the vector container from the standard library

// Forward declarations
class Player;
class PlayerbotAI;

// Class representing a point to flee to
class FleePoint
{
    public:
        // Constructor for FleePoint, initializing member variables
        FleePoint(PlayerbotAI* botAI, float x, float y, float z) : botAI(botAI), sumDistance(0.0f), minDistance(0.0f), x(x), y(y), z(z) { }

        float x; // X coordinate
        float y; // Y coordinate
        float z; // Z coordinate

        float sumDistance;  // Sum of distances to all creatures
        float minDistance;  // Minimum distance to the nearest creature

    private:
        PlayerbotAI* botAI; // Pointer to the PlayerbotAI instance
};

// Class managing the fleeing logic for a player bot
class FleeManager
{
    public:
        // Constructor for FleeManager, initializing member variables
        FleeManager(Player* bot, float maxAllowedDistance, float followAngle, bool forceMaxDistance = false, WorldPosition startPosition = WorldPosition());

        // Calculate the destination coordinates for fleeing
        bool CalculateDestination(float* rx, float* ry, float* rz);
        // Determine if fleeing is useful based on the proximity of enemies
        bool isUseful();

    private:
        // Calculate possible destinations for fleeing
        void calculatePossibleDestinations(std::vector<FleePoint*> &points);
        // Calculate distance to creatures for a given FleePoint
        void calculateDistanceToCreatures(FleePoint *point);
        // Cleanup and delete all FleePoint objects in the points vector
        void cleanup(std::vector<FleePoint*> &points);
        // Select the optimal destination from a vector of FleePoints
        FleePoint* selectOptimalDestination(std::vector<FleePoint*> &points);
        // Compare two FleePoints to determine if one is better than the other
        bool isBetterThan(FleePoint* point, FleePoint* other);

        Player* bot; // Pointer to the bot (player) instance
        float maxAllowedDistance; // Maximum allowed distance for fleeing
        [[maybe_unused]] float followAngle; // Unused follow angle
        bool forceMaxDistance; // Flag to force maximum distance
        WorldPosition startPosition; // Starting position for fleeing
};

#endif // End of header guard
