/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PERFORMANCEMONITOR_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_PERFORMANCEMONITOR_H

#include "Common.h"  // Include common definitions and utilities

#include <chrono>  // Include for time measurement
#include <ctime>  // Include for C time library
#include <map>  // Include for map container
#include <mutex>  // Include for mutex
#include <vector>  // Include for vector container

// Define a type alias for a vector of strings, representing a stack of performance metrics
typedef std::vector<std::string> PerformanceStack;

// Structure to store performance data
struct PerformanceData
{
    uint32 minTime;  // Minimum recorded time for an operation
    uint32 maxTime;  // Maximum recorded time for an operation
    uint32 totalTime;  // Total accumulated time for an operation
    uint32 count;  // Count of how many times the operation was recorded
    std::mutex lock;  // Mutex to protect access to this data
};

// Enumeration for different performance metrics
enum PerformanceMetric
{
    PERF_MON_TRIGGER,  // Metric for trigger performance
    PERF_MON_VALUE,  // Metric for value performance
    PERF_MON_ACTION,  // Metric for action performance
    PERF_MON_RNDBOT,  // Metric for random bot performance
    PERF_MON_TOTAL  // Total performance metrics count
};

// Class to represent a performance monitoring operation
class PerformanceMonitorOperation
{
    public:
        // Constructor to initialize with performance data, name, and stack
        PerformanceMonitorOperation(PerformanceData* data, std::string const name, PerformanceStack* stack);
        // Function to finish the operation and record the elapsed time
        void finish();

    private:
        PerformanceData* data;  // Pointer to the performance data
        std::string const name;  // Name of the operation
        PerformanceStack* stack;  // Stack of performance metrics
        std::chrono::milliseconds started;  // Start time of the operation
};

// Class to monitor performance of various operations
class PerformanceMonitor
{
    public:
        // Constructor
        PerformanceMonitor() { };
        // Virtual destructor
        virtual ~PerformanceMonitor() { };
        // Static function to get the singleton instance of the monitor
        static PerformanceMonitor* instance()
        {
            static PerformanceMonitor instance;  // Static instance of the monitor
            return &instance;
        }

	public:
        // Function to start monitoring a performance metric
        PerformanceMonitorOperation* start(PerformanceMetric metric, std::string const name, PerformanceStack* stack = nullptr);
        // Function to print performance statistics
        void PrintStats(bool perTick = false, bool fullStack = false);
        // Function to reset the performance data
        void Reset();

	private:
        // Map to store performance data for each metric and operation name
        std::map<PerformanceMetric, std::map<std::string, PerformanceData*> > data;
        // Mutex to protect access to the performance data map
        std::mutex lock;
};

// Macro to get the singleton instance of the performance monitor
#define sPerformanceMonitor PerformanceMonitor::instance()

#endif  // End of header guard
