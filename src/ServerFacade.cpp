/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ServerFacade.h"
#include "Playerbots.h"
#include "TargetedMovementGenerator.h"

// Calculate and return the 2D distance between a Unit and a WorldObject, rounded to one decimal place
float ServerFacade::GetDistance2d(Unit* unit, WorldObject* wo)
{
    // Ensure that neither the unit nor the world object is null
    ASSERT_NOTNULL(unit);
    ASSERT_NOTNULL(wo);

    // Get the raw distance and round it to one decimal place
    float dist = unit->GetDistance2d(wo);
    return std::round(dist * 10.0f) / 10.0f;
}

// Calculate and return the 2D distance between a Unit and a point (x, y), rounded to one decimal place
float ServerFacade::GetDistance2d(Unit *unit, float x, float y)
{
    // Get the raw distance and round it to one decimal place
    float dist = unit->GetDistance2d(x, y);
    return std::round(dist * 10.0f) / 10.0f;
}

// Check if the first distance is less than the second distance
bool ServerFacade::IsDistanceLessThan(float dist1, float dist2)
{
    // Return whether dist1 is less than dist2
    // This function originally had an unused commented section which is now replaced with a direct comparison
    return dist1 < dist2;
}

// Check if the first distance is greater than the second distance
bool ServerFacade::IsDistanceGreaterThan(float dist1, float dist2)
{
    // Return whether dist1 is greater than dist2
    // This function originally had an unused commented section which is now replaced with a direct comparison
    return dist1 > dist2;
}

// Check if the first distance is greater than or equal to the second distance
bool ServerFacade::IsDistanceGreaterOrEqualThan(float dist1, float dist2)
{
    // Return the negation of IsDistanceLessThan to determine if dist1 is greater than or equal to dist2
    return !IsDistanceLessThan(dist1, dist2);
}

// Check if the first distance is less than or equal to the second distance
bool ServerFacade::IsDistanceLessOrEqualThan(float dist1, float dist2)
{
    // Return the negation of IsDistanceGreaterThan to determine if dist1 is less than or equal to dist2
    return !IsDistanceGreaterThan(dist1, dist2);
}

// Set the orientation of the Player to face a WorldObject
void ServerFacade::SetFacingTo(Player* bot, WorldObject* wo, bool force)
{
    // Calculate the angle between the Player and the WorldObject
    float angle = bot->GetAngle(wo);

    // The following commented code checks if the bot is moving and sets the facing direction accordingly
    // if (!force && bot->isMoving())
    //     bot->SetFacingTo(bot->GetAngle(wo));
    // else
    // {
    
    // Set the bot's orientation to the calculated angle
    bot->SetOrientation(angle);
    
    // Update the bot's movement flags to reflect the new orientation
    bot->SendMovementFlagUpdate();
    
    // }
}

// Retrieve the target that the Unit is currently chasing, if any
Unit* ServerFacade::GetChaseTarget(Unit* target)
{
    // Get the current movement generator from the target's motion master
    MovementGenerator* movementGen = target->GetMotionMaster()->top();
    
    // Check if the movement generator is a chase movement generator
    if (movementGen && movementGen->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        // Depending on whether the target is a player or a creature, cast the movement generator to the appropriate type and return the chase target
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            return static_cast<ChaseMovementGenerator<Player> const*>(movementGen)->GetTarget();
        }
        else
        {
            return static_cast<ChaseMovementGenerator<Creature> const*>(movementGen)->GetTarget();
        }
    }

    // Return nullptr if there is no chase target
    return nullptr;
}
