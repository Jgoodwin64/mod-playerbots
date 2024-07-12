/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NAMEDOBJECTCONEXT_H
#define _PLAYERBOT_NAMEDOBJECTCONEXT_H

#include "Common.h"

#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <vector>

// Forward declaration of PlayerbotAI class
class PlayerbotAI;

// Class representing a qualified object
class Qualified
{
    public:
        // Default constructor
        Qualified() { }; // Should this be Qualified() { }?

        // Constructor initializing with a string qualifier
        Qualified(std::string const qualifier) : qualifier(qualifier) { }

        // Constructor initializing with an integer qualifier
        Qualified(int32 qualifier1)
        {
            Qualify(qualifier1);
        }

        // Virtual function to set the qualifier using an integer
        virtual void Qualify(int qual);

        // Virtual function to set the qualifier using a string
        virtual void Qualify(std::string const qual)
        {
            qualifier = qual;
        }

        // Function to get the current qualifier
        std::string const getQualifier() { return qualifier; }

        // Static function to combine multiple qualifiers into a single string
        static std::string const MultiQualify(std::vector<std::string> qualifiers);

        // Static function to split a single string of qualifiers into a vector of qualifiers
        static std::vector<std::string> getMultiQualifiers(std::string const qualifier1);

        // Static function to get a specific qualifier from a string of qualifiers
        static int32 getMultiQualifier(std::string const qualifier1, uint32 pos);

    protected:
        std::string qualifier;  // The qualifier string
};

// Template class for creating named objects
template <class T>
class NamedObjectFactory
{
    protected:
        // Type definition for the action creator function pointer
        typedef T*(*ActionCreator)(PlayerbotAI* botAI);

        // Unordered map to store the creators for different names
        std::unordered_map<std::string, ActionCreator> creators;

    public:
        // Function to create an object by name
        T* create(std::string name, PlayerbotAI* botAI)
        {
            // Find the position of the "::" delimiter in the name
            size_t found = name.find("::");
            std::string qualifier;

            // If the delimiter is found, extract the qualifier
            if (found != std::string::npos)
            {
                qualifier = name.substr(found + 2);
                name = name.substr(0, found);
            }

            // Check if a creator exists for the name
            if (creators.find(name) == creators.end())
                return nullptr;

            // Get the creator function pointer
            ActionCreator creator = creators[name];
            if (!creator)
                return nullptr;

            // Create the object using the creator function
            T* object = (*creator)(botAI);

            // If the object is qualified, set its qualifier
            Qualified* q = dynamic_cast<Qualified*>(object);
            if (q && found != std::string::npos)
                q->Qualify(qualifier);

            return object;
        }

        // Function to get the supported object names
        std::set<std::string> supports()
        {
            std::set<std::string> keys;
            for (typename std::unordered_map<std::string, ActionCreator>::iterator it = creators.begin(); it != creators.end(); it++)
                keys.insert(it->first);

            return keys;
        }
};

// Template class for managing named object contexts
template <class T>
class NamedObjectContext : public NamedObjectFactory<T>
{
    public:
        // Constructor initializing with shared and supportsSiblings flags
        NamedObjectContext(bool shared = false, bool supportsSiblings = false) :
            NamedObjectFactory<T>(), shared(shared), supportsSiblings(supportsSiblings) { }

        // Virtual destructor to clear the context
        virtual ~NamedObjectContext()
        {
            Clear();
        }

        // Function to create or get an existing object by name
        T* create(std::string const name, PlayerbotAI* botAI)
        {
            if (created.find(name) == created.end())
                return created[name] = NamedObjectFactory<T>::create(name, botAI);

            return created[name];
        }

        // Function to clear all created objects
        void Clear()
        {
            for (typename std::unordered_map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }

            created.clear();
        }

        // Function to update all created objects
        void Update()
        {
            for (typename std::unordered_map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Update();
            }
        }

        // Function to reset all created objects
        void Reset()
        {
            for (typename std::unordered_map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Reset();
            }
        }

        // Function to check if the context is shared
        bool IsShared() { return shared; }

        // Function to check if the context supports siblings
        bool IsSupportsSiblings() { return supportsSiblings; }

        // Function to get the names of all created objects
        std::set<std::string> GetCreated()
        {
            std::set<std::string> keys;
            for (typename std::unordered_map<std::string, T*>::iterator it = created.begin(); it != created.end(); it++)
                keys.insert(it->first);

            return keys;
        }

    protected:
        std::unordered_map<std::string, T*> created;  // Unordered map to store created objects
        bool shared;  // Flag indicating if the context is shared
        bool supportsSiblings;  // Flag indicating if the context supports siblings
};

// Template class for managing a list of named object contexts
template <class T>
class NamedObjectContextList
{
    public:
        // Virtual destructor to delete non-shared contexts
        virtual ~NamedObjectContextList()
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                NamedObjectContext<T>* context = *i;
                if (!context->IsShared())
                    delete context;
            }
        }

        // Function to add a context to the list
        void Add(NamedObjectContext<T>* context)
        {
            contexts.push_back(context);
        }

        // Function to get an object by name from the contexts
        T* GetContextObject(std::string const name, PlayerbotAI* botAI)
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (T* object = (*i)->create(name, botAI))
                    return object;
            }

            return nullptr;
        }

        // Function to update all non-shared contexts
        void Update()
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsShared())
                    (*i)->Update();
            }
        }

        // Function to reset all contexts
        void Reset()
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                (*i)->Reset();
            }
        }

        // Function to get sibling names for a given name
        std::set<std::string> GetSiblings(std::string const name)
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsSupportsSiblings())
                    continue;

                std::set<std::string> supported = (*i)->supports();
                std::set<std::string>::iterator found = supported.find(name);
                if (found == supported.end())
                    continue;

                supported.erase(found);
                return supported;
            }

            return std::set<std::string>();
        }

        // Function to get all supported names
        std::set<std::string> supports()
        {
            std::set<std::string> result;
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                std::set<std::string> supported = (*i)->supports();

                for (std::set<std::string>::iterator j = supported.begin(); j != supported.end(); j++)
                    result.insert(*j);
            }

            return result;
        }

        // Function to get the names of all created objects
        std::set<std::string> GetCreated()
        {
            std::set<std::string> result;
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                std::set<std::string> createdKeys = (*i)->GetCreated();

                for (std::set<std::string>::iterator j = createdKeys.begin(); j != createdKeys.end(); j++)
                    result.insert(*j);
            }

            return result;
        }

    private:
        std::vector<NamedObjectContext<T>*> contexts;  // Vector to store the list of contexts
};

// Template class for managing a list of named object factories
template <class T>
class NamedObjectFactoryList
{
    public:
        // Virtual destructor to delete all factories in the list
        virtual ~NamedObjectFactoryList()
        {
            for (typename std::list<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
                delete *i;
        }

        // Function to add a factory to the list
        void Add(NamedObjectFactory<T>* context)
        {
            factories.push_front(context);
        }

        // Function to get an object by name from the factories
        T* GetContextObject(std::string const &name, PlayerbotAI* botAI)
        {
            for (typename std::list<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
            {
                if (T* object = (*i)->create(name, botAI))
                    return object;
            }

            return nullptr;
        }

    private:
        std::list<NamedObjectFactory<T>*> factories;  // List to store the factories
};

#endif
