#include "PlayerbotsCompatibility.h"

#include "Entities/Player.h"
#include "LootMgr.h"
#include "Objects/Corpse.h"
#include "Objects/Item.h"
#include "playerbot/PlayerbotAI.h"
#include "World/World.h"

#include <unordered_map>

namespace
{
    // Temporary compile scaffolding for upstream playerbots.
    // Turtle's existing PlayerBotAI system remains separate; do not treat this as final runtime ownership.
    std::unordered_map<Player const*, PlayerbotAI*> sPlayerbotAIRegistry;
}

PlayerbotAI* Player::GetPlayerbotAI() const
{
    auto itr = sPlayerbotAIRegistry.find(this);
    return itr != sPlayerbotAIRegistry.end() ? itr->second : nullptr;
}

void Player::CreatePlayerbotAI()
{
    if (GetPlayerbotAI())
        return;

    sPlayerbotAIRegistry[this] = new PlayerbotAI(this);
}

void Player::RemovePlayerbotAI()
{
    auto itr = sPlayerbotAIRegistry.find(this);
    if (itr == sPlayerbotAIRegistry.end())
        return;

    delete itr->second;
    sPlayerbotAIRegistry.erase(itr);
}

Loot* PlayerbotsCompatibility::LootMgrCompatibility::GetLoot(Player* player, ObjectGuid guid) const
{
    if (!player || !guid || !player->IsInWorld())
        return nullptr;

    Map* map = player->GetMap();
    if (!map)
        return nullptr;

    switch (guid.GetHigh())
    {
        case HIGHGUID_UNIT:
            if (Creature* creature = map->GetCreature(guid))
                return &creature->loot;
            return nullptr;
        case HIGHGUID_GAMEOBJECT:
            if (GameObject* go = map->GetGameObject(guid))
                return &go->loot;
            return nullptr;
        case HIGHGUID_CORPSE:
            if (Corpse* corpse = map->GetCorpse(guid))
                return &corpse->loot;
            return nullptr;
        case HIGHGUID_ITEM:
            if (Item* item = player->GetItemByGuid(guid))
                return &item->loot;
            return nullptr;
        default:
            return nullptr;
    }
}

Loot* PlayerbotsCompatibility::LootMgrCompatibility::GetLoot(Player* player) const
{
    return player ? GetLoot(player, player->GetLootGuid()) : nullptr;
}

PlayerbotHolder* PlayerbotsCompatibility::GetPlayerbotMgr(Player* player)
{
    if (!player)
        return nullptr;

    PlayerbotAI* ai = player->GetPlayerbotAI();
    return ai ? ai->GetHolder() : nullptr;
}

void PlayerbotsCompatibility::LearnClassLevelSpells(Player* player, bool includeTrainerSpells)
{
    if (!player)
        return;

    player->LearnDefaultSpells();

    if (!includeTrainerSpells)
        return;

    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!co)
            continue;

        if (co->trainer_type != TRAINER_TYPE_CLASS &&
            co->trainer_type != TRAINER_TYPE_TRADESKILLS &&
            co->trainer_type != TRAINER_TYPE_PETS)
            continue;

        if (co->trainer_type == TRAINER_TYPE_PETS && player->GetClass() == CLASS_HUNTER)
            continue;

        if ((co->trainer_type == TRAINER_TYPE_CLASS || co->trainer_type == TRAINER_TYPE_PETS) && co->trainer_class != player->GetClass())
            continue;

        uint32 trainerId = co->trainer_id;
        if (!trainerId)
            trainerId = co->entry;

        TrainerSpellData const* trainerSpells = sObjectMgr.GetNpcTrainerTemplateSpells(trainerId);
        if (!trainerSpells)
            trainerSpells = sObjectMgr.GetNpcTrainerSpells(trainerId);

        if (!trainerSpells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainerSpells->spellList.begin(); itr != trainerSpells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;
            if (!tSpell)
                continue;

            if (player->GetTrainerSpellState(tSpell) != TRAINER_SPELL_GREEN)
                continue;

            if (uint32 learnedSpell = GetTrainerLearnedSpellId(tSpell))
                player->LearnSpell(learnedSpell, false);
            else
                player->LearnSpell(tSpell->spell, false);
        }
    }
}

uint32 PlayerbotsCompatibility::CalculateTalentPoints(Player const* player)
{
    uint32 talentPointsForLevel = player->GetLevel() < 10 ? 0 : player->GetLevel() - 9;
    talentPointsForLevel += player->GetBonusTalentCount();
    return uint32(talentPointsForLevel * sWorld.getConfig(CONFIG_FLOAT_RATE_TALENT));
}

WorldLocation PlayerbotsCompatibility::GetSpawnLocation(CreatureDataPair const* pair)
{
    return pair ? pair->second.position : WorldLocation();
}

WorldLocation PlayerbotsCompatibility::GetSpawnLocation(GameObjectDataPair const* pair)
{
    return pair ? pair->second.position : WorldLocation();
}
