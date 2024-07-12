/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PerformanceMonitor.h"  // Include the header for PerformanceMonitor class
#include "Playerbots.h"  // Include the header for Playerbots

// Function to start monitoring a performance metric
PerformanceMonitorOperation* PerformanceMonitor::start(PerformanceMetric metric, std::string const name, PerformanceStack* stack)
{
    // Check if performance monitoring is enabled in the configuration
    if (!sPlayerbotAIConfig->perfMonEnabled)
        return nullptr;  // Return null if performance monitoring is disabled

    std::string stackName = name;  // Initialize stackName with the provided name

    if (stack)  // If a stack is provided
    {
        if (!stack->empty())  // If the stack is not empty
        {
            std::ostringstream out;
            out << stackName << " [";  // Start constructing the stack name string

            // Iterate over the stack in reverse order and append each element to the string
            for (std::vector<std::string>::reverse_iterator i = stack->rbegin(); i != stack->rend(); ++i)
                out << *i << (std::next(i) == stack->rend() ? "" : "|");

            out << "]";  // Close the stack name string

            stackName = out.str().c_str();  // Convert to string and assign to stackName
        }

        stack->push_back(name);  // Push the current name onto the stack
    }

    std::lock_guard<std::mutex> guard(lock);  // Lock the mutex for thread safety
    PerformanceData* pd = data[metric][stackName];  // Retrieve performance data for the metric and stackName
    if (!pd)  // If no data exists, create a new PerformanceData object
    {
        pd = new PerformanceData();
        pd->minTime = 0;
        pd->maxTime = 0;
        pd->totalTime = 0;
        pd->count = 0;
        data[metric][stackName] = pd;  // Store the new PerformanceData object
    }

    // Return a new PerformanceMonitorOperation object initialized with the PerformanceData, name, and stack
    return new PerformanceMonitorOperation(pd, name, stack);
}

// Function to print performance statistics
void PerformanceMonitor::PrintStats(bool perTick, bool fullStack)
{
    if (data.empty())  // If no performance data exists, return
        return;

    uint32 total = 0;  // Initialize total time variable

    if (!perTick)  // If printing total statistics
    {
        // Calculate total time for specific metrics
        for (auto& map : data[PERF_MON_TOTAL])
            if (map.first.find("PlayerbotAI::UpdateAIInternal") != std::string::npos)
                total += map.second->totalTime;

        // Log header for total bot statistics
        LOG_INFO("playerbots", "--------------------------------------[TOTAL BOT]------------------------------------------------------");
        LOG_INFO("playerbots", "percentage     time  |    min ..    max (      avg  of      count) - type      : name");

        // Iterate over performance metrics
        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin(); i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;  // Retrieve performance data map for the current metric

            std::string key;  // Initialize key for metric type
            switch (i->first)
            {
                case PERF_MON_TRIGGER:
                    key = "T";
                    break;
                case PERF_MON_VALUE:
                    key = "V";
                    break;
                case PERF_MON_ACTION:
                    key = "A";
                    break;
                case PERF_MON_RNDBOT:
                    key = "RndBot";
                    break;
                case PERF_MON_TOTAL:
                    key = "Total";
                    break;
                default:
                    key = "?";
                    break;
            }

            std::vector<std::string> names;  // Initialize vector to hold names of performance data entries

            // Add names to the vector, skipping irrelevant entries for "Total" key
            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                if (key == "Total" && j->first.find("PlayerbotAI::UpdateAIInternal") == std::string::npos)
                    continue;

                names.push_back(j->first);
            }

            // Sort names by total time in ascending order
            std::sort(names.begin(), names.end(), [pdMap](std::string const i, std::string const j)
            {
                return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime;
            });

            // Iterate over sorted names and log performance statistics
            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                float perc = (float)pd->totalTime / (float)total * 100.0f;  // Calculate percentage of total time
                float secs = (float)pd->totalTime / 1000.0f;  // Convert total time to seconds
                float avg = (float)pd->totalTime / (float)pd->count;  // Calculate average time per operation
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";  // Truncate name if full stack not required

                // Log the performance data if average time or max time exceed thresholds
                if (avg >= 0.5f || pd->maxTime > 10)
                {
                    LOG_INFO("playerbots", "{:7.3f}% {:10.3f}s | {:6d} .. {:6d} ({:10.3f} of {:10d}) - {:6}    : {}"
                        , perc
                        , secs
                        , pd->minTime
                        , pd->maxTime
                        , avg
                        , pd->count
                        , key.c_str()
                        , disName.c_str());
                }
            }
            LOG_INFO("playerbots", " ");  // Log a blank line for separation
        }
    }
    else  // If printing per tick statistics
    {
        // Calculate total time and count for per tick statistics
        float totalCount = data[PERF_MON_TOTAL]["RandomPlayerbotMgr::FullTick"]->count;
        total = data[PERF_MON_TOTAL]["RandomPlayerbotMgr::FullTick"]->totalTime;

        // Log header for per tick statistics
        LOG_INFO("playerbots", "---------------------------------------[PER TICK]------------------------------------------------------");
        LOG_INFO("playerbots", "percentage     time  |    min ..    max (      avg  of      count) - type      : name");

        // Iterate over performance metrics
        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin(); i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;  // Retrieve performance data map for the current metric

            std::string key;  // Initialize key for metric type
            switch (i->first)
            {
                case PERF_MON_TRIGGER:
                    key = "T";
                    break;
                case PERF_MON_VALUE:
                    key = "V";
                    break;
                case PERF_MON_ACTION:
                    key = "A";
                    break;
                case PERF_MON_RNDBOT:
                    key = "RndBot";
                    break;
                case PERF_MON_TOTAL:
                    key = "Total";
                    break;
                default:
                    key = "?";
            }

            std::vector<std::string> names;  // Initialize vector to hold names of performance data entries

            // Add names to the vector
            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                names.push_back(j->first);
            }

            // Sort names by total time in ascending order
            std::sort(names.begin(), names.end(), [pdMap](std::string const i, std::string const j)
            {
                return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime;
            });

            // Iterate over sorted names and log performance statistics
            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                float perc = (float)pd->totalTime / (float)total * 100.0f;  // Calculate percentage of total time
                uint32 secs = pd->totalTime / totalCount;  // Calculate total time per tick
                float avg = (float)pd->totalTime / (float)pd->count;  // Calculate average time per operation
                float amount = (float)pd->count / (float)totalCount;  // Calculate count ratio per tick
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";  // Truncate name if full stack not required

                // Log the performance data if average time or max time exceed thresholds
                if (avg >= 0.5f || pd->maxTime > 10)
                {
                    LOG_INFO("playerbots", "{:7.3f}% {:9d}ms | {:6d} .. {:6d} ({:10.3f} of {:10.3f}) - {:6}    : {}"
                        , perc
                        , secs
                        , pd->minTime
                        , pd->maxTime
                        , avg
                        , amount
                        , key.c_str()
                        , disName.c_str());
                }
            }
            LOG_INFO("playerbots", " ");  // Log a blank line for separation
        }
    }
}

// Function to reset all performance data
void PerformanceMonitor::Reset()
{
    // Iterate over all performance metrics
    for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*> >::iterator i = data.begin(); i != data.end(); ++i)
    {
        std::map<std::string, PerformanceData*> pdMap = i->second;  // Retrieve performance data map for the current metric
        for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
        {
            PerformanceData* pd = j->second;  // Retrieve performance data entry
            std::lock_guard<std::mutex> guard(pd->lock);  // Lock the mutex for thread safety
            pd->minTime = 0;  // Reset minimum time
            pd->maxTime = 0;  // Reset maximum time
            pd->totalTime = 0;  // Reset total time
            pd->count = 0;  // Reset count
        }
    }
}

// Constructor for PerformanceMonitorOperation
PerformanceMonitorOperation::PerformanceMonitorOperation(PerformanceData* data, std::string const name, PerformanceStack* stack)
    : data(data), name(name), stack(stack)
{
    // Record the start time in milliseconds since epoch
    started = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
}

// Function to finish monitoring and record the elapsed time
void PerformanceMonitorOperation::finish()
{
    // Record the finish time in milliseconds since epoch
    std::chrono::milliseconds finished = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
    uint32 elapsed = (finished - started).count();  // Calculate the elapsed time

    std::lock_guard<std::mutex> guard(data->lock);  // Lock the mutex for thread safety
    if (elapsed > 0)  // If the elapsed time is greater than 0
    {
        if (!data->minTime || data->minTime > elapsed)
            data->minTime = elapsed;  // Update minimum time if necessary

        if (!data->maxTime || data->maxTime < elapsed)
            data->maxTime = elapsed;  // Update maximum time if necessary

        data->totalTime += elapsed;  // Add the elapsed time to the total time
    }

    ++data->count;  // Increment the count

    if (stack)  // If a stack is provided
    {
        // Remove the current name from the stack
        stack->erase(std::remove(stack->begin(), stack->end(), name), stack->end());
    }

    delete this;  // Delete the current operation object
}
