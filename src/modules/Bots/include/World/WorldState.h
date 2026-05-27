#pragma once

#include "Common.h"

enum Expansion
{
    EXPANSION_NONE = 0,
    EXPANSION_TBC = 1,
    EXPANSION_WOTLK = 2,
};

// Playerbots compatibility shim: Turtle does not expose a global WorldState
// expansion-state manager. The upstream playerbots checks only use this as a
// compile-time gate, so keep the shim minimal and return classic-era semantics.
class WorldState
{
public:
    uint32 GetExpansion() const { return EXPANSION_NONE; }
};

inline WorldState sWorldState;
