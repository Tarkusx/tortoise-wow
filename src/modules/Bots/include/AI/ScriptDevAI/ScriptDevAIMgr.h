#pragma once

#include "Common.h"
#include <ScriptMgr.h>

class Player;
class Creature;

// Playerbots compatibility shim: Turtle does not ship the legacy ScriptDevAI
// manager. Gossip hooks can be forwarded to Turtle's ScriptMgr, while spell
// click hooks remain stubbed until the real upstream behavior is ported.
class ScriptDevAIMgrCompatibility
{
public:
    bool OnGossipHello(Player* player, Creature* creature) const
    {
        return sScriptMgr.OnGossipHello(player, creature);
    }

    bool OnNpcSpellClick(Player*, Creature*, uint32) const
    {
        return false;
    }
};

inline ScriptDevAIMgrCompatibility sScriptDevAIMgr;
