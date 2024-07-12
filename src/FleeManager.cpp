/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FleeManager.h"  // Include the header file for FleeManager class
#include "Playerbots.h"   // Include the header file for Playerbots
#include "ServerFacade.h" // Include the header file for ServerFacade

// Constructor for FleeManager class, initializing member variables
FleeManager::FleeManager(Player* bot, float maxAllowedDistance, float followAngle, bool forceMaxDistance, WorldPosition startPosition) :
    bot(bot), maxAllowedDistance(maxAllowedDistance), followAngle(followAngle), forceMaxDistance(forceMaxDistance), 
    startPosition(startPosition ? startPosition : WorldPosition(bot)) // If startPosition is not provided, use bot's current position
{
}

// Calculate distance from a given FleePoint to all possible enemy units
void FleeManager::calculateDistanceToCreatures(FleePoint *point)
{
    point->minDistance = -1.0f; // Initialize minimum distance as -1 (indicating no distance calculated yet)
    point->sumDistance = 0.0f;  // Initialize sum of distances to 0

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot); // Get the AI for the bot
    if (!botAI) {
        return; // If no AI is available, return
    }

    // Get a list of possible target units that do not have line of sight to the bot
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los");
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i); // Get the unit corresponding to the GUID
        if (!unit)
            continue; // If the unit is null, skip to the next

        // Calculate the 2D distance from the unit to the point
        float d = sServerFacade->GetDistance2d(unit, point->x, point->y);
        point->sumDistance += d; // Add this distance to the sum of distances

        // Update the minimum distance if necessary
        if (point->minDistance < 0 || point->minDistance > d)
            point->minDistance = d;
    }
}

// Check if an angle intersects with any of the existing angles within a specified increment
bool intersectsOri(float angle, std::vector<float>& angles, float angleIncrement)
{
    for (std::vector<float>::iterator i = angles.begin(); i != angles.end(); ++i)
    {
        float ori = *i; // Get the current angle from the list
        if (abs(angle - ori) < angleIncrement) // Check if the difference between angles is less than the increment
            return true; // If so, return true indicating an intersection
    }

    return false; // No intersection found
}

// Calculate all possible destinations for fleeing and populate the points vector
void FleeManager::calculatePossibleDestinations(std::vector<FleePoint*> &points)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot); // Get the AI for the bot
    if (!botAI) {
        return; // If no AI is available, return
    }
    Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target"); // Get the current target of the bot

    // Get the bot's current position
    float botPosX = startPosition.getX();
    float botPosY = startPosition.getY();
    float botPosZ = startPosition.getZ();

    // Create a starting flee point at the bot's position and calculate distances to creatures
    FleePoint start(botAI, botPosX, botPosY, botPosZ);
    calculateDistanceToCreatures(&start);

    // Collect angles to all enemy units
    std::vector<float> enemyOri;
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los");
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i); // Get the unit corresponding to the GUID
        if (!unit)
            continue; // If the unit is null, skip to the next

        float ori = bot->GetAngle(unit); // Get the angle between the bot and the unit
        enemyOri.push_back(ori); // Add this angle to the list of enemy angles
    }

    // Determine distance increment based on configuration
    float distIncrement = std::max(sPlayerbotAIConfig->followDistance, 
                                   (maxAllowedDistance - sPlayerbotAIConfig->tooCloseDistance) / 10.0f);
    for (float dist = maxAllowedDistance; dist >= sPlayerbotAIConfig->tooCloseDistance ; dist -= distIncrement)
    {
        // Determine angle increment based on distance
        float angleIncrement = std::max(M_PI / 20, M_PI / 4 / (1.0 + dist - sPlayerbotAIConfig->tooCloseDistance));
        for (float add = 0.0f; add < M_PI / 4 + angleIncrement; add += angleIncrement)
        {
            for (float angle = add; angle < add + 2 * static_cast<float>(M_PI) + angleIncrement; angle += static_cast<float>(M_PI) / 4)
            {
                if (intersectsOri(angle, enemyOri, angleIncrement))
                    continue; // If the angle intersects with any enemy orientation, skip it

                // Calculate potential new position based on angle and distance
                float x = botPosX + cos(angle) * maxAllowedDistance, 
                      y = botPosY + sin(angle) * maxAllowedDistance, 
                      z = botPosZ + CONTACT_DISTANCE;
                if (forceMaxDistance && sServerFacade->IsDistanceLessThan(sServerFacade->GetDistance2d(bot, x, y), 
                                                                          maxAllowedDistance - sPlayerbotAIConfig->tooCloseDistance))
                    continue; // If forced maximum distance is not met, skip this position

                // Update the Z coordinate to ensure it is valid
                bot->UpdateAllowedPositionZ(x, y, z);

                // Check if the position is in water
                Map* map = startPosition.getMap();
                if (map && map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
                    continue; // If the position is in water, skip it

                // Check line of sight for the bot and its target
                if (!bot->IsWithinLOS(x, y, z) || (target && !target->IsWithinLOS(x, y, z)))
                    continue; // If the bot or its target do not have line of sight to the position, skip it

                // Create a new flee point at the calculated position
                FleePoint* point = new FleePoint(botAI, x, y, z);
                calculateDistanceToCreatures(point); // Calculate distances to creatures for the new point

                // Check if the new point is sufficiently far from enemies
                if (sServerFacade->IsDistanceGreaterOrEqualThan(point->minDistance - start.minDistance, sPlayerbotAIConfig->followDistance))
                    points.push_back(point); // If so, add the point to the list of possible points
                else
                    delete point; // Otherwise, delete the point
            }
        }
    }
}

// Cleanup and delete all FleePoint objects in the points vector
void FleeManager::cleanup(std::vector<FleePoint*> &points)
{
    for (std::vector<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
        delete *i; // Delete each FleePoint object
    }

    points.clear(); // Clear the vector
}

// Compare two FleePoints to determine if one is better than the other
bool FleeManager::isBetterThan(FleePoint* point, FleePoint* other)
{
    return point->sumDistance - other->sumDistance > 0; // Return true if the point has a greater sum distance than the other
}

// Select the optimal destination from a vector of FleePoints
FleePoint* FleeManager::selectOptimalDestination(std::vector<FleePoint*> &points)
{
    FleePoint* best = nullptr; // Initialize the best point as null
    for (std::vector<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
        FleePoint* point = *i;
        if (!best || isBetterThan(point, best))
            best = point; // Update the best point if the current point is better
    }

    return best; // Return the best point
}

// Calculate the destination coordinates for fleeing
bool FleeManager::CalculateDestination(float* rx, float* ry, float* rz)
{
    std::vector<FleePoint*> points; // Vector to hold possible flee points
    calculatePossibleDestinations(points); // Calculate possible destinations

    FleePoint* point = selectOptimalDestination(points); // Select the optimal destination
    if (!point)
    {
        cleanup(points); // If no point is found, clean up and return false
        return false;
    }

    // Set the output coordinates to the selected point's position
    *rx = point->x;
    *ry = point->y;
    *rz = point->z;

    cleanup(points); // Clean up the points vector
    return true; // Return true indicating a valid destination was found
}

// Determine if fleeing is useful based on the proximity of enemies
bool FleeManager::isUseful()
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot); // Get the AI for the bot
    if (!botAI) {
        return false; // If no AI is available, return false
    }

    // Get a list of possible target units that do not have line of sight to the bot
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los");
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Creature* creature = botAI->GetCreature(*i); // Get the creature corresponding to the GUID
        if (!creature)
            continue; // If the creature is null, skip to the next

        // Check if the creature is within its attack distance from the starting position
        if (startPosition.sqDistance(WorldPosition(creature)) < creature->GetAttackDistance(bot) * creature->GetAttackDistance(bot))
            return true; // If so, fleeing is useful

        // float d = sServerFacade->GetDistance2d(unit, bot);
        // if (sServerFacade->IsDistanceLessThan(d, sPlayerbotAIConfig->aggroDistance)) return true;
    }

    return false; // No creatures are close enough to make fleeing useful
}
