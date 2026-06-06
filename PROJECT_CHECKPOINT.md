# Project Checkpoint

Date: 2026-06-06

## Branch Snapshot

- `tortoise-wow`: `playerbots-runtime-checkpoint`
- `src/modules/Bots/upstream`: `playerbots-runtime-checkpoint`

## Latest Commits

- `tortoise-wow`: `c8c0e33a` - `Checkpoint stable playerbot runtime movement`
- `src/modules/Bots/upstream`: `002eb0ef` - `Stabilize random bot travel and fake-session chat`

## Playerbot Fixes Applied So Far

### Runtime and bootstrap

| File | Function / Area | Reason | Validation result |
| --- | --- | --- | --- |
| `src/game/World.cpp` | `PlayerbotsUpdateAI(diff)` world hook | Ensure playerbot AI is ticked from the world update loop. | Verified in long runtime soaks; bots continue ticking and logging `PBDBG` state. |
| `src/modules/Bots/upstream/playerbot/RandomPlayerbotMgr.cpp` | `UpdateAIInternal(...)` / round-robin bot ticking | Limit how much bot AI work runs per world update and keep the scheduler bounded. | Verified with `AiPlayerbot.BotAITicksPerWorldUpdate = 1`; server stayed responsive in soak tests. |
| `src/modules/Bots/upstream/playerbot/PlayerbotAIConfig.cpp` | `botAITicksPerWorldUpdate` config load | Make the per-world AI tick budget configurable. | Confirmed active in current config. |
| `src/game/PlayerBots/PlayerBotAI.cpp` | `sess->SetPlayer(newChar)` / social setup | Bind the fake session to the spawned bot player correctly. | Bots log in cleanly and remain usable after restart. |
| `src/game/PlayerBots/PlayerBotMgr.cpp` | `session->InitAntiCheatSession(&dummyKey)` | Avoid null/invalid anti-cheat session state for fake bot sessions. | Confirmed present; no login-time crash in current runs. |
| `src/game/PlayerBots/PlayerBotAI.cpp` | `mPlayer->SetSocial(sSocialMgr.LoadFromDB(...))` cleanup order | Prevent stale social state and null social access during bot load. | Verified indirectly by stable login and invite flow. |

### Logging, travel, and state cleanup

| File | Function / Area | Reason | Validation result |
| --- | --- | --- | --- |
| `src/modules/Bots/upstream/playerbot/RandomPlayerbotMgr.cpp` | `PBDBG rndprocess*` logging | Remove/throttle oversized strategy dumps that could stall inside `sLog.outString`. | Validated by 5+ minute soaks with no logging stall. |
| `src/modules/Bots/upstream/playerbot/TravelMgr.cpp` | `SetNullTravelTarget(...)` | Clean up invalid/expired travel targets instead of leaving stale state behind. | Confirmed by continued travel updates without stuck null-target loops. |
| `src/modules/Bots/upstream/playerbot/strategy/actions/ChooseTravelTargetAction.cpp` | travel target selection | Guard trainer-class travel stalls, speculative RPG scoring, and zero-destination tele-cache fallback. | Verified by sustained travel selection logs and no travel-target crash during soak. |
| `src/modules/Bots/upstream/playerbot/strategy/values/TravelValues.cpp` | travel-target validity/working values | Keep invalid travel targets from being treated as active/working. | Verified indirectly by stable travel state transitions. |

### Social, chat, emote, and group safety

| File | Function / Area | Reason | Validation result |
| --- | --- | --- | --- |
| `src/modules/Bots/upstream/playerbot/PlayerbotAI.cpp` | `CanBotUseTurtleChannel(...)` | Fake bots and low-level bots must not route through Turtle public-channel permission checks. | No public-channel crash observed in current runs; public-channel path is disabled for fake/socketless bots. |
| `src/modules/Bots/upstream/playerbot/BroadcastHelper.cpp` | `BroadcastToChannelWithGlobalChance(...)` | Stop spontaneous bot suggestion broadcasts into Turtle chat channels from fake sessions. | No public-channel stall observed in the current validation set. |
| `src/modules/Bots/upstream/playerbot/strategy/actions/SayAction.cpp` | `isUseful()` | Disable spontaneous `custom::say` for socketless bots so fake sessions do not enter player-only chat paths. | Confirmed by stable soak; no bot speech-triggered stall after the fix. |
| `src/game/Handlers/ChatHandler.cpp` | `WorldSession::HandleTextEmoteOpcode(...)` | Fake/socketless bot sessions must not execute the full client text-emote path, which includes real chat/mute/map visitation logic. | No emote-triggered stall in the current 5+ minute grouped soak. |
| `src/game/Handlers/GroupHandler.cpp` | `WorldSession::HandleGroupInviteOpcode(...)` | Avoid `GetSocial()` on fake bots and avoid re-entering the fake session accept handler. | Validated: `/invite Dariston` no longer disconnects immediately; bot joins the group successfully. |
| `src/game/Group/Group.cpp` | `Group::SendUpdate()` | Do not send client-only group packets to socketless fake bot sessions. | Validated: group join succeeds without immediate disconnect. |
| `src/modules/Bots/upstream/playerbot/ServerFacade.cpp` | `ServerFacade::GetChaseTarget(...)` | Prevent invalid casts when the current movement generator is not chase/follow; this was the confirmed post-invite crash. | Validated: grouped server stayed alive past the prior 4-second crash point and through a 5+ minute soak. |

## Remaining Known Issues

- Dariston still travels independently and may be far from the player; the bot was in Badlands while the master was elsewhere, so local follow movement was not visually exercised.
- The bot remains on its own travel cadence, so a short-range follow test has not yet been run in the same area as the player.
- Travel logging is still noisy, but it is no longer stalling the server in the current validation path.
- I have not re-run the old greet/emote stall as a standalone isolated test after the follow-trigger fix; the current evidence is absence of regression during the grouped soak.

## Current Stable Validation Status

- Build completed successfully after the latest code changes.
- One-bot standalone soak passed for more than 5 minutes.
- `/invite Dariston` no longer disconnects the client immediately.
- Dariston joined the group successfully and both client and bot remained online.
- The server stayed responsive for more than 5 minutes after the invite, including past the previous crash window.
- The last confirmed fatal issue was the invalid follow-trigger cast; that is now fixed and no longer reproduces in the current soak.

## Recommended Next Debugging Steps

1. Run a co-located follow test so the bot is near the player and actual follow movement can be observed without long-distance travel noise.
2. If a disconnect returns, capture whether the last visible event is a chat/emote packet, a group update, or a movement-generator transition.
3. If follow still misbehaves but does not crash, inspect the follow strategy and chase/follow movement generators next, not travel selection.
4. If chat/emote issues reappear, isolate `TellPlayer`/`Whisper` from the Turtle-specific public-channel gating and fake-session handling.

## Rebuild And Reproduce

```bash
cmake --build build --target mangosd -j2
./build/src/mangosd/mangosd -c server/etc/mangosd.conf
```

Current playerbot config requirements in `server/etc/aiplayerbot.conf`:

```ini
AiPlayerbot.MinRandomBots = 1
AiPlayerbot.MaxRandomBots = 1
AiPlayerbot.BotAITicksPerWorldUpdate = 1
```

To reproduce the validated path:

1. Start `mangosd` with the command above.
2. Log in with the real client.
3. Wait until Dariston is online.
4. Run `/invite Dariston`.
5. Stay logged in and move normally for at least 5 minutes.

