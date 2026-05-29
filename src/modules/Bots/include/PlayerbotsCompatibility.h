#pragma once

#ifndef PLAYERBOT_RUNTIME_BOOTSTRAP
#define PLAYERBOT_RUNTIME_BOOTSTRAP 1
#endif

#include "Common.h"
#include "Database/DBCEnums.h"
#include "Globals/ObjectMgr.h"
#include "Maps/Map.h"
#include "ObjectGuid.h"
#include "Objects/GameObject.h"
#include "Objects/Creature.h"
#include "Database/DBCStructure.h"
#include "Spells/SpellMgr.h"

#include <algorithm>
#include <cstddef>
#include <chrono>

#ifndef DEFAULT_MAX_LEVEL
#define DEFAULT_MAX_LEVEL PLAYER_MAX_LEVEL
#endif

#ifndef UNIT_FLAG_CLIENT_CONTROL_LOST
#define UNIT_FLAG_CLIENT_CONTROL_LOST UNIT_FLAG_UNK_0
#endif

#define InterruptMoving(force) StopMoving(force)

#ifndef TEAM_BOTH_ALLOWED
constexpr Team TEAM_BOTH_ALLOWED = TEAM_NONE;
#endif

constexpr MovementFlags movementFlagsMask = MOVEFLAG_MASK_MOVING_OR_TURN;

using GuidSet = ObjectGuidSet;
using AreaTableEntry = AreaEntry;
using AreaTrigger = AreaTriggerEntry;
using ClientLootType = LootType;
using TimePoint = std::chrono::steady_clock::time_point;

class BarGoLink
{
public:
    explicit BarGoLink(std::size_t) {}
    void step() {}
    static void SetOutputState(bool) {}
};

class PlayerbotsSpellTemplateCompatibility
{
public:
    template <typename T>
    T const* LookupEntry(uint32 id) const
    {
        return static_cast<T const*>(sSpellMgr.GetSpellEntry(id));
    }

    uint32 GetMaxEntry() const
    {
        return sSpellMgr.GetMaxSpellId();
    }
};

inline PlayerbotsSpellTemplateCompatibility sSpellTemplate;

class PlayerbotsItemStorageCompatibility
{
public:
    template <typename T>
    T const* LookupEntry(uint32 id) const
    {
        return static_cast<T const*>(sObjectMgr.GetItemPrototype(id));
    }

    uint32 GetMaxEntry() const
    {
        uint32 maxEntry = 0;
        for (auto const& [id, _] : sObjectMgr.GetItemPrototypeMap())
            maxEntry = std::max(maxEntry, id);

        return maxEntry + 1;
    }
};

inline PlayerbotsItemStorageCompatibility sItemStorage;

class PlayerbotsCreatureStorageCompatibility
{
public:
    CreatureInfo const* LookupEntry(uint32 id) const
    {
        return sObjectMgr.GetCreatureTemplate(id);
    }

    template <typename T>
    T const* LookupEntry(uint32 id) const
    {
        return static_cast<T const*>(sObjectMgr.GetCreatureTemplate(id));
    }

    uint32 GetMaxEntry() const
    {
        uint32 maxEntry = 0;
        for (auto const& [id, _] : sObjectMgr.GetCreatureInfoMap())
            maxEntry = std::max(maxEntry, id);

        return maxEntry + 1;
    }
};

inline PlayerbotsCreatureStorageCompatibility sCreatureStorage;

class PlayerbotsGameObjectStorageCompatibility
{
public:
    GameObjectInfo const* LookupEntry(uint32 id) const
    {
        return sObjectMgr.GetGameObjectInfo(id);
    }

    template <typename T>
    T const* LookupEntry(uint32 id) const
    {
        return static_cast<T const*>(sObjectMgr.GetGameObjectInfo(id));
    }

    uint32 GetMaxEntry() const
    {
        uint32 maxEntry = 0;
        for (auto const& [id, _] : sObjectMgr.GetGameObjectInfoMap())
            maxEntry = std::max(maxEntry, id);

        return maxEntry + 1;
    }
};

inline PlayerbotsGameObjectStorageCompatibility sGOStorage;

class PlayerbotsFactionTemplateCompatibility
{
public:
    FactionTemplateEntry const* LookupEntry(uint32 id) const
    {
        return sObjectMgr.GetFactionTemplateEntry(id);
    }

    uint32 GetNumRows() const
    {
        return static_cast<uint32>(sObjectMgr.GetFactionTemplateMap().size());
    }

    uint32 GetMaxEntry() const
    {
        uint32 maxEntry = 0;
        for (auto const& [id, _] : sObjectMgr.GetFactionTemplateMap())
            maxEntry = std::max(maxEntry, id);

        return maxEntry + 1;
    }
};

inline PlayerbotsFactionTemplateCompatibility sFactionTemplateStore;

class PlayerbotsFactionCompatibility
{
public:
    FactionEntry const* LookupEntry(uint32 id) const
    {
        return sObjectMgr.GetFactionEntry(id);
    }

    template <typename T>
    T const* LookupEntry(uint32 id) const
    {
        return static_cast<T const*>(sObjectMgr.GetFactionEntry(id));
    }

    uint32 GetNumRows() const
    {
        return static_cast<uint32>(sObjectMgr.GetFactionMap().size());
    }

    uint32 GetMaxEntry() const
    {
        uint32 maxEntry = 0;
        for (auto const& [id, _] : sObjectMgr.GetFactionMap())
            maxEntry = std::max(maxEntry, id);

        return maxEntry + 1;
    }
};

inline PlayerbotsFactionCompatibility sFactionStore;

class Player;
class PlayerbotAI;
class PlayerbotHolder;
struct Loot;
class Corpse;
class Item;
class Transport;

using GenericTransport = Transport;

inline AreaTableEntry const* GetAreaEntryByAreaID(uint32 id)
{
    return AreaEntry::GetById(id);
}

inline AreaTableEntry const* GetAreaEntryByAreaFlagAndMap(uint32 areaFlag, uint32 mapId)
{
    return AreaEntry::GetByAreaFlagAndMap(areaFlag, mapId);
}

namespace PlayerbotsCompatibility
{
    inline uint32 GetTrainerLearnedSpellId(TrainerSpell const* trainerSpell)
    {
        if (!trainerSpell)
            return 0;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(trainerSpell->spell);
        if (!spellInfo)
            return 0;

        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL && spellInfo->EffectTriggerSpell[i])
                return spellInfo->EffectTriggerSpell[i];

        return 0;
    }

    void LearnClassLevelSpells(Player* player, bool includeTrainerSpells);

    inline bool IsInGroup(Player const* player, Player const* other, bool includeRaid = false)
    {
        if (!player || !other)
            return false;

        if (includeRaid)
            return player->IsInSameGroupWith(other) || player->IsInSameRaidWith(other);

        return player->IsInSameGroupWith(other);
    }

    class LootMgrCompatibility
    {
    public:
        Loot* GetLoot(Player* player, ObjectGuid guid) const;
        Loot* GetLoot(Player* player) const;
    };

    class TaxiNodesStoreCompatibility
    {
    public:
        uint32 GetNumRows() const
        {
            return sObjectMgr.GetMaxTaxiNodeId();
        }

        TaxiNodesEntry const* LookupEntry(uint32 id) const
        {
            return sObjectMgr.GetTaxiNodeEntry(id);
        }
    };

    inline uint32 GetQuestSlotQuestId(Player const* player, uint16 slot)
    {
        return player->GetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET + QUEST_ID_OFFSET);
    }

    inline bool IsGameObjectInUse(GameObject const* go)
    {
        return go && go->GetUseCount() > 0;
    }

    inline bool HasRealPlayerInZone(Map* map, uint32 zoneId)
    {
        if (!map)
            return false;

        for (auto itr = map->GetPlayers().getFirst(); itr != nullptr; itr = itr->next())
        {
            if (auto player = itr->getSource())
            {
                if (player->isRealPlayer() && player->GetZoneId() == zoneId)
                    return true;
            }
        }

        return false;
    }

    PlayerbotHolder* GetPlayerbotMgr(Player* player);

    inline char const* GetAreaName(AreaEntry const* entry, uint32 /*locale*/ = 0)
    {
        return entry && entry->Name ? entry->Name : "";
    }

    inline float GetAttackDistance(Unit const* unit, Unit const* target)
    {
        if (!unit)
            return 0.0f;

        if (Creature const* creature = unit->ToCreature())
            return creature->GetAttackDistance(target);

        return unit->GetCombatReach(target, false, 0.0f);
    }

    uint32 CalculateTalentPoints(Player const* player);
    WorldLocation GetSpawnLocation(CreatureDataPair const* pair);
    WorldLocation GetSpawnLocation(GameObjectDataPair const* pair);
    inline Item* GetItemByEntry(Player const* player, uint32 entry)
    {
        Item* found = nullptr;
        if (player)
        {
            player->ApplyForAllItems([entry, &found](Item* item)
            {
                if (!found && item->GetEntry() == entry)
                    found = item;
            }, false);
        }
        return found;
    }
    bool MeleeAttackStart(Player* player, Unit* target);
    bool MeleeAttackStop(Player* player, Unit* target);
}

inline PlayerbotsCompatibility::LootMgrCompatibility sLootMgr;
inline PlayerbotsCompatibility::TaxiNodesStoreCompatibility sTaxiNodesStore;

#define IsRaidGroup isRaidGroup
#define CanReachWithMeleeAttack CanReachWithMeleeAutoAttack
#define IsInSwimmableWater IsReachableBySwmming

#define GetPlayerMenu() PlayerTalkClass
#define GetLootState getLootState
#define GetSpellRecoveryTime(proto) (proto)->GetRecoveryTime()

#define TRIGGERED_OLD_TRIGGERED true
#define TRIGGERED_NONE false
#define GetMasterLooterGuid GetLooterGuid

#define TAXI_MOTION_TYPE FLIGHT_MOTION_TYPE
#define UnitAI CreatureAI


struct AuctionEntry;
uint32 GetAuctionItemCount(AuctionEntry const* entry);


#define coord_x x
#define coord_y y
#define coord_z z

typedef uint32 ForcedMovement;
#define FORCED_MOVEMENT_NONE MOVE_NONE
#define FORCED_MOVEMENT_RUN MOVE_RUN_MODE
#define FORCED_MOVEMENT_WALK MOVE_WALK_MODE
#define FORCED_MOVEMENT_FLIGHT MOVE_FLY_MODE

#include <vector>
#include "G3D/Vector3.h"
class Unit;
void MovePath(Unit* bot, std::vector<G3D::Vector3> const& path, uint32 options, bool cyclic, bool falling = false);

enum BG_AB_GameObjects_compat
{
    BG_AB_BANNER_ALLIANCE       = 180087,
    BG_AB_BANNER_HORDE          = 180088,
    BG_AB_BANNER_CONTESTED_A    = 180089,
    BG_AB_BANNER_CONTESTED_H    = 180090,
    BG_AB_BANNER_STABLE         = 180076,
    BG_AB_BANNER_BLACKSMITH     = 180078,
    BG_AB_BANNER_FARM           = 180080,
    BG_AB_BANNER_LUMBER_MILL    = 180082,
    BG_AB_BANNER_MINE           = 180084,
};

#define GO_WS_SILVERWING_FLAG WS_ALLIANCE_FLAG_BASE
#define GO_WS_WARSONG_FLAG WS_HORDE_FLAG_BASE
#define GO_WS_SILVERWING_FLAG_DROP WS_ALLIANCE_FLAG_GROUND
#define GO_WS_WARSONG_FLAG_DROP WS_HORDE_FLAG_GROUND

#define GetTeamIndexByTeamId BattleGround::GetTeamIndexByTeamId
#define HandleBattlefieldPortOpcode HandleBattleFieldPortOpcode
#define CanInteract CanInteractWithGameObject
// Replaced toxic IsInUse() macro with inline free function to avoid collision with SpellAuras::IsInUse()
inline bool GameObjectIsInUse(const GameObject* go) { return go && go->getLootState() == GO_ACTIVATED; }
// Turtle uses lowercase getLootState(); upstream uses GetLootState()
#define GetLootState getLootState
#define DIST_CALC_NONE SizeFactor::None

#define WS_AT_WARSONG_ROOM AREATRIGGER_HORDE_FLAG_SPAWN
#define WS_AT_SILVERWING_ROOM AREATRIGGER_ALLIANCE_FLAG_SPAWN

#include "Battlegrounds/BattleGroundWS.h"
inline ObjectGuid GetFlagCarrierGuid(BattleGroundWS* bg, uint32 teamIndex)
{
    return bg ? (teamIndex == BG_TEAM_ALLIANCE ? bg->GetAllianceFlagPickerGuid() : bg->GetHordeFlagPickerGuid()) : ObjectGuid();
}

// ---- BattleGround bracket ID: in Turtle this is a static method on Player ----
#include "Objects/Player.h"
inline BattleGroundBracketId GetBGBracketIdFromLevel(BattleGroundTypeId bgTypeId, uint32 level)
{
    return Player::GetBattleGroundBracketIdFromLevel(bgTypeId, level);
}

// ---- BG_AB_NODE_STATUS_NEUTRAL = BG_AB_NODE_TYPE_NEUTRAL in Turtle ----
#include "Battlegrounds/BattleGroundAB.h"
#ifndef BG_AB_NODE_STATUS_NEUTRAL
#define BG_AB_NODE_STATUS_NEUTRAL BG_AB_NODE_TYPE_NEUTRAL
#endif


// ---- Position::GetDistance does not exist in Turtle Position struct ----
#include <cmath>
inline float PositionGetDistance(const Position& a, const Position& b)
{
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}
// ---- BG_AV_NODE_STATUS_* aliases for Turtle's BG_AV_Event_Control_State ----
#ifndef BG_AV_NODE_STATUS_ALLY_CONTESTED
#define BG_AV_NODE_STATUS_ALLY_CONTESTED ALLIANCE_ASSAULTED
#endif
#ifndef BG_AV_NODE_STATUS_ALLY_OCCUPIED
#define BG_AV_NODE_STATUS_ALLY_OCCUPIED ALLIANCE_CONTROLLED
#endif
#ifndef BG_AV_NODE_STATUS_HORDE_CONTESTED
#define BG_AV_NODE_STATUS_HORDE_CONTESTED HORDE_ASSAULTED
#endif
#ifndef BG_AV_NODE_STATUS_HORDE_OCCUPIED
#define BG_AV_NODE_STATUS_HORDE_OCCUPIED HORDE_CONTROLLED
#endif// ---- TEAM_INDEX_* aliases for Turtle's BattleGroundTeamIndex / BattleGroundAVTeamIndex ----
#ifndef TEAM_INDEX_ALLIANCE
#define TEAM_INDEX_ALLIANCE 0
#endif
#ifndef TEAM_INDEX_HORDE
#define TEAM_INDEX_HORDE 1
#endif
#ifndef TEAM_INDEX_NEUTRAL
#define TEAM_INDEX_NEUTRAL 2
#endif

// ---- BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED ----
#ifndef BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED
#define BG_AV_NODE_STATUS_NEUTRAL_OCCUPIED NEUTRAL_CONTROLLED
#endif

// ---- BG_AV_NODE_CAPTAIN_DEAD_* ----
#ifndef BG_AV_NODE_CAPTAIN_DEAD_A
#define BG_AV_NODE_CAPTAIN_DEAD_A BG_AV_NodeEventCaptainDead_A
#endif
#ifndef BG_AV_NODE_CAPTAIN_DEAD_H
#define BG_AV_NODE_CAPTAIN_DEAD_H BG_AV_NodeEventCaptainDead_H
#endif

// ---- BG_AV_GameObjects_compat ----
enum BG_AV_GameObjects_compat
{
    BG_AV_GO_BANNER_ALLIANCE            = 179537,
    BG_AV_GO_BANNER_ALLIANCE_CONT       = 179536,
    BG_AV_GO_BANNER_HORDE               = 179535,
    BG_AV_GO_BANNER_HORDE_CONT          = 179534,
    BG_AV_GO_GY_BANNER_ALLIANCE         = 179838,
    BG_AV_GO_GY_BANNER_ALLIANCE_CONT    = 179837,
    BG_AV_GO_GY_BANNER_HORDE            = 179840,
    BG_AV_GO_GY_BANNER_HORDE_CONT       = 179839,
    BG_AV_GO_GY_BANNER_SNOWFALL         = 179841,
};

#include <random>

inline std::mt19937* GetRandomGenerator()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    return &rng;
}
