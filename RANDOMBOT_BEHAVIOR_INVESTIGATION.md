# RandomBot Autonomous Behavior Investigation

## 1. How Autonomous Bots Decide to Move, Travel, Fight, Quest, and Grind
Random bots use the Playerbot AI engine. Their behavior is determined by the **strategies** assigned to them and the **triggers** those strategies provide.

* **Movement & Travel:** The `travel` strategy (added if `AiPlayerbot.AutoDoQuests` is enabled) provides the trigger `val::not::travel target active`, which executes the `choose travel target` action. This action searches for a suitable location (for questing, grinding, or PVP) and sets it as the bot's destination.
* **Fighting:** Bots are assigned combat strategies based on their class and talents during initialization in `AiFactory::InitCombatStrategies`. For example, a Fury Warrior gets `fury`, `dps assist`, and `behind`.
* **Questing & Grinding:** The `AiFactory::InitNonCombatStrategies` automatically adds `grind`, `rpg`, and `travel` to free random bots (if `AutoDoQuests` is enabled).

## 2. Configuration Options Enabling Autonomous Behavior
The following options in `aiplayerbot.conf` control autonomous activities:

* **`AiPlayerbot.AutoDoQuests`**: This is the master switch for autonomous movement. If `1` (which is the default in `PlayerbotAIConfig.cpp`), `AiFactory` adds the `travel`, `tfish`, and `rpg` strategies to the non-combat engine.
* **`AiPlayerbot.RandomBotCombatStrategies`**: Defaults to `-threat,+custom::say`. This string modifies the base combat strategies (which already include class-specific DPS/Tank logic).
* **`AiPlayerbot.RandomBotNonCombatStrategies`**: Defaults to `+custom::say`. This string modifies the base non-combat strategies.
* **`AiPlayerbot.RandomBotRpgChance`**: Controls the chance to choose an RPG location (inn, town) versus a Grind/PvP location during a strategy change.

## 3. Are Random Bots Passive by Design?
**No, they are not passive by design, nor are they missing strategies.** They have the `travel`, `rpg`, and `grind` strategies active. 

**Root Cause of Passivity:**
They are failing a state transition check. Specifically, they are being blocked from moving because the engine believes they are **"waiting for mana."**

**Evidence from Code:**
1. The `choose travel target` action checks `ChooseTravelTargetAction::isUseful()`.
2. `isUseful()` immediately returns `false` if the bot `!AI_VALUE(bool, "can move around")`.
3. `can move around` (in `MaintenanceValues.h`) returns `false` if `!AI_VALUE(bool, "group ready")`.
4. `group ready` (in `GroupValues.cpp`) iterates over the bot's group (which includes just the bot itself if solo) and checks:
```cpp
    float mana = (static_cast<float> (member->GetPower(POWER_MANA)) / member->GetMaxPower(POWER_MANA)) * 100;
    if (mana < sPlayerbotAIConfig.mediumMana && !member->IsInCombat())
        return false;
```
When random bots are created via `rndbot init`, their maximum health and mana are set to their generated level (e.g., Level 60), but their *current* mana starts very low (the base mana of a Level 1, or 0 out-of-combat). 

Because their mana is below `AiPlayerbot.MediumMana` (default 40%), `group ready` evaluates to `false`. Because `group ready` is false, `can move around` is false, and the bot refuses to choose a travel target. They are stuck standing completely still waiting for their mana to passively regenerate before they will start moving, grinding, or questing.

*Note: Classes without mana (Warriors, Rogues) bypass this specific check because `GetPower(POWER_MANA) == 0` causes the loop to `continue`. However, if they have low health, they will also wait.*
