#pragma once

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
}

inline PlayerbotsCompatibility::LootMgrCompatibility sLootMgr;
inline PlayerbotsCompatibility::TaxiNodesStoreCompatibility sTaxiNodesStore;
