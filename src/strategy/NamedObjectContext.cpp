/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NamedObjectContext.h"
#include "Playerbots.h"

// Function to qualify an object with an integer qualifier
void Qualified::Qualify(int qual)
{
    // Convert the integer qualifier to a string
    std::ostringstream out;
    out << qual;
    qualifier = out.str();
}

// Function to combine multiple qualifiers into a single string
std::string const Qualified::MultiQualify(std::vector<std::string> qualifiers)
{
    // Create a string stream to concatenate the qualifiers
    std::ostringstream out;
    for (auto& qualifier : qualifiers)
        // Add each qualifier to the stream, separating them with a space if needed
        out << qualifier << (&qualifier != &qualifiers.back() ? " " : "");

    // Return the concatenated string
    return out.str();
}

// Function to split a single string of qualifiers into a vector of qualifiers
std::vector<std::string> Qualified::getMultiQualifiers(std::string const qualifier1)
{
    // Use a string stream to split the qualifiers by spaces
    std::istringstream iss(qualifier1);
    return { std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
}

// Function to get a specific qualifier from a string of qualifiers
int32 Qualified::getMultiQualifier(std::string const qualifier1, uint32 pos)
{
    // Convert the desired qualifier to an integer
    return std::stoi(getMultiQualifiers(qualifier1)[pos]);
}
