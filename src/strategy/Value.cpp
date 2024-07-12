/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"
#include "PerformanceMonitor.h"
#include "Playerbots.h"
#include "Timer.h"

// Constructor for UnitCalculatedValue class
UnitCalculatedValue::UnitCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval) : CalculatedValue<Unit*>(botAI, name, checkInterval)
{
}

// Method to format the calculated unit value as a string
std::string const UnitCalculatedValue::Format()
{
    // Calculate the unit value
    Unit* unit = Calculate();
    // Return the unit name or "<none>" if null
    return unit ? unit->GetName() : "<none>";
}

// Method to format the manually set unit value as a string
std::string const UnitManualSetValue::Format()
{
    // Get the unit value
    Unit* unit = Get();
    // Return the unit name or "<none>" if null
    return unit ? unit->GetName() : "<none>";
}

// Method to format the calculated uint8 value as a string
std::string const Uint8CalculatedValue::Format()
{
    // Create an output string stream
    std::ostringstream out;
    // Calculate the uint8 value and write to the stream
    out << Calculate();
    // Return the formatted string
    return out.str();
}

// Method to format the calculated uint32 value as a string
std::string const Uint32CalculatedValue::Format()
{
    // Create an output string stream
    std::ostringstream out;
    // Calculate the uint32 value and write to the stream
    out << Calculate();
    // Return the formatted string
    return out.str();
}

// Method to format the calculated float value as a string
std::string const FloatCalculatedValue::Format()
{
    // Create an output string stream
    std::ostringstream out;
    // Calculate the float value and write to the stream
    out << Calculate();
    // Return the formatted string
    return out.str();
}

// Constructor for CDPairCalculatedValue class
CDPairCalculatedValue::CDPairCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    CalculatedValue<CreatureData const*>(botAI, name, checkInterval)
{
    // lastCheckTime = getMSTime() - checkInterval / 2;
}

// Method to format the calculated CDPair value as a string
std::string const CDPairCalculatedValue::Format()
{
    // Calculate the CDPair value
    CreatureData const* creatureData = Calculate();
    if (creatureData)
    {
        // Get the creature template for the calculated value
        CreatureTemplate const* bmTemplate = sObjectMgr->GetCreatureTemplate(creatureData->id1);
        // Return the creature template name or "<none>" if null
        return bmTemplate ? bmTemplate->Name : "<none>";
    }

    // Return "<none>" if no creature data is found
    return "<none>";
}

// Constructor for CDPairListCalculatedValue class
CDPairListCalculatedValue::CDPairListCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    CalculatedValue<std::vector<CreatureData const*>>(botAI, name, checkInterval)
{
    // lastCheckTime = time(nullptr) - checkInterval / 2;
}

// Method to format the calculated CDPairList value as a string
std::string const CDPairListCalculatedValue::Format()
{
    // Create an output string stream
    std::ostringstream out;
    out << "{";
    // Calculate the CDPairList value
    std::vector<CreatureData const*> cdPairs = Calculate();
    for (CreatureData const* cdPair : cdPairs)
    {
        // Write each pair's id1 to the stream
        out << cdPair->id1 << ",";
    }

    out << "}";
    // Return the formatted string
    return out.str();
}

// Constructor for ObjectGuidCalculatedValue class
ObjectGuidCalculatedValue::ObjectGuidCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    CalculatedValue<ObjectGuid>(botAI, name, checkInterval)
{
    // lastCheckTime = time(nullptr) - checkInterval / 2;
}

// Method to format the calculated ObjectGuid value as a string
std::string const ObjectGuidCalculatedValue::Format()
{
    // Calculate the ObjectGuid value
    ObjectGuid guid = Calculate();
    // Return the raw value as a string or "<none>" if null
    return guid ? std::to_string(guid.GetRawValue()) : "<none>";
}

// Constructor for ObjectGuidListCalculatedValue class
ObjectGuidListCalculatedValue::ObjectGuidListCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    CalculatedValue<GuidVector>(botAI, name, checkInterval)
{
}

// Method to format the calculated ObjectGuidList value as a string
std::string const ObjectGuidListCalculatedValue::Format()
{
    // Create an output string stream
    std::ostringstream out;
    out << "{";

    // Calculate the ObjectGuidList value
    GuidVector guids = Calculate();
    for (GuidVector::iterator i = guids.begin(); i != guids.end(); ++i)
    {
        // Write each guid's raw value to the stream
        ObjectGuid guid = *i;
        out << guid.GetRawValue() << ",";
    }
    out << "}";

    // Return the formatted string
    return out.str();
}

// Method to get the calculated unit value
Unit* UnitCalculatedValue::Get()
{
    if (checkInterval < 2) {
        // Start performance monitoring operation
        PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
        // Calculate the value
        value = Calculate();
        if (pmo)
            pmo->finish(); // Finish performance monitoring
    } else {
        // Get the current time
        time_t now = getMSTime();
        if (!lastCheckTime || now - lastCheckTime >= checkInterval)
        {
            lastCheckTime = now;
            // Start performance monitoring operation
            PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
            // Calculate the value
            value = Calculate();
            if (pmo)
                pmo->finish(); // Finish performance monitoring
        }
    }
    // Prevent crashing by checking if value is in world
    if (value && value->IsInWorld())
        return value;
    return nullptr;
}

// Method to get the manually set unit value
Unit* UnitManualSetValue::Get()
{
    // Prevent crashing by checking if value is in world
    if (value && value->IsInWorld())
        return value;
    return nullptr;
}
