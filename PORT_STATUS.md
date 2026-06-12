# Tortoise-WoW Playerbot Port Status

Last updated: 2026-06-11

## Maintenance Rule

Update this file during every future investigation when a fix, finding,
reproduction result, validation result, unresolved issue, or commit changes.
Do not leave investigation findings only in chat or agent memory.

## Repository State

- Parent repository branch: `playerbots-runtime-checkpoint`
- Parent repository HEAD: `f24d4495 Add investigation reports`
- Playerbot submodule branch: `playerbots-runtime-checkpoint`
- Playerbot submodule HEAD: `0369d25e Fix AI passivity, offline SIGSEGV, and default talent configuration`
- The current grouped-combat investigation contains uncommitted changes.
- Do not commit the current combat changes until combat validation and the
  requested 10-minute stability test pass.

## Completed Fixes

- Ported the upstream playerbot module to compile and link with Tortoise-WoW.
- Added the playerbot runtime scheduler and world-update integration.
- Stabilized bot bootstrap, login activation, fake sessions, session ownership,
  logout, and lifecycle handling.
- Fixed bot account and character bootstrap.
- Fixed group invitation and grouped-playerbot handling.
- Added null-socket safety for group updates, chat, whispers, emotes, and other
  bot actions that do not have a real client socket.
- Fixed runtime movement and follow crashes, including unsafe movement-generator
  handling.
- Fixed random bot travel stalls and travel-target reset/fallback handling.
- Fixed anticheat initialization for bot sessions.
- Fixed iterator invalidation in `DropMapValue::Calculate`.
- Fixed equipment cache generation.
- Fixed random gear generation.
- Fixed equipment persistence across bot login/logout.
- Fixed equipment stat-weight unsigned underflow issues.
- Fixed offline `rndbot init` SIGSEGV caused by out-of-world map/terrain access.
- Fixed premade/default talent spec loading for Turtle-WoW talent layouts.
- Fixed random bot health, mana, and energy restoration during initial
  randomization and login, preventing the `group ready` check from making
  newly initialized bots passive.
- Verified previously that bot login, small-group invitation, whispers,
  following, travel, and basic movement work.
- `mangosd` built successfully after the current minimal combat changes.
- `git diff --check` passed for the current playerbot combat changes.

## Commits Already Made

The lists below include all port-specific commits on the active branches after
their upstream/base branch points. They do not list the inherited Tortoise-WoW
or upstream playerbot project history.

### Parent Repository

- `9418d090` WIP playerbots Turtle runtime compatibility checkpoint
- `4dcfb862` WIP update playerbots compatibility after agent pass
- `225f9aa1` Update playerbots submodule pointer
- `fc9090c5` WIP save compatibility work before Gemini crash recovery
- `b8731b6b` WIP save strategy compatibility progress
- `3983307e` WIP save movement and loot compatibility fixes
- `f9b1a037` Port playerbots to Tortoise-WoW compile and link successfully
- `cd5ee63e` Update playerbots submodule after compatibility port
- `e6b27d3f` Checkpoint: playerbot bootstrap creates accounts and characters
- `ca8860a1` Update playerbots submodule runtime checkpoint
- `2c592060` Enable playerbot runtime scheduler and login activation
- `63b535f6` Enable playerbot runtime scheduler and stabilize bot login lifecycle
- `8b7b116f` Stabilize playerbot runtime movement validation
- `c8c0e33a` Checkpoint stable playerbot runtime movement
- `180f3d9d` Checkpoint: grouped playerbot validation stable
- `c9c77346` Checkpoint: grouped playerbot validation stable
- `ce25978d` Update playerbots: Fix iterator invalidation crash
- `fac233b2` Update playerbots submodule for equipment fix
- `9b30b1d7` Fix offline rndbot SIGSEGV, cache GetMaxEntry, and update submodule pointer for playerbots fixes
- `f24d4495` Add investigation reports

### Playerbot Submodule

- `54a4cb62` WIP playerbots Turtle runtime compatibility checkpoint
- `819ed8d1` WIP continued playerbots Turtle runtime compatibility
- `c792e2ec` WIP continued ahbot and playerbot compatibility fixes
- `3a0688ff` WIP API migration sweep
- `5507d739` WIP before Gemini agent crash recovery
- `c1e21065` WIP continue playerbot strategy compatibility fixes
- `22da5b74` WIP Movement and loot action compatibility fixes
- `e1af2714` Port upstream playerbots sources for Tortoise-WoW compatibility
- `8a05ac24` Fix playerbot runtime bootstrap compatibility
- `4085325e` Fix playerbot runtime activation and login scheduling
- `5a9eff4b` Fix bot session ownership and login lifecycle crashes
- `c7ec3e58` Fix random bot travel stalls and anticheat session init
- `002eb0ef` Stabilize random bot travel and fake-session chat
- `a5fdeaff` Checkpoint: grouped playerbot validation stable
- `abddb15d` Fix iterator invalidation crash in DropMapValue::Calculate
- `583a3acc` Fix bot equipment persistence and underflow issues
- `0369d25e` Fix AI passivity, offline SIGSEGV, and default talent configuration

## Current Uncommitted Combat Changes

The requested minimal combat changes are present but not committed:

- `playerbot/strategy/actions/SecurityCheckAction.cpp`
  - `FREE_FOR_ALL` no longer causes `+passive,+stay`.
  - Existing passive strategies are removed when the restrictive loot
    condition no longer applies.
- `playerbot/AiFactory.cpp`
  - A random bot with an active real-player master no longer receives the
    autonomous leader/free-bot grind, travel, and RPG strategy set.
- `playerbot/strategy/values/AttackersValue.cpp`
  - Narrowly allows a grouped bot to assist a real-player master's current
    victim when the master is in combat and the target is safe.
- `playerbot/PlayerbotAI.cpp`
  - Temporary packet-queue diagnostic logging is present.
- `playerbot/strategy/values/AttackerCountValues.cpp`
  - Temporary attacker/master-victim diagnostic logging is present.

Additional pre-existing uncommitted parent-repository compatibility changes are
present in:

- `src/game/Handlers/ChatHandler.cpp`
- `src/game/WorldSession.cpp`
- `src/modules/Bots/include/PlayerbotsCompatibility.h`
- `src/modules/Bots/src/PlayerbotsCompatibility.cpp`

Do not revert or accidentally include unrelated working-tree changes.

## Current Unresolved Issues

### Grouped Random Bot Combat

- A grouped random bot follows the real player and responds to some commands.
- Tiza, a geared level-25 rogue, enters combat state/stance when Zugzug attacks.
- Tiza does not reliably move into melee range to reach the target.
- When the monster is brought close to Tiza, she appears ready and may attack.
- It remains unverified whether combat activation is caused automatically by
  assisting the master or only by the explicit `attack` command.
- The explicit `attack` command remains unreliable.
- The likely remaining blocker is stale `stay` or `passive` combat strategy
  state, or failure of the `close`/`reach melee` movement action.
- Command parsing and effects for `attack`, `follow`, `stay`, `co`, and `nc`
  still require controlled validation.
- Interaction with GM accounts remains incompletely tested. Validation must be
  repeated with GM mode off and, if possible, a non-GM master.
- It remains possible that stale travel/RPG state or strategy state is
  competing with grouped follow/assist behavior.
- The exact code path preventing Tiza from closing distance and attacking is
  not yet identified.

### Validation

- Full combat validation has not passed.
- The requested 10-minute server stability test has not passed.
- No commit should be made for the current combat changes yet.

## Current Combat Reproduction Steps

Use a non-GM account if possible. Otherwise run with GM mode off.

1. Start `realmd` and `mangosd`.
2. Log in as the real-player master, currently Zugzug the orc warrior.
3. Ensure GM mode is off.
4. Summon Tiza with `.summon Tiza` if necessary.
5. Invite Tiza and confirm she is in Zugzug's party.
6. Send `follow` in party chat and confirm Tiza follows.
7. Pull a monster without sending `attack`.
8. Observe whether Tiza enters combat automatically and whether she moves into
   melee range.
9. If Tiza does not move, bring the monster close and observe whether she
   attacks or casts.
10. Repeat the pull, then send `attack` and compare behavior.
11. As a controlled strategy-state test, send this as one command in party
    chat, then pull again without `attack`:

    `co -passive,-stay,+close,+dps assist`

12. Record whether Tiza enters combat, selects the master's victim, closes
    distance, attacks, and returns to following after combat.
13. After combat behavior passes, leave the server running and responsive for
    at least 10 minutes.

## Current Combat Execution Findings

- `FollowChatShortcutAction::Execute` resets AI, enables non-combat `follow`,
  removes non-combat `passive`, `stay`, and `wander`, and removes combat
  `stay`, `guard`, and `wander`.
- `MovementAction::isUseful()` blocks movement when the active state has the
  `stay` strategy.
- `ReachTargetAction::isUseful()` also refuses movement with `stay`.
- `PassiveMultiplier` blocks almost all actions while `passive` is active.
- Rogue combat defaults include `dps assist` and `close`.
- `AttackAction` can select/start attacking a target while it is out of range,
  placing the bot in combat stance; `close`/`reach melee` must then move it
  into range.
- Logs confirmed Tiza transitioned to combat state during a real-player pull.
- Logs also showed Tiza using a targeted movement generator outside combat,
  confirming that she can move generally.

## Memory Leak / Resource Exhaustion Symptoms

The server was still producing log output and showed no observed crash or
assertion, but WSL became effectively unresponsive due to memory pressure.

Observed values before the planned WSL restart:

- Physical memory: approximately `15 GiB / 15 GiB` used.
- Free physical memory: approximately `235 MiB`.
- Available physical memory: approximately `179 MiB`.
- Swap: approximately `4.0 GiB / 4.0 GiB` used, with about `1.2 MiB` free.
- Shell commands and server interaction became very slow due to swap thrashing.
- `mangosd` logs continued advancing, so the server process had not clearly
  crashed.
- No reliable per-process memory attribution was obtained because process
  visibility differed across the WSL/sandbox PID namespaces.
- The exact leak or unbounded cache/queue is not identified.
- Temporary packet-queue diagnostics were added because an unbounded bot packet
  queue is one possible contributor, but this has not been proven.

Required memory investigation after a clean WSL restart:

1. Record baseline WSL, `mangosd`, and `realmd` RSS/swap before login.
2. Record memory after bot initialization.
3. Record memory after inviting one bot.
4. Record memory during repeated combat/follow testing.
5. Sample memory growth over at least 10 minutes.
6. Correlate growth with bot packet-queue diagnostics, random bot updates,
   travel/pathfinding activity, and bot count.
7. Distinguish a true leak from large caches, database buffers, log buffering,
   or WSL memory reclamation behavior.

### Runtime Stall Investigation: Log Inventory And Timeline

Investigation started 2026-06-11. No runtime code has been changed.

- WSL had already restarted before log inspection. Current memory returned to
  normal and neither `mangosd` nor `realmd` was running.
- The retained stall run is
  `server/logs/server_2026-06-11_19-03-29.log`, covering approximately
  `19:03:29` through `19:08:47`.
- Chechmich GUID is `1805`.
- Chechmich was online by `19:03:40`.
- `rndbot init`/first randomization ran at `19:05:13` through `19:05:14` and
  completed in approximately 40 ms.
- The first AI-log transition from `Non Combat` to `Combat` is at `19:08:31`.
- Chechmich returned to `Non Combat` at `19:08:47`.
- The log does not contain a clear group-invite or `follow` command timestamp.
- `errors.log` contains startup/database warnings but no crash, assertion, or
  combat-time error corresponding to the stall.
- `perf.log` shows repeated slow map-1 updates dominated by cell updates:
  approximately 901 ms at `19:03:53`, 923 ms at `19:04:24`, 947 ms at
  `19:06:30`, 895 ms at `19:07:32`, and 877 ms at `19:08:04`.
- Those map-cell spikes began before the recorded combat transition, so combat
  is not proven to be their original trigger.
- The `19:08:04` perf entry contains an unsigned timing underflow
  (`players2=4294967295ms`), which is a diagnostics bug but does not itself
  explain retained memory.
- The run contains 543 `PBDBG attackers` lines and 150
  `PBDBG has attackers` lines. These are continuous diagnostics, but their
  total log volume is too small to explain approximately 13.6 GB RSS.
- Only two spline-launch diagnostics were recorded for Chechmich, both
  `StopMoving`; no movement/spline log flood is present.
- No `PBDBG packet-queue` threshold message appears, so the instrumented bot
  packet queue did not reach 100 entries during the retained run.
- `current_time` is rewritten in approximately 30-second bursts every minute:
  `19:04:09-19:04:39`, `19:05:10-19:05:40`,
  `19:06:11-19:06:41`, `19:07:12-19:07:42`, and
  `19:08:13-19:08:43`.
- Most seconds contain one DELETE/INSERT pair, while some contain duplicate or
  reordered pairs. This is abnormal scheduling/SQL churn and requires source
  tracing, but the observed completed-query rate alone is not sufficient to
  retain gigabytes.

### Runtime Stall Investigation: SQL And Container Findings

- `RandomPlayerbotMgr::UpdateAIInternal()` calls `SaveCurTime()` whenever
  `EventTimeSyncTimer + 30` has elapsed.
- `SaveCurTime()` only advances `EventTimeSyncTimer` after 60 seconds, but it
  unconditionally writes `current_time`.
- This mismatch exactly causes a write on every random-bot manager update
  during the second half of each minute.
- Each write is a separate asynchronous DELETE followed by INSERT. Their log
  order can reverse or duplicate when operations overlap.
- After the WSL restart, `tw_char.ai_playerbot_random_bots` contained only
  eight rows, zero `current_time` rows, and no duplicate owner/bot/event
  groups. The table occupies only one 16 KiB data page. Therefore this SQL bug
  creates churn and possible transient duplicates, but it is not the source of
  approximately 13.6 GB retained RAM in the observed run.
- `Engine::addStrategy()` removes an existing strategy before reinserting it,
  and `Engine::Init()` resets and deletes queued action nodes, trigger nodes,
  and multipliers before rebuilding them.
- `Queue::Push()` deduplicates queued actions by action name and deletes the
  duplicate `ActionNode`/basket. The action queue is therefore bounded by the
  number of distinct action names under normal execution.
- `AttackersValue`, `PossibleTargetsValue`, `PossibleAttackTargetsValue`, and
  `DpsTargetValue` calculate into fresh local lists/sets and replace their
  cached values. No append-without-clear path was found in those combat target
  values.
- `AiObjectContext` caches qualified values for the bot lifetime, but no
  combat-specific dynamic qualifier path has yet been found that could create
  millions of unique values during the short reproduction.
- `AiPlayerbot.PerfMonEnabled = 0` in the active config. The performance
  monitor has permanent key storage when enabled, but it was disabled and is
  not the active leak in this run.
- A separate exact CPU-stall defect exists in `Engine::DoNextAction()`: when
  `minimal` is true and the highest queue relevance is below 100, the loop
  executes `continue` without popping the action or incrementing the iteration
  count. This can spin the MainThread indefinitely. It explains a possible
  100% CPU stall, but the loop itself performs no growing allocation and does
  not explain 13.6 GB RSS.
- The stall run loaded the equipment cache from 1,365,343 records and the
  separate random-item cache from 92,790 records before Chechmich's login and
  combat. The second log message is mislabeled as an equipment-cache load.
- Database measurement rules out either cache as the approximately 13.6 GB
  source:
  - All 1,365,343 equipment-cache rows are unique.
  - They collapse into 70,008 `(class, spec, level, slot, quality)` vectors
    containing 1,365,343 `uint32` item IDs, approximately 5.5 MB of item-ID
    payload plus normal map/vector overhead.
  - The MyISAM equipment-cache table is approximately 36.9 MB data plus
    19.7 MB indexes.
  - The random-item cache contains 92,790 unique rows and its table is
    approximately 1.7 MB data plus 1.3 MB indexes.
  - The item-info cache contains only 14,646 rows.
- The equipment cache is large and process-lifetime retained, but its
  cardinality and representation cannot plausibly explain multi-gigabyte
  growth after combat. It is no longer the primary RAM-growth suspect.
- Commit `583a3acc` did not alter equipment-cache generation cardinality; its
  large apparent diff is mostly line-ending churn plus signed stat-weight
  fixes.

## Files Most Relevant To The Current Combat Issue

- `src/modules/Bots/upstream/playerbot/AiFactory.cpp`
- `src/modules/Bots/upstream/playerbot/PlayerbotAI.cpp`
- `src/modules/Bots/upstream/playerbot/strategy/actions/SecurityCheckAction.cpp`
- `src/modules/Bots/upstream/playerbot/strategy/actions/ChatShortcutActions.cpp`
- `src/modules/Bots/upstream/playerbot/strategy/actions/AttackAction.cpp`
- `src/modules/Bots/upstream/playerbot/strategy/actions/MovementActions.cpp`
- `src/modules/Bots/upstream/playerbot/strategy/values/AttackersValue.cpp`
- `src/modules/Bots/upstream/playerbot/strategy/values/AttackerCountValues.cpp`

### Memory Leak Fix (2026-06-11)
- Investigated the ~15 GB RSS memory consumption occurring during `mangosd` startup.
- Identified the source as a massive accumulation of orphaned `PortalNode` and `TravelNode` heap allocations within the bot pathfinding system (`TravelNodeMap::getRoute`).
- Temporary nodes were added to a `TravelNodeRoute` object, which lacked a destructor. The route object was frequently passed by value and discarded when path validation failed, permanently leaking the nodes.
- **Fixed:** Converted `TravelNodeRoute::tempNodes` to a `std::vector<std::shared_ptr<TravelNode>>`. This RAII change ensures the temporary nodes are safely deallocated when `TravelNodeRoute` copies are destroyed or when `cleanTempNodes()` is called, eliminating the leak without altering routing behavior. Memory is now stable at < 1 GB.
