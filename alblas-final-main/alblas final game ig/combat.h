#pragma once
#include "player.h"
#include <string>

// ── Enemy Struct ───────────────────────────────────────────────────────────
struct Enemy {
    std::string name;
    int maxHP;
    int currentHP;
    int attackPower;
    int defense;
    int xpReward;
    int goldReward;
    std::string dropResource;
    int dropAmount;
    std::string biome;    // "overworld" or "nether"
    std::string color;    // ANSI color for display
};

// ── Function Declarations ──────────────────────────────────────────────────
Enemy generateEnemy(int depth, int biome);
void runCombat(Player& player, Enemy& enemy);
void printEnemyStats(const Enemy& enemy);