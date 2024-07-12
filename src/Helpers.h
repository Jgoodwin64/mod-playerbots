/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HELPERS_H  // Start of header guard to prevent multiple inclusions
#define _PLAYERBOT_HELPERS_H

#include "Common.h"  // Include common definitions and utilities

// Standard library includes
#include <map>       // Include the map container
#include <vector>    // Include the vector container
#include <functional>  // Include function objects
#include <cctype>    // Include character handling functions
#include <locale>    // Include locale for handling character classification
#include <sstream>   // Include string stream for string manipulation
#include <stdio.h>   // Include standard I/O functions
#include <string.h>  // Include string handling functions

// Function to split a string into a vector of strings using a delimiter
void split(std::vector<std::string>& dest, std::string const str, char const* delim)
{
    char* pTempStr = strdup(str.c_str());  // Duplicate the input string to avoid modifying the original
    char* pWord = strtok(pTempStr, delim);  // Tokenize the string using the delimiter

    while (pWord != nullptr)  // Loop through all tokens
    {
        dest.push_back(pWord);  // Add each token to the destination vector
        pWord = strtok(nullptr, delim);  // Get the next token
    }

    free(pTempStr);  // Free the duplicated string to avoid memory leak
}

// Overloaded function to split a string into a vector of strings using a single character delimiter
std::vector<std::string>& split(std::string const s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);  // Create a stringstream from the input string
    std::string item;  // Temporary string to hold each split item

    while (getline(ss, item, delim))  // Get each substring delimited by the character
    {
        elems.push_back(item);  // Add the substring to the vector
    }

    return elems;  // Return the vector containing the split elements
}

// Function to split a string into a vector of strings using a single character delimiter, returns a new vector
std::vector<std::string> split(std::string const s, char delim)
{
    std::vector<std::string> elems;  // Create a vector to hold the split elements
    return split(s, delim, elems);  // Call the overloaded split function and return the result
}

#endif  // End of header guard
