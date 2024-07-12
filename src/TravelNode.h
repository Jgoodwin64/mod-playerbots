/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVELNODE_H
#define _PLAYERBOT_TRAVELNODE_H

#include "TravelMgr.h"
#include <shared_mutex>

// THEORY
//
// Pathfinding in (c)mangos is based on detour recast an opensource nashmesh creation and pathfinding codebase.
// This system is used for mob and npc pathfinding and in this codebase also for bots.
// Because mobs and npc movement is based on following a player or a set path the PathGenerator is limited to 296y.
// This means that when trying to find a path from A to B distances beyond 296y will be a best guess often moving in a straight path.
// Bots would get stuck moving from Northshire to Stormwind because there is no 296y path that doesn't go (initially) the wrong direction.
//
// To remedy this limitation without altering the PathGenerator limits too much this node system was introduced.
//
//  <S> ---> [N1] ---> [N2] ---> [N3] ---> <E>
//
// Bot at <S> wants to move to <E>
// [N1],[N2],[N3] are predefined nodes for wich we know we can move from [N1] to [N2] and from [N2] to [N3] but not from [N1] to [N3]
// If we can move fom [S] to [N1] and from [N3] to [E] we have a complete route to travel.
//
// Termonology:
// Node: a location on a map for which we know bots are likely to want to travel to or need to travel past to reach other nodes.
// Link: the connection between two nodes. A link signifies that the bot can travel from one node to another. A link is one-directional.
// Path: the waypointpath returned by the standard PathGenerator to move from one node (or position) to another. A path can be imcomplete or empty which means there is no link.
// Route: the list of nodes that give the shortest route from a node to a distant node. Routes are calculated using a standard A* search based on links.
//
// On server start saved nodes and links are loaded. Paths and routes are calculated on the fly but saved for future use.
// Nodes can be added and removed realtime however because bots access the nodes from different threads this requires a locking mechanism.
//
// Initially the current nodes have been made:
// Flightmasters and Inns (Bots can use these to fast-travel so eventually they will be included in the route calculation)
// WorldBosses and Unique bosses in instances (These are a logical places bots might want to go in instances)
// Player start spawns (Obviously all lvl1 bots will spawn and move from here)
// Area triggers locations with teleport and their teleport destinations (These used to travel in or between maps)
// Transports including elevators (Again used to travel in and in maps)
// (sub)Zone means (These are the center most point for each sub-zone which is good for global coverage)
//
// To increase coverage/linking extra nodes can be automatically be created.
// Current implentation places nodes on paths (including complete) at sub-zone transitions or randomly.
// After calculating possible links the node is removed if it does not create local coverage.
//

// Enumeration representing the different types of travel node paths.
enum class TravelNodePathType : uint8
{
    none          = 0,  // No path type.
    walk          = 1,  // Walking path type.
    portal        = 2,  // Portal path type.
    transport     = 3,  // Transport path type.
    flightPath    = 4,  // Flight path type.
    teleportSpell = 5   // Teleport spell path type.
};

// Class representing a connection between two travel nodes.
class TravelNodePath
{
    public:
        //Legacy Constructor for travelnodestore
        //TravelNodePath(float distance1, float extraCost1, bool portal1 = false, uint32 portalId1 = 0, bool transport1 = false, bool calculated = false, uint8 maxLevelMob1 = 0, uint8 maxLevelAlliance1 = 0, uint8 maxLevelHorde1 = 0, float swimDistance1 = 0, bool flightPath1 = false);
        
	// Constructor for TravelNodePath with various parameters
        TravelNodePath(float distance = 0.1f, float extraCost = 0, uint8 pathType = (uint8)TravelNodePathType::walk, uint32 pathObject = 0, bool calculated = false,
            std::vector<uint8> maxLevelCreature = { 0, 0, 0 }, float swimDistance = 0)
            : extraCost(extraCost), calculated(calculated), distance(distance), maxLevelCreature(maxLevelCreature), swimDistance(swimDistance), pathType(TravelNodePathType(pathType)), pathObject(pathObject) // reorder args - whipowill
        {
            if (pathType != (uint8)TravelNodePathType::walk)
                complete = true;
        };

        // Copy constructor for TravelNodePath
        TravelNodePath(TravelNodePath* basePath)
        {
            complete = basePath->complete;
            path = basePath->path;
            extraCost = basePath->extraCost;
            calculated = basePath->calculated;
            distance = basePath->distance;
            maxLevelCreature = basePath->maxLevelCreature;
            swimDistance = basePath->swimDistance;
            pathType = basePath->pathType;
            pathObject = basePath->pathObject;
        };

        // Getters for various path attributes
        bool getComplete() { return complete || pathType != TravelNodePathType::walk; }
        std::vector<WorldPosition> getPath() { return path; }

        TravelNodePathType getPathType() { return pathType; }
        uint32 getPathObject() { return pathObject; }

        float getDistance() { return distance; }
        float getSwimDistance() { return swimDistance; }
        float getExtraCost(){ return extraCost; }
        std::vector<uint8> getMaxLevelCreature(){ return  maxLevelCreature; }

        // Setter to mark the path as calculated
        void setCalculated(bool calculated1 = true) { calculated = calculated1; }

        // Getter to check if the path is calculated.
        bool getCalculated() { return calculated; }

        // Print function for debugging
        std::string const print();

        // Setters for various path attributes
        void setComplete(bool complete1)
        {
            complete = complete1;
        }

        void setPath(std::vector<WorldPosition> path1)
        {
            path = path1;
        }

        void setPathAndCost(std::vector<WorldPosition> path1, float speed)
        {
            setPath(path1);
            calculateCost(true);
            extraCost = distance / speed;
        }

        //void setPortal(bool portal1, uint32 portalId1 = 0) { portal = portal1; portalId = portalId1; }
        //void setTransport(bool transport1) { transport = transport1; }

        void setPathType(TravelNodePathType pathType1)
        {
            pathType = pathType1;
        }

        void setPathObject(uint32 pathObject1)
        {
            pathObject = pathObject1;
        }

        // Function to calculate the cost of the path
        void calculateCost(bool distanceOnly = false);

        // Function to get the cost of the path
        float getCost(Player* bot = nullptr, uint32 cGold = 0);
        uint32 getPrice();

    private:
        // Flag to check if the path is complete
        bool complete = false;

        // List of WorldPositions to get to the destination.
        std::vector<WorldPosition> path = {};

        // Extra (loading/transport) time it takes to take this path.
        float extraCost = 0;

        // Flag to check if the path is calculated
        bool calculated = false;

        // Derived distance in yards
        float distance = 0.1f;

        // Maximum level of creatures along the path
        std::vector<uint8> maxLevelCreature = { 0, 0, 0 }; // mobs, horde, alliance

        // Swimming distances along the path
        float swimDistance = 0;

        // Type of the path.
        TravelNodePathType pathType = TravelNodePathType::walk;
        // Object associated with the path
        uint32 pathObject = 0;

        /*
        //Is the path a portal/teleport to the destination?
        bool portal = false;

        //Area trigger Id
        uint32 portalId = 0;

        //Is the path transport based?
        bool transport = false;

        // Is the path a flightpath?
        bool flightPath = false;
        */
};

//A waypoint to travel from or to
//Each node knows which other nodes can be reached without help.
// Class representing a waypoint to travel from or to.
class TravelNode
{
    public:
        // Default constructor
        TravelNode() { };

        // Constructor for TravelNode with position and name
        TravelNode(WorldPosition point1, std::string const nodeName1 = "Travel Node", bool important1 = false)
        {
            nodeName = nodeName1;
            point = point1;
            important = important1;
        }

        // Copy constructor for TravelNode
        TravelNode(TravelNode* baseNode)
        {
            nodeName = baseNode->nodeName;
            point = baseNode->point;
            important = baseNode->important;
        }

        // Setter for linked flag
        void setLinked(bool linked1) { linked = linked1; }
        // Setter for point.
        void setPoint(WorldPosition point1) { point = point1; }

        // Getter for node name
        std::string const getName() { return nodeName; };
        // Getter for position
        WorldPosition* getPosition() { return &point; };
        // Getter for paths
        std::unordered_map<TravelNode*, TravelNodePath>* getPaths() { return &paths; }
        // Getter for links
        std::unordered_map<TravelNode*, TravelNodePath*>* getLinks() { return &links; }
        // Check if the node is important
        bool isImportant() { return important; };
        // Check if the node is linked
        bool isLinked() { return linked; }

        // Check if the node is a transport node
        bool isTransport()
        {
            for (auto const& link : *getLinks())
                if (link.second->getPathType() == TravelNodePathType::transport)
                    return true;

            return false;
        }

        // Get the transport ID of the node
        uint32 getTransportId()
        {
            for (auto const& link : *getLinks())
                if (link.second->getPathType() == TravelNodePathType::transport)
                    return link.second->getPathObject();

            return false;
        }

        // Check if the node is a portal node
        bool isPortal()
        {
            for (auto const& link : *getLinks())
                if (link.second->getPathType() == TravelNodePathType::portal)
                    return true;

            return false;
        }

        // Check if the node has a walking path
        bool isWalking()
        {
            for (auto link : *getLinks())
                if (link.second->getPathType() == TravelNodePathType::walk)
                    return true;

            return false;
        }

        // WorldLocation shortcuts
        uint32 getMapId() { return point.getMapId(); }
        float getX() { return point.getX(); }
        float getY() { return point.getY(); }
        float getZ() { return point.getZ(); }
        float getO() { return point.getO(); }
        float getDistance(WorldPosition pos) { return point.distance(pos); }
        float getDistance(TravelNode* node) { return point.distance(node->getPosition()); }
        float fDist(TravelNode* node) { return point.fDist(node->getPosition()); }
        float fDist(WorldPosition pos) { return point.fDist(pos); }

        // Function to set a path to another node
        TravelNodePath* setPathTo(TravelNode* node, TravelNodePath path = TravelNodePath(), bool isLink = true)
        {
            if (this != node)
            {
                paths[node] = path;
                if (isLink)
                    links[node] = &paths[node];

                return &paths[node];
            }

            return nullptr;
        }

        // Check if there is a path to another node
        bool hasPathTo(TravelNode* node) { return paths.find(node) != paths.end(); }
        // Get the path to another node.
        TravelNodePath* getPathTo(TravelNode* node) { return &paths[node]; }
        // Check if there is a complete path to another node.
        bool hasCompletePathTo(TravelNode* node) { return hasPathTo(node) && getPathTo(node)->getComplete(); }
        // Function to build a path to another node.
        TravelNodePath* buildPath(TravelNode* endNode, Unit* bot, bool postProcess = false);

        // Function to set a link to another node
        void setLinkTo(TravelNode* node, float distance = 0.1f)
        {
            if (this != node)
            {
                if (!hasPathTo(node))
                    setPathTo(node, TravelNodePath(distance));
                else
                    links[node] = &paths[node];
            }
        }

        // Check if there is a link to another node
        bool hasLinkTo(TravelNode* node) { return links.find(node) != links.end(); }
        // Get the cost of the link to another node
        float linkCostTo(TravelNode* node) { return paths.find(node)->second.getDistance(); }
        // Get the distance of the link to another node
        float linkDistanceTo(TravelNode* node) { return paths.find(node)->second.getDistance(); }
        // Function to remove a link to another node
        void removeLinkTo(TravelNode* node, bool removePaths = false);

        // Check if the current node is equal to another node
        bool isEqual(TravelNode* compareNode);

        // Function to remove links to other nodes that can also be reached by passing another node
        bool isUselessLink(TravelNode* farNode);
        void cropUselessLink(TravelNode* farNode);
        bool cropUselessLinks();

        // Function to get all nodes that can be reached from this node
        std::vector<TravelNode*> getNodeMap(bool importantOnly = false, std::vector<TravelNode*> ignoreNodes = {});

        // Check if it is possible to route to another node
        bool hasRouteTo(TravelNode* node)
        {
            if (routes.empty())
                for (auto mNode : getNodeMap())
                    routes[mNode] = true;

            return routes.find(node) != routes.end();
        };

        // Print function for debugging.
        void print(bool printFailed = true);

    protected:
        // Logical name of the node
        std::string nodeName;
        // WorldPosition of the node
        WorldPosition point;

        // List of paths to other nodes
        std::unordered_map<TravelNode*, TravelNodePath> paths;
        // List of links to other nodes
        std::unordered_map<TravelNode*, TravelNodePath*> links;

        // List of nodes and if there is 'any' route possible
        std::unordered_map<TravelNode*, bool> routes;

        // Flag to indicate if the node is important
        bool important = false;

        // Flag to indicate if the node has been checked for nearby links
        bool linked = false;

        //This node is a (moving) transport
        //bool transport = false;
        //Entry of transport
        //uint32 transportId = 0;
};

// Class representing a portal node
class PortalNode : public TravelNode
{
    public:
        // Constructor for PortalNode.
        PortalNode(TravelNode* baseNode) : TravelNode(baseNode) { };

        // Function to set a portal from one node to another.
        void SetPortal(TravelNode* baseNode, TravelNode* endNode, uint32 portalSpell)
        {
            nodeName = baseNode->getName();
            point = *baseNode->getPosition();
            paths.clear();
            links.clear();
            TravelNodePath path(0.1f, 0.1f, (uint8)TravelNodePathType::teleportSpell, portalSpell, true);
            setPathTo(endNode, path);
        };
};

// Enumeration representing the different types of route steps
enum PathNodeType
{
    NODE_PREPATH    = 0,  // Prepath node type
    NODE_PATH       = 1,  // Path node type
    NODE_NODE       = 2,  // Node type
    NODE_PORTAL     = 3,  // Portal node type
    NODE_TRANSPORT  = 4,  // Transport node type
    NODE_FLIGHTPATH = 5,  // Flightpath node type
    NODE_TELEPORT   = 6   // Teleport node type
};

// Struct representing a point in a route step
struct PathNodePoint
{
    WorldPosition point;         // Position of the point
    PathNodeType type = NODE_PATH; // Type of the route step
    uint32 entry = 0;            // Entry ID
};

// Class representing a complete list of points the bot has to walk or teleport to
class TravelPath
{
    public:
        // Default constructor
        TravelPath() { };
        // Constructor with a full path
        TravelPath(std::vector<PathNodePoint> fullPath1) { fullPath = fullPath1; }
        // Constructor with a path and type
        TravelPath(std::vector<WorldPosition> path, PathNodeType type = NODE_PATH, uint32 entry = 0)
        {
            addPath(path, type, entry);
        }

        // Function to add a point to the path
        void addPoint(PathNodePoint point) { fullPath.push_back(point); }
        // Function to add a point with type and entry to the path
        void addPoint(WorldPosition point, PathNodeType type = NODE_PATH, uint32 entry = 0) { fullPath.push_back(PathNodePoint{ point, type, entry }); }
        // Function to add a path with type and entry to the path
        void addPath(std::vector<WorldPosition> path, PathNodeType type = NODE_PATH, uint32 entry = 0) { for (auto& p : path) { fullPath.push_back(PathNodePoint{ p, type, entry }); }; }
        // Function to add a new path to the path
        void addPath(std::vector<PathNodePoint> newPath) { fullPath.insert(fullPath.end(), newPath.begin(), newPath.end()); }
        // Function to clear the path
        void clear() { fullPath.clear(); }

        // Check if the path is empty
        bool empty() { return fullPath.empty(); }
        // Getter for the path.
        std::vector<PathNodePoint> getPath() { return fullPath; }
        // Getter for the front point of the path
        WorldPosition getFront() {return fullPath.front().point; }
        // Getter for the back point of the path
        WorldPosition getBack() { return fullPath.back().point; }

        // Function to get a vector of WorldPositions from the path
        std::vector<WorldPosition> getPointPath()
        {
            std::vector<WorldPosition> retVec;
            for (auto const& p : fullPath)
                retVec.push_back(p.point);
            return retVec;
        };

        // Function to make a shortcut in the path
        bool makeShortCut(WorldPosition startPos, float maxDist);
        // Function to check if the bot should move to the next point
        bool shouldMoveToNextPoint(WorldPosition startPos, std::vector<PathNodePoint>::iterator beg, std::vector<PathNodePoint>::iterator ed, std::vector<PathNodePoint>::iterator p, float& moveDist, float maxDist);
        // Function to get the next point in the path
        WorldPosition getNextPoint(WorldPosition startPos, float maxDist, TravelNodePathType& pathType, uint32& entry);

        // Print function for debugging
        std::ostringstream const print();

    private:
        // Vector of PathNodePoints representing the full path
        std::vector<PathNodePoint> fullPath;
};

// Class representing an A* search route
class TravelNodeRoute
{
    public:
        // Default constructor
        TravelNodeRoute() { }
        // Constructor with a vector of nodes
        TravelNodeRoute(std::vector<TravelNode*> nodes1) { nodes = nodes1; /*currentNode = route.begin();*/ }

        // Check if the route is empty
        bool isEmpty() { return nodes.empty(); }

        // Check if the route contains a specific node
        bool hasNode(TravelNode* node) { return findNode(node) != nodes.end(); }
        // Function to get the total distance of the route
        float getTotalDistance();

        // Getter for the nodes in the route
        std::vector<TravelNode*> getNodes() { return nodes; }

        // Function to build a path from the route
        TravelPath buildPath(std::vector<WorldPosition> pathToStart = {}, std::vector<WorldPosition> pathToEnd = {}, Unit* bot = nullptr);

        // Print function for debugging
        std::ostringstream const print();

    private:
        // Function to find a specific node in the route
        std::vector<TravelNode*>::iterator findNode(TravelNode* node) { return std::find(nodes.begin(), nodes.end(), node); }
        // Vector of nodes representing the route
        std::vector<TravelNode*> nodes;
};

// Class representing a node container to aid A* calculations
class TravelNodeStub
{
    public:
        // Constructor with a TravelNode pointer
        TravelNodeStub(TravelNode* dataNode1) { dataNode = dataNode1; }

        // TravelNode pointer representing the data node
        TravelNode* dataNode;
        // A* search attributes
        float m_f = 0.0, m_g = 0.0, m_h = 0.0;
        // Flags to indicate if the node is open or closed
        bool open = false, close = false;
        // Parent node in the A* search
        TravelNodeStub* parent = nullptr;
        // Current gold of the bot
        uint32 currentGold = 0;
};

// Class representing the container of all nodes
class TravelNodeMap
{
    public:
        // Default constructor
        TravelNodeMap() { };
        // Constructor with a base map
        TravelNodeMap(TravelNodeMap* baseMap);

        // Singleton instance of TravelNodeMap
        static TravelNodeMap* instance()
        {
            static TravelNodeMap instance;
            return &instance;
        }

        // Function to add a node to the map
        TravelNode* addNode(WorldPosition pos, std::string const preferedName = "Travel Node", bool isImportant = false, bool checkDuplicate = true, bool transport = false, uint32 transportId = 0);
        // Function to remove a node from the map
        void removeNode(TravelNode* node);
        // Function to remove all nodes from the map
        bool removeNodes()
        {
            if (m_nMapMtx.try_lock_for(std::chrono::seconds(10)))
            {
                for (auto& node : m_nodes)
                    removeNode(node);

                m_nMapMtx.unlock();
                return true;
            }

            return false;
        };

        // Function to fully link a node
        void fullLinkNode(TravelNode* startNode, Unit* bot);

        // Getter for all nodes
        std::vector<TravelNode*> getNodes() { return m_nodes; }
        // Getter for nodes within a specific range
        std::vector<TravelNode*> getNodes(WorldPosition pos, float range = -1);

        // Function to find the nearest node to another node
        TravelNode* getNode(TravelNode* sameNode)
        {
            for (auto& node : m_nodes)
            {
                if (node->getName() == sameNode->getName() && node->getPosition() == sameNode->getPosition())
                    return node;
            }

            return nullptr;
        }

        // Function to find the nearest node to a position with a path
        TravelNode* getNode(WorldPosition pos, std::vector<WorldPosition>& ppath, Unit* bot = nullptr, float range = -1);
        // Function to find the nearest node to a position
        TravelNode* getNode(WorldPosition pos, Unit* bot = nullptr, float range = -1)
        {
            std::vector<WorldPosition> ppath;
            return getNode(pos, ppath, bot, range);
        }

        // Function to get a random node near a position
        TravelNode* getRandomNode(WorldPosition pos)
        {
            std::vector<TravelNode*> rNodes = getNodes(pos);
            if (rNodes.empty())
                return nullptr;

            return rNodes[urand(0, rNodes.size() - 1)];
        }

        // Function to find the best node path between two nodes
        TravelNodeRoute getRoute(TravelNode* start, TravelNode* goal, Player* bot = nullptr);

        // Function to find the best node path between two positions
        TravelNodeRoute getRoute(WorldPosition startPos, WorldPosition endPos, std::vector<WorldPosition>& startPath, Player* bot = nullptr);

        // Function to find the full path between two locations
        static TravelPath getFullPath(WorldPosition startPos, WorldPosition endPos, Player* bot = nullptr);

        // Function to manage/update nodes
        void manageNodes(Unit* bot, bool mapFull = false);

        // Function to set the flag to generate nodes
        void setHasToGen() { hasToGen = true; }

        // Function to generate NPC nodes
        void generateNpcNodes();
        // Function to generate start nodes
        void generateStartNodes();
        // Function to generate area trigger nodes
        void generateAreaTriggerNodes();
        // Function to generate nodes
        void generateNodes();
        // Function to generate transport nodes
        void generateTransportNodes();
        // Function to generate zone mean nodes
        void generateZoneMeanNodes();

        // Function to generate walking paths
        void generateWalkPaths();
        // Function to remove low nodes
        void removeLowNodes();
        // Function to remove useless paths
        void removeUselessPaths();
        // Function to calculate path costs
        void calculatePathCosts();
        // Function to generate taxi paths
        void generateTaxiPaths();
        // Function to generate paths
        void generatePaths();

        // Function to generate all nodes and paths
        void generateAll();

        // Function to print the map
        void printMap();

        // Function to print the node store
        void printNodeStore();
        // Function to save the node store
        void saveNodeStore();
        // Function to load the node store
        void loadNodeStore();

        // Function to crop useless nodes
        bool cropUselessNode(TravelNode* startNode);
        // Function to add a zone link node
        TravelNode* addZoneLinkNode(TravelNode* startNode);
        // Function to add a random external node
        TravelNode* addRandomExtNode(TravelNode* startNode);

        // Function to calculate the map offset
        void calcMapOffset();
        // Getter for the map offset of a specific map
        WorldPosition getMapOffset(uint32 mapId)

        // Mutex for node map operations
        std::shared_timed_mutex m_nMapMtx;
        // Map of teleport nodes
        std::unordered_map<ObjectGuid, std::unordered_map<uint32, TravelNode*>> teleportNodes;

    private:
        // Vector of all nodes
        std::vector<TravelNode*> m_nodes;

        // Vector of map offsets
        std::vector<std::pair<uint32, WorldPosition>> mapOffsets;

        // Flags for node generation and saving
        bool hasToSave = false
        bool hasToGen = false;
        bool hasToFullGen = false;
};

#define sTravelNodeMap TravelNodeMap::instance()

#endif
