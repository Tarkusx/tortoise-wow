# Memory Leak Investigation Report

## Executive Summary
The ~15 GB RSS memory consumption during `mangosd` startup is caused by a massive, rapid memory leak in the bot pathfinding system (`TravelNodeMap::getRoute`). Specifically, temporary nodes (`PortalNode` and `TravelNode`) are dynamically allocated during A* search but are never freed because the `TravelNodeRoute` class lacks a destructor to clean up its `tempNodes` array, and callers/internal loops frequently discard the returned route objects by value.

Since bots repeatedly attempt to calculate cross-world travel paths during their initial RPG state, and these paths often fail subsequent validation checks, the pathfinding loop runs continuously. Each tick generates and leaks multiple temporary node objects per bot, rapidly accumulating into gigabytes of orphaned heap allocations.

## Detailed Findings

### 1. The Core Defect: Missing RAII in `TravelNodeRoute`
In `src/modules/Bots/upstream/playerbot/TravelNode.h`, the `TravelNodeRoute` class stores temporary nodes that are dynamically allocated during path calculation:
```cpp
    class TravelNodeRoute
    {
    public:
        // ...
        void cleanTempNodes() { for (auto node : tempNodes) delete node; }
    private:
        std::vector<TravelNode*> nodes;
        std::vector<TravelNode*> tempNodes;
    };
```
**Crucially, `TravelNodeRoute` does not define a destructor (e.g., `~TravelNodeRoute() { cleanTempNodes(); }`).** It relies entirely on the caller manually invoking `cleanTempNodes()` before the object goes out of scope. 

### 2. Leaks Inside `TravelNodeMap::getRoute`
When `sTravelNodeMap.getRoute` calculates a path for a bot, it evaluates teleport spells and hearthstone, dynamically allocating `PortalNode` and `TravelNode` objects:

#### The `PortalNode` Leak
In `TravelNode.cpp` around line 1600, `getRoute(TravelNode* start, TravelNode* goal, Unit* unit)` creates up to 8 `PortalNode` objects for the bot's teleport spells:
```cpp
    PortalNode* portNode = new PortalNode(start);
    portNode->SetPortal(start, homeNode, spellId);
    portNodes.push_back(portNode);
```
If the A* search successfully reaches the goal, it returns these nodes wrapped in a `TravelNodeRoute`:
```cpp
    return TravelNodeRoute(path, portNodes); // portNodes are moved into the route's tempNodes
```
In `TravelNodeMap::getRoute(WorldPosition startPos, WorldPosition endPos...)`, this inner `getRoute` is called in a nested loop testing up to 25 start/end node combinations. If the returned route fails the subsequent `isPathTo` reachability checks, the loop executes `continue` or `break`:
```cpp
            TravelNodeRoute route = getRoute(startNode, endNode, unit);
            // ...
            if (!startPos.isPathTo(startPath))
            {
                startPath.clear();
                badStartNodes.push_back(startNode);
                continue; // LEAK! 'route' is destroyed, but 'tempNodes' pointers are orphaned
            }
```
Because the `route` object is discarded without calling `cleanTempNodes()`, all 8 `PortalNode` allocations leak for every failed combination.

#### The Hearthstone `botNode` Leak
Later in `TravelNodeMap::getRoute`, if hearthstone is evaluated:
```cpp
        if (AI_VALUE2(bool, "action useful", "hearthstone"))
        {
            TravelNode* botNode = new TravelNode(startPos, "Bot Pos", false);
            // ...
            for (auto& endNode : endNodes)
            {
                TravelNodeRoute route = getRoute(botNode, endNode, bot);
                route.addTempNodes({botNode}); // Adds to local route object
                if (!route.isEmpty()) return route;
            }
        }
```
If all `endNodes` return an empty route, the loop finishes, the local `route` variables are destroyed, and `botNode` is never freed.

### 3. Leaks in Callers
Even when `sTravelNodeMap.getRoute` successfully returns a complete path, several callers discard the returned `TravelNodeRoute` without cleaning it. For example, in `GoAction.cpp` and `DebugAction.cpp`, the route is used or checked for `isEmpty()` and then immediately allowed to go out of scope, permanently leaking any teleport/hearthstone nodes it generated.

### 4. Why This Causes 15 GB of Memory Usage During Startup
When random bots are initialized during a clientless startup, they are placed in the world and enter the `rpg` or `travel` strategy. Because they are often far from their targets, they repeatedly invoke `ReachTargetAction` -> `getRoute`.
Since grid loading is inactive or sparse, the `isPathTo` raycast validations frequently fail, causing the bots to discard the route, leak the nodes, and try again on the very next AI tick.
At 10 AI updates per second across 8 bots, calculating 25 path combinations per update, the server is executing thousands of A* searches per second. Each failed valid path leaves behind multiple orphaned `PortalNode` heap allocations, quickly snowballing into merged heap chunks that reach 15 GB before the operating system runs out of physical memory and triggers the `std::bad_alloc` crash we observed.

## Conclusion
The 15 GB retained memory is a **true memory leak** consisting of millions of orphaned `PortalNode` and `TravelNode` objects. 

**Recommended Fix:**
1. Implement a proper RAII wrapper for `TravelNodeRoute` (e.g., using `std::shared_ptr<TravelNode>` for temporary nodes, or defining a destructor with proper copy/move semantics).
2. Ensure that any dynamically allocated node that does not end up in a `TravelNodeRoute` is deleted immediately.
