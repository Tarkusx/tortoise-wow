# Project Checkpoint: Playerbot Stabilization & Scale Testing

## Architecture Summary
The server runs on the MaNGOS (tortoise-wow) core with the `playerbots` module integrated via submodule. The core handles networking, world state, and entities (players, creatures). The `playerbots` module injects AI controllers (`PlayerbotAI`) into `Player` objects. Bots operate as standard `Player` objects without a real physical client connection (no socket). The server update loop ticks the bots via the bot scheduler, simulating player inputs (movement, spellcasting, chat, grouping).

## Playerbot Runtime Flow
1. **Initialization**: Server startup loads the playerbots module.
2. **Login/Activation**: Bots are spawned and added to the world during the update loop. They bypass standard auth and login packets.
3. **Update Loop**: The `sPlayerbotAIConfig` scheduler periodically ticks bot AI.
4. **AI Decision Making**: Bots evaluate their environment using strategies and triggers (e.g., travel, follow, combat, group).
5. **Actions**: Bots execute actions (spells, movement, emotes, group invites) by directly invoking server-side handlers or simulating opcodes.
6. **Movement**: Bots use movement generators (e.g., `ChaseMovementGenerator`, `WaypointMovementGenerator`) to navigate the world.

## Fixes Applied
1. **Runtime Scheduler & Login**: Enabled the runtime scheduler and stabilized the bot login lifecycle, allowing bots to spawn and tick.
2. **Movement & Follow**: Fixed an unsafe casting of idle movement to `ChaseMovementGenerator` that caused crashes when bots were following a leader.
3. **Group Handling**: Fixed crashes in `Group::SendUpdate` by verifying the socket connection. Altered group invite handling (`GroupHandler.cpp`) so bots can directly join groups without needing fake client response packets.
4. **Chat & Emotes**: Added null socket safety checks to `ChatHandler.cpp` and `SayAction.cpp` so bots can use text emotes and chat without crashing the server.
5. **Travel Target Safety**: Added fallback logic in `TravelMgr` (`FindFallbackTeleCachePosition`) to prevent stalls and crashes when bots fail to find valid travel destinations. Added proper clearing of travel values on target reset.
6. **Talent System Spec Support**: Upstream playerbots shipped with Vanilla specs that failed to apply against the Turtle WoW talent tree layout. Replaced legacy config specs with TWoW-compliant specs (`simple_warrior`, `simple_paladin`, etc.) in `aiplayerbot.conf.dist.in`.
7. **Equipment Underflow Crash (`statWeight`)**: Fixed a severe underflow bug in `RandomItemMgr` where low-level rings/trinkets with stat weights < 1.0 were being cast to an unsigned integer, resulting in a large integer loop underflow during equipment calculation.
8. **Offline Rndbot Init SIGSEGV**: Fixed a null pointer dereference in `MapNodes::MasterPlayer::GetTerrainHeight()` caused by bots attempting to initialize equipment/stats out-of-world during startup. Patched `Item::CreateItem` parameter logic to safely skip line-of-sight/Z-axis map checks for bots lacking a valid `map_id`.
9. **Random Bot Resource Restoration (Passivity Fix)**: Fixed an issue where randomly spawned bots would enter the world with low or 0 health/mana/energy. This immediately failed the internal `group ready` check, causing them to sit passively instead of triggering travel/quest logic. Bots are now properly restored to 100% resources during `RandomizeFirst` and `OnBotLoginInternal`.

## Current Known-Good Configuration
- Playerbots login successfully.
- Runtime scheduler is active.
- Travel and movement are working.
- Bots can level and progress.
- Group invite works.
- Bot whisper works.
- Follow behavior works.
- Tested stable with small groups for 5+ minutes.
- Both player and bot remain online and responsive.

## Reproduction & Build Instructions
1. Ensure both the main `tortoise-wow` repo and `playerbots` submodule are on the correct commits.
2. Compile the server using the standard CMake build process:
   ```bash
   mkdir build && cd build
   cmake ../
   make -j$(nproc)
   ```
3. Run `mangosd` and ensure the bot configuration is loaded.

## Validation History
- **Movement/Follow Validation**: 5-minute grouped validation passed. Bots successfully followed the leader without crashing the server.
- **Login Validation**: Bots successfully spawn and populate the world on server start.
- **Group Invite**: Verified that inviting a bot correctly forms a party without stalling or disconnecting.
- **Emotes/Chat**: Verified that bot communications do not crash the handler due to missing sockets.

## Remaining Known Issues
- System needs scale testing (up to 25 bots) to identify potential CPU, memory, or world-thread bottlenecks.

## Potential Risks
- **World Update Stalls**: As bot count increases, AI calculations (specifically pathfinding, travel logic, and target selection) may starve the main thread.
- **Memory Growth**: Long-running bots with complex state machines might leak memory or cache excessively.
- **Pathfinding Overload**: Concurrent grid loading and path generation for many bots across different maps.
