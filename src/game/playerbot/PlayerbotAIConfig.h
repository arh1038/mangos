#pragma once

#include "../../shared/Config/Config.h"

class Player;
class PlayerbotMgr;
class ChatHandler;

class PlayerbotAIConfig
{
public:
    PlayerbotAIConfig();

public:
    bool Initialize();
    bool IsInRandomAccountList(uint32 id);

    bool enabled;
    bool allowGuildBots;
    uint32 globalCoolDown, reactDelay, teleportDelay;
    float sightDistance, spellDistance, reactDistance, grindDistance, lootDistance, fleeDistance, tooCloseDistance, meleeDistance, followDistance;
    std::list<uint32> randomBotAccounts;
    uint32 criticalHealth, lowHealth, mediumHealth, almostFullHealth;
    uint32 lowMana, mediumMana;
    uint32 pvpChance;
    uint32 randomGearQuality;
    uint32 iterationsPerTick;

    std::string GetValue(std::string name);
    void SetValue(std::string name, std::string value);

private:
    Config config;
};

#define sPlayerbotAIConfig MaNGOS::Singleton<PlayerbotAIConfig>::Instance()