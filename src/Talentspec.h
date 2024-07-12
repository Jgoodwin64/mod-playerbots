/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TALENTSPEC_H
#define _PLAYERBOT_TALENTSPEC_H

#include "Action.h"

struct TalentEntry; // Forward declaration of TalentEntry structure
struct TalentTabEntry; // Forward declaration of TalentTabEntry structure

// Constants for sorting and calculating talent changes
#define SORT_BY_DEFAULT 0
#define SORT_BY_POINTS_TREE 1
#define ABSOLUTE_DIST 0
#define SUBSTRACT_OLD_NEW 1
#define SUBSTRACT_NEW_OLD -1
#define ADDED_POINTS 2
#define REMOVED_POINTS -2

// unused currently
// Class representing a talent specification for a player
class TalentSpec
{
    public:
        // Structure to represent an entry in the talent list
        struct TalentListEntry
        {
            uint32 entry; // Entry ID of the talent
            uint32 rank; // Current rank of the talent
            uint32 maxRank; // Maximum rank of the talent
            TalentEntry const* talentInfo; // Pointer to talent information
            TalentTabEntry const* talentTabInfo; // Pointer to talent tab information
            uint32 tabPage() const; // Function to get the tab page of the talent
        };

        TalentSpec() { }; // Default constructor
        virtual ~TalentSpec() { } // Destructor
        TalentSpec(uint32 classMask); // Constructor with class mask
        TalentSpec(TalentSpec* base, std::string const link); // Constructor with base talent spec and link
        TalentSpec(Player* bot); // Constructor with player pointer
        TalentSpec(Player* bot, std::string const link); // Constructor with player pointer and link

        uint32 points = 0; // Total points in the talent spec
        std::vector<TalentListEntry> talents; // Vector of talents in the spec

        bool CheckTalentLink(std::string const link, std::ostringstream* out); // Function to check the validity of a talent link
        virtual bool CheckTalents(uint32 maxPoints, std::ostringstream* out); // Function to check talents against max points
        void CropTalents(uint32 level); // Function to crop talents based on player level
        void ShiftTalents(TalentSpec* oldTalents, uint32 level); // Function to shift talents from old spec to new spec
        void ApplyTalents(Player* bot, std::ostringstream* out); // Function to apply talents to a player

        uint32 GetTalentPoints(std::vector<TalentListEntry>& talents, int32 tabpage = -1); // Function to get talent points for a specific tab page
        uint32 GetTalentPoints(int32 tabpage = -1); // Function to get total talent points
        bool isEarlierVersionOf(TalentSpec& newSpec); // Function to check if the current spec is an earlier version of another spec

        std::string const GetTalentLink(); // Function to get the talent link
        uint32 highestTree(); // Function to get the highest talent tree
        std::string const FormatSpec(Player* bot); // Function to format the talent spec for a player

    protected:
        uint32 LeveltoPoints(uint32 level) const; // Function to convert level to talent points
        uint32 PointstoLevel(uint32 points) const; // Function to convert talent points to level
        void GetTalents(uint32 classMask); // Function to get talents for a class
        void SortTalents(std::vector<TalentListEntry>& talents, uint32 sortBy); // Function to sort talents by a specified method
        void SortTalents(uint32 sortBy); // Function to sort talents by a specified method

        void ReadTalents(Player* bot); // Function to read talents from a player
        void ReadTalents(std::string const link); // Function to read talents from a link

        std::vector<TalentListEntry> GetTalentTree(uint32 tabpage); // Function to get the talent tree for a specific tab page
        std::vector<TalentListEntry> SubTalentList(std::vector<TalentListEntry>& oldList, std::vector<TalentListEntry>& newList, uint32 reverse); // Function to get the difference between two talent lists
};

// Class representing a talent path
class TalentPath
{
    public:
        TalentPath(uint32 pathId, std::string const pathName, uint32 pathProbability)
        {
            id = pathId;
            name = pathName;
            probability = pathProbability;
        };

        uint32 id = 0; // ID of the talent path
        std::string name = ""; // Name of the talent path
        uint32 probability = 100; // Probability of selecting this path
        std::vector<TalentSpec> talentSpec; // Vector of talent specs for this path
};

// Class representing the specifications for a class
class ClassSpecs
{
    public:
        ClassSpecs() { }; // Default constructor
        ClassSpecs(uint32 specsClassMask)
        {
            classMask = specsClassMask;
            baseSpec = TalentSpec(specsClassMask);
        }

        uint32 classMask = 0; // Class mask for the specifications
        TalentSpec baseSpec; // Base talent spec for the class

        std::vector<TalentPath> talentPath; // Vector of talent paths for the class
};

#endif
