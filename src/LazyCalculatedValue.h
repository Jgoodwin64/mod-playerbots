/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LAZYCALCULATEDVALUE_H  // Header guard to prevent multiple inclusions of this header file
#define _PLAYERBOT_LAZYCALCULATEDVALUE_H

// Template class for lazy evaluation and caching of a value
template <class TValue, class TOwner>
class LazyCalculatedValue
{
    public:
        // Define a type alias for a member function pointer to a method of TOwner that returns TValue
        typedef TValue (TOwner::*Calculator)();

    public:
        // Constructor that takes an owner object and a calculator function pointer
        LazyCalculatedValue(TOwner* owner, Calculator calculator) : calculator(calculator), owner(owner)
        {
            Reset();  // Initialize the object by resetting the calculated flag
        }

    public:
        // Method to get the value, calculates it if not already done
        TValue GetValue()
        {
            if (!calculated)  // Check if the value has already been calculated
            {
                value = (owner->*calculator)();  // Call the calculator method on the owner object
                calculated = true;  // Mark as calculated
            }

            return value;  // Return the cached value
        }

        // Method to reset the calculated flag, forcing recalculation on next GetValue call
        void Reset()
        {
            calculated = false;  // Reset the calculated flag
        }

    protected:
        Calculator calculator;  // Pointer to the calculator method
        TOwner* owner;  // Pointer to the owner object
        bool calculated;  // Flag indicating whether the value has been calculated
        TValue value;  // Cached value
};

#endif  // End of header guard
