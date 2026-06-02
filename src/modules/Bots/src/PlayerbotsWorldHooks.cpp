#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotMgr.h"

bool PlayerbotsLoadConfig()
{
    return sPlayerbotAIConfig.LoadConfig();
}

void PlayerbotsUpdateAI(uint32 diff)
{
    static bool bootstrapAttempted = false;

    if (!sPlayerbotAIConfig.enabled)
        return;

    if (!sPlayerbotAIConfig.IsRuntimeBootstrapped())
    {
        if (bootstrapAttempted)
            return;

        bootstrapAttempted = true;
        if (!sPlayerbotAIConfig.BootstrapRuntime())
            return;
    }

    sRandomPlayerbotMgr.UpdateAI(diff);
}
