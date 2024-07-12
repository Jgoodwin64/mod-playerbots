/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotCommandServer.h"  // Include the header for PlayerbotCommandServer
#include "IoContext.h"  // Include the header for IoContext
#include "Playerbots.h"  // Include the header for Playerbots

#include <cstdlib>  // Include the standard library for general purpose functions
#include <iostream>  // Include the input-output stream library

#include <boost/bind.hpp>  // Include Boost library for binding functions
#include <boost/smart_ptr.hpp>  // Include Boost library for smart pointers
#include <boost/asio.hpp>  // Include Boost ASIO library for network and low-level I/O programming
#include <boost/thread/thread.hpp>  // Include Boost library for threading

using boost::asio::ip::tcp;  // Use the TCP protocol from Boost ASIO
typedef boost::shared_ptr<tcp::socket> socket_ptr;  // Define a type alias for a shared pointer to a TCP socket

// Function to read a line from the socket
bool ReadLine(socket_ptr sock, std::string* buffer, std::string* line)
{
    // Do the real reading from the socket until the buffer contains '\n'
    std::string::iterator pos;
    while ((pos = find(buffer->begin(), buffer->end(), '\n')) == buffer->end())
    {
        char buf[1025];  // Buffer to hold incoming data
        boost::system::error_code error;  // Variable to hold error codes
        size_t n = sock->read_some(boost::asio::buffer(buf), error);  // Read data from the socket
        if (n == -1 || error == boost::asio::error::eof)  // Check for end of file or error
            return false;  // Return false if there is an error or EOF
        else if (error)
            throw boost::system::system_error(error);  // Throw an exception for other errors

        buf[n] = 0;  // Null-terminate the buffer
        *buffer += buf;  // Append the buffer to the string
    }

    *line = std::string(buffer->begin(), pos);  // Extract the line from the buffer
    *buffer = std::string(pos + 1, buffer->end());  // Remove the extracted line from the buffer
    return true;  // Return true if a line was successfully read
}

// Function to handle a session with a client
void session(socket_ptr sock)
{
    try
    {
        std::string buffer, request;  // Strings to hold the buffer and request
        while (ReadLine(sock, &buffer, &request))  // Read lines from the socket
        {
            std::string const response = sRandomPlayerbotMgr->HandleRemoteCommand(request) + "\n";  // Handle the command and get the response
            boost::asio::write(*sock, boost::asio::buffer(response.c_str(), response.size()));  // Write the response back to the socket
            request = "";  // Clear the request string
        }
    }
    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "{}", e.what());  // Log any exceptions that occur
    }
}

// Function to run the server
void server(Acore::Asio::IoContext& io_service, short port)
{
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));  // Create a TCP acceptor to listen for incoming connections
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_service));  // Create a new socket for each connection
        a.accept(*sock);  // Accept an incoming connection
        boost::thread t(boost::bind(session, sock));  // Create a new thread to handle the session
    }
}

// Function to start the server
void Run()
{
    if (!sPlayerbotAIConfig->commandServerPort)  // Check if the command server port is configured
    {
        return;  // Exit if the port is not configured
    }

    std::ostringstream s;  // Create an output string stream
    s << "Starting Playerbots Command Server on port " << sPlayerbotAIConfig->commandServerPort;  // Create the start message
    LOG_INFO("playerbots", "{}", s.str().c_str());  // Log the start message

    try
    {
        Acore::Asio::IoContext io_service;  // Create an IO context
        server(io_service, sPlayerbotAIConfig->commandServerPort);  // Start the server
    }
    catch (std::exception& e)
    {
        LOG_ERROR("playerbots", "{}", e.what());  // Log any exceptions that occur
    }
}

// Function to start the Playerbot command server
void PlayerbotCommandServer::Start()
{
    std::thread serverThread(Run);  // Create a new thread to run the server
    serverThread.detach();  // Detach the thread to allow it to run independently
}
