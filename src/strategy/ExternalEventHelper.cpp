/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ExternalEventHelper.h"
#include "Trigger.h"
#include "ChatHelper.h"
#include "Playerbots.h"

bool ExternalEventHelper::ParseChatCommand(std::string const command, Player* owner)
{
    if (HandleCommand(command, "", owner)) // Try to handle the command without parameters
        return true;

    size_t i = std::string::npos;
    while (true)
    {
        size_t found = command.rfind(" ", i); // Find the last space in the command
        if (found == std::string::npos || !found)
            break;

        std::string const name = command.substr(0, found); // Extract the command name
        std::string const param = command.substr(found + 1); // Extract the command parameter

        i = found - 1;

        if (HandleCommand(name, param, owner)) // Try to handle the command with extracted parameters
            return true;
    }

    if (!ChatHelper::parseable(command)) // Check if the command is parseable
        return false;

    HandleCommand("c", command, owner); // Handle command as chat command
    HandleCommand("t", command, owner); // Handle command as target command

    return true;
}

void ExternalEventHelper::HandlePacket(std::map<uint16, std::string>& handlers, WorldPacket const& packet, Player* owner)
{
    uint16 opcode = packet.GetOpcode(); // Get the packet opcode
    std::string const name = handlers[opcode]; // Get the handler name for the opcode
    if (name.empty())
        return;

    Trigger* trigger = aiObjectContext->GetTrigger(name); // Get the trigger for the handler
    if (!trigger)
        return;

    WorldPacket p(packet); // Copy the packet
    trigger->ExternalEvent(p, owner); // Trigger the external event with the packet
}

bool ExternalEventHelper::HandleCommand(std::string const name, std::string const param, Player* owner)
{
    Trigger* trigger = aiObjectContext->GetTrigger(name); // Get the trigger for the command name
    if (!trigger)
        return false;

    trigger->ExternalEvent(param, owner); // Trigger the external event with the command parameter

    return true;
}
