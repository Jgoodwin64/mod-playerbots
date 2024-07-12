/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Helpers.h"  // Include the Helpers header file

// Function to perform a case-insensitive search of `needle` in `haystack`
char* strstri(char const* haystack, char const* needle)
{
    // If needle is an empty string, return the haystack as the match
    if (!*needle)
    {
        return (char*)haystack;
    }

    // Loop through each character in the haystack
    for (; *haystack; ++haystack)
    {
        // If the current character in haystack matches the first character in needle (case-insensitive)
        if (tolower(*haystack) == tolower(*needle))
        {
            // Pointers to traverse the remaining characters in haystack and needle
            char const* h = haystack, * n = needle;

            // Continue comparing the subsequent characters in both strings
            for (; *h && *n; ++h, ++n)
            {
                // If characters do not match (case-insensitive), break the inner loop
                if (tolower(*h) != tolower(*n))
                {
                    break;
                }
            }

            // If the end of the needle is reached, a match is found, return the start of the match in haystack
            if (!*n)
            {
                return (char*)haystack;
            }
        }
    }

    // No match found, return null pointer
    return 0;
}

// Function to remove leading whitespace from a string
std::string& ltrim(std::string& s)
{
    // Use erase-remove idiom with a lambda to check for whitespace
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); }));
    return s;
}

// Function to remove trailing whitespace from a string
std::string& rtrim(std::string& s)
{
    // Use erase-remove idiom with a lambda to check for whitespace, applied in reverse
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(), s.end());
    return s;
}

// Function to remove both leading and trailing whitespace from a string
std::string& trim(std::string& s)
{
    // Apply both left trim and right trim
    return ltrim(rtrim(s));
}
