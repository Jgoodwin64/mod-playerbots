/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VALUE_H
#define _PLAYERBOT_VALUE_H

#include "AiObject.h"
#include "ObjectGuid.h"
#include "PerformanceMonitor.h"
#include "Timer.h"
#include "Unit.h"

#include <time.h>

class PlayerbotAI;
class Unit;

// Class definition for FleeInfo
class FleeInfo
{
    public:
        Position fromPos; // Position from which the bot is fleeing
        float radius; // Radius of the flee circle
        float angle; // Angle of the flee direction
        uint32 timestamp; // Timestamp of when the flee started
        int GetAngleRangeIndex() { return (angle + 2 * M_PI) / (M_PI / 2); } // [0, 7) // Get the angle range index
};

struct CreatureData;

// Class definition for UntypedValue, inheriting from AiNamedObject
class UntypedValue : public AiNamedObject
{
    public:
        // Constructor for UntypedValue
        UntypedValue(PlayerbotAI* botAI, std::string const name) : AiNamedObject(botAI, name) { }
        // Virtual destructor for UntypedValue
        virtual ~UntypedValue() { }
        // Virtual method to update the value
        virtual void Update() { }
        // Virtual method to reset the value
        virtual void Reset() { }
        // Virtual method to format the value as a string
        virtual std::string const Format() { return "?"; }
        // Virtual method to save the value as a string
        virtual std::string const Save() { return "?"; }
        // Virtual method to load the value from a string
        virtual bool Load([[maybe_unused]] std::string const value) { return false; }
};

// Template class definition for Value
template<class T>
class Value
{
    public:
        // Virtual destructor for Value
        virtual ~Value() { }
        // Pure virtual method to get the value
        virtual T Get() = 0;
        // Pure virtual method to get the value lazily
        virtual T LazyGet() = 0;
        // Pure virtual method to get a reference to the value
        virtual T& RefGet() = 0;
        // Virtual method to reset the value
        virtual void Reset() { }
        // Pure virtual method to set the value
        virtual void Set(T value) = 0;
        // Conversion operator to get the value
        operator T() { return Get(); }
};

// Template class definition for CalculatedValue, inheriting from UntypedValue and Value
template<class T>
class CalculatedValue : public UntypedValue, public Value<T>
{
    public:
        // Constructor for CalculatedValue
        CalculatedValue(PlayerbotAI* botAI, std::string const name = "value", uint32 checkInterval = 1) : UntypedValue(botAI, name),
            checkInterval(checkInterval == 1 ? 1 : (checkInterval < 100 ? checkInterval * 1000 : checkInterval)) /*turn s -> ms?*/, lastCheckTime(0) { } 

        // Virtual destructor for CalculatedValue
        virtual ~CalculatedValue() { }

        // Method to get the value
        T Get() override
        {
            if (checkInterval < 2) {
                // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
                value = Calculate();
                // if (pmo)
                //     pmo->finish();
            } else {
                time_t now = getMSTime();
                if (!lastCheckTime || now - lastCheckTime >= checkInterval)
                {
                    lastCheckTime = now;
                    // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
                    value = Calculate();
                    // if (pmo)
                    //     pmo->finish();
                }
            }
            return value;
        }

        // Method to get the value lazily
        T LazyGet() override
        {
            if (!lastCheckTime)
                return Get();

            return value;
        }

        // Method to get a reference to the value
        T& RefGet() override
        {
            if (checkInterval < 2) {
                // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
                value = Calculate();
                // if (pmo)
                //     pmo->finish();
            } else {
                time_t now = getMSTime();
                if (!lastCheckTime || now - lastCheckTime >= checkInterval)
                {
                    lastCheckTime = now;
                    // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
                    value = Calculate();
                    // if (pmo)
                    //     pmo->finish();
                }
            }
            return value;
        }

        // Method to set the value
        void Set(T val) override { value = val; }
        // Method to update the value
        void Update() override {}
        // Method to reset the value
        void Reset() override { lastCheckTime = 0; }

    protected:
        // Pure virtual method to calculate the value
        virtual T Calculate() = 0;

        uint32 checkInterval; // Interval between checks
        uint32 lastCheckTime; // Time of the last check
        T value; // Cached value
};

// Template class definition for SingleCalculatedValue, inheriting from CalculatedValue
template <class T>
class SingleCalculatedValue : public CalculatedValue<T>
{
    public:
        // Constructor for SingleCalculatedValue
        SingleCalculatedValue(PlayerbotAI* botAI, std::string const name = "value") : CalculatedValue<T>(botAI, name)
        {
            this->Reset();
        }

        // Method to get the value
        T Get() override
        {
            time_t now = time(0);
            if (!this->lastCheckTime)
            {
                this->lastCheckTime = now;

                PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
                this->value = this->Calculate();
                if (pmo)
                    pmo->finish();
            }

            return this->value;
        }
};

// Template class definition for MemoryCalculatedValue, inheriting from CalculatedValue
template<class T>
class MemoryCalculatedValue : public CalculatedValue<T>
{
    public:
        // Constructor for MemoryCalculatedValue
        MemoryCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1) : CalculatedValue<T>(botAI, name, checkInterval)
        {
            lastChangeTime = time(0);
        }

        // Pure virtual method to check if the value is equal to the last value
        virtual bool EqualToLast(T value) = 0;

        // Method to check if the value change can be checked
        virtual bool CanCheckChange()
        {
            return time(0) - lastChangeTime < minChangeInterval || EqualToLast(this->value);
        }

        // Method to update the value change
        virtual bool UpdateChange()
        {
            if (CanCheckChange())
                return false;

            lastChangeTime = time(0);
            lastValue = this->value;
            return true;
        }

        // Method to set the value
        void Set([[maybe_unused]] T value) override
        {
            CalculatedValue<T>::Set(this->value);
            UpdateChange();
        }

        // Method to get the value
        T Get() override
        {
            this->value = CalculatedValue<T>::Get();
            UpdateChange();
            return this->value;
        }

        // Method to get the value lazily
        T LazyGet() override
        {
            return this->value;
        }

        // Method to get the last change time
        time_t LastChangeOn()
        {
            Get();
            UpdateChange();
            return lastChangeTime;
        }

        // Method to get the delay since the last change
        uint32 LastChangeDelay() { return time(0) - LastChangeOn(); }

        // Method to reset the value
        void Reset() override
        {
            CalculatedValue<T>::Reset();
            lastChangeTime = time(0);
        }

    protected:
        T lastValue; // Last value
        uint32 minChangeInterval = 0; // Minimum interval between changes
        time_t lastChangeTime; // Time of the last change
};

// Template class definition for LogCalculatedValue, inheriting from MemoryCalculatedValue
template<class T>
class LogCalculatedValue : public MemoryCalculatedValue<T>
{
    public:
        // Constructor for LogCalculatedValue
        LogCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1) : MemoryCalculatedValue<T>(botAI, name, checkInterval) { }

        // Method to update the value change
        bool UpdateChange() override
        {
            if (MemoryCalculatedValue<T>::UpdateChange())
                return false;

            valueLog.push_back(std::make_pair(this->value, time(0)));

            if (valueLog.size() > logLength)
                valueLog.pop_front();
            return true;
        }

        // Method to get the value log
        std::list<std::pair<T, time_t>> ValueLog() { return valueLog; }

        // Method to reset the value
        void Reset() override
        {
            MemoryCalculatedValue<T>::Reset();
            valueLog.clear();
        }

    protected:
        std::list<std::pair<T, time_t>> valueLog; // Log of values
        uint8 logLength = 10; // Length of the log
};

// Class definition for Uint8CalculatedValue, inheriting from CalculatedValue<uint8>
class Uint8CalculatedValue : public CalculatedValue<uint8>
{
    public:
        // Constructor for Uint8CalculatedValue
        Uint8CalculatedValue(PlayerbotAI* botAI, std::string const name = "value", uint32 checkInterval = 1) :
            CalculatedValue<uint8>(botAI, name, checkInterval) { }

        // Method to format the value as a string
        std::string const Format() override;
};

// Class definition for Uint32CalculatedValue, inheriting from CalculatedValue<uint32>
class Uint32CalculatedValue : public CalculatedValue<uint32>
{
    public:
        // Constructor for Uint32CalculatedValue
        Uint32CalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int checkInterval = 1) :
            CalculatedValue<uint32>(botAI, name, checkInterval) { }

        // Method to format the value as a string
        std::string const Format() override;
};

// Class definition for FloatCalculatedValue, inheriting from CalculatedValue<float>
class FloatCalculatedValue : public CalculatedValue<float>
{
    public:
        // Constructor for FloatCalculatedValue
        FloatCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int checkInterval = 1) :
            CalculatedValue<float>(botAI, name, checkInterval) { }

        // Method to format the value as a string
        std::string const Format() override;
};

// Class definition for BoolCalculatedValue, inheriting from CalculatedValue<bool>
class BoolCalculatedValue : public CalculatedValue<bool>
{
    public:
        // Constructor for BoolCalculatedValue
        BoolCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int checkInterval = 1) :
            CalculatedValue<bool>(botAI, name, checkInterval) { }

        // Method to format the value as a string
        std::string const Format() override
        {
            return Calculate() ? "true" : "false";
        }
};

// Class definition for UnitCalculatedValue, inheriting from CalculatedValue<Unit*>
class UnitCalculatedValue : public CalculatedValue<Unit*>
{
    public:
        // Constructor for UnitCalculatedValue
        UnitCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

        // Method to format the value as a string
        std::string const Format() override;
        // Method to get the value
        Unit* Get() override;
};

// Class definition for CDPairCalculatedValue, inheriting from CalculatedValue<CreatureData const*>
class CDPairCalculatedValue : public CalculatedValue<CreatureData const*>
{
    public:
        // Constructor for CDPairCalculatedValue
        CDPairCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

        // Method to format the value as a string
        std::string const Format() override;
};

// Class definition for CDPairListCalculatedValue, inheriting from CalculatedValue<std::vector<CreatureData const*>>
class CDPairListCalculatedValue : public CalculatedValue<std::vector<CreatureData const*>>
{
    public:
        // Constructor for CDPairListCalculatedValue
        CDPairListCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

        // Method to format the value as a string
        std::string const Format() override;
};

// Class definition for ObjectGuidCalculatedValue, inheriting from CalculatedValue<ObjectGuid>
class ObjectGuidCalculatedValue : public CalculatedValue<ObjectGuid>
{
    public:
        // Constructor for ObjectGuidCalculatedValue
        ObjectGuidCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

        // Method to format the value as a string
        std::string const Format() override;
};

// Class definition for ObjectGuidListCalculatedValue, inheriting from CalculatedValue<GuidVector>
class ObjectGuidListCalculatedValue : public CalculatedValue<GuidVector>
{
    public:
        // Constructor for ObjectGuidListCalculatedValue
        ObjectGuidListCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

        // Method to format the value as a string
        std::string const Format() override;
};

// Template class definition for ManualSetValue, inheriting from UntypedValue and Value
template<class T>
class ManualSetValue : public UntypedValue, public Value<T>
{
    public:
        // Constructor for ManualSetValue
        ManualSetValue(PlayerbotAI* botAI, T defaultValue, std::string const name = "value") :
            UntypedValue(botAI, name), value(defaultValue), defaultValue(defaultValue) { }

        // Virtual destructor for ManualSetValue
        virtual ~ManualSetValue() { }

        // Method to get the value
        T Get() override { return value; }
        // Method to get the value lazily
        T LazyGet() override { return value; }
        // Method to get a reference to the value
        T& RefGet() override { return value; }
        // Method to set the value
        void Set(T val) override { value = val; }
        // Method to update the value
        void Update() override {}
        // Method to reset the value
        void Reset() override
        {
            value = defaultValue;
        }

    protected:
        T value; // Current value
        T defaultValue; // Default value
};

// Class definition for UnitManualSetValue, inheriting from ManualSetValue<Unit*>
class UnitManualSetValue : public ManualSetValue<Unit*>
{
    public:
        // Constructor for UnitManualSetValue
        UnitManualSetValue(PlayerbotAI* botAI, Unit* defaultValue, std::string const name = "value") :
            ManualSetValue<Unit*>(botAI, defaultValue, name) { }

        // Method to format the value as a string
        std::string const Format() override;
        // Method to get the value
        Unit* Get() override;
};

// Class definition for DisperseDistanceValue, inheriting from ManualSetValue<float>
class DisperseDistanceValue : public ManualSetValue<float>
{
    public:
        // Constructor for DisperseDistanceValue
        DisperseDistanceValue(PlayerbotAI* botAI, float defaultValue = -1.0f, std::string const name = "disperse distance") :
            ManualSetValue<float>(botAI, defaultValue, name) { }
};

// Class definition for LastFleeAngleValue, inheriting from ManualSetValue<float>
class LastFleeAngleValue : public ManualSetValue<float>
{
    public:
        // Constructor for LastFleeAngleValue
        LastFleeAngleValue(PlayerbotAI* botAI, float defaultValue = 0.0f, std::string const name = "last flee angle") :
            ManualSetValue<float>(botAI, defaultValue, name) { }
};

// Class definition for LastFleeTimestampValue, inheriting from ManualSetValue<uint32>
class LastFleeTimestampValue : public ManualSetValue<uint32>
{
    public:
        // Constructor for LastFleeTimestampValue
        LastFleeTimestampValue(PlayerbotAI* botAI, uint32 defaultValue = 0, std::string const name = "last flee timestamp") :
            ManualSetValue<uint32>(botAI, defaultValue, name) { }
};

// Class definition for RecentlyFleeInfo, inheriting from ManualSetValue<std::list<FleeInfo>>
class RecentlyFleeInfo : public ManualSetValue<std::list<FleeInfo>>
{
    public:
        // Constructor for RecentlyFleeInfo
        RecentlyFleeInfo(PlayerbotAI* botAI, std::list<FleeInfo> defaultValue = {}, std::string const name = "recently flee info") :
            ManualSetValue<std::list<FleeInfo>>(botAI, defaultValue, name) { }
};

#endif
