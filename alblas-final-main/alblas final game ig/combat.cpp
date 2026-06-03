#include "combat.h"
#include "utils.h"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

// ── Random Engine (new research topic #2) ─────────────────────────────────
// Using C++11 <random> library instead of basic rand().
// mt19937 is the Mersenne Twister engine — much better randomness.
// random_device seeds it so each run is different.
static std::mt19937 rng(std::random_device{}());

// Helper: get a random int in range [min, max] inclusive
static int randInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

// ── generateEnemy ──────────────────────────────────────────────────────────
// Creates an enemy scaled to the player's current depth and biome.
// Deeper = harder enemies with better drops.
Enemy generateEnemy(int depth, int biome) {
    // Enemy pools for each biome
    // Each entry: {name, maxHP, currentHP, attackPower, defense, xpReward, goldReward,
    // dropResource, dropAmount, biome, color}
    std::vector<Enemy> overworldEnemies = {
        {"Zombie",      30, 30, 8,  2, 20, 5,  "Iron",    1, "overworld", Color::GREEN},
        {"Skeleton",    25, 25, 10, 1, 25, 8,  "Bone",    2, "overworld", Color::WHITE},
        {"Spider",      20, 20, 12, 0, 22, 6,  "String",  2, "overworld", Color::BRIGHT_RED},
        {"Creeper",     35, 35, 18, 0, 40, 12, "Gunpowder",1, "overworld", Color::BRIGHT_GREEN},
        {"Witch",       45, 45, 14, 4, 50, 15, "Iron",    2, "overworld", Color::MAGENTA},
        {"Cave Spider", 22, 22, 14, 0, 30, 7,  "String",  3, "overworld", Color::BLUE},
    };

    std::vector<Enemy> netherEnemies = {
        {"Blaze",          50, 50, 20, 5, 60, 20, "Blaze Rod", 1, "nether", Color::BRIGHT_YELLOW},
        {"Piglin",         55, 55, 18, 8, 55, 25, "Gold",      2, "nether", Color::YELLOW},
        {"Wither Skeleton",70, 70, 22, 6, 80, 30, "Netherite", 1, "nether", Color::BRIGHT_WHITE},
        {"Ghast",          60, 60, 25, 3, 75, 22, "Gunpowder", 2, "nether", Color::WHITE},
        {"Magma Cube",     45, 45, 15, 10,50, 18, "Magma Cream",2, "nether", Color::BRIGHT_RED},
    };

    // Pick a random enemy from the correct biome pool
    Enemy e;
    if (biome == 0) {
        e = overworldEnemies[randInt(0, (int)overworldEnemies.size() - 1)];
    } else {
        e = netherEnemies[randInt(0, (int)netherEnemies.size() - 1)];
    }

    // Scale stats with depth using a loop to apply multipliers
    float scale = 1.0f + (depth * 0.15f);
    e.maxHP       = static_cast<int>(e.maxHP * scale);
    e.currentHP   = e.maxHP;
    e.attackPower = static_cast<int>(e.attackPower * scale);
    e.xpReward    = static_cast<int>(e.xpReward * scale);
    e.goldReward  = static_cast<int>(e.goldReward * scale);

    return e;
}

// ── printEnemyStats ────────────────────────────────────────────────────────
void printEnemyStats(const Enemy& enemy) {
    std::cout << enemy.color << Color::BOLD << "  " << enemy.name << Color::RESET
              << Color::BRIGHT_RED << "  HP: " << enemy.currentHP << "/" << enemy.maxHP
              << Color::BRIGHT_CYAN << "  ATK: " << enemy.attackPower
              << Color::BRIGHT_BLUE << "  DEF: " << enemy.defense
              << Color::RESET << "\n";
}

// ── runCombat ──────────────────────────────────────────────────────────────
// Main combat loop. Runs turn-by-turn until player or enemy reaches 0 HP.
// Enemy has simple AI: attacks normally, but uses a "power attack" at low HP.
void runCombat(Player& player, Enemy& enemy) {
    clearScreen();
    printBanner("⚔  COMBAT: " + enemy.name + " appears!", enemy.color);
    printEnemyStats(enemy);
    std::cout << "\n";
    pause(500);

    bool playerTurn = true; // Player always goes first

    // ── Combat Loop ─────────────────────────────────────────────────────
    while (enemy.currentHP > 0 && player.currentHP > 0) {

        if (playerTurn) {
            // ── Player's Turn ────────────────────────────────────────────
            std::cout << Color::BRIGHT_CYAN << "\n--- YOUR TURN ---\n" << Color::RESET;
            std::cout << Color::BRIGHT_RED << "Your HP: " << player.currentHP << "/" << player.maxHP
                      << "  |  " << enemy.color << enemy.name
                      << Color::BRIGHT_RED << " HP: " << enemy.currentHP << "/" << enemy.maxHP
                      << Color::RESET << "\n\n";

            std::cout << "  1) Attack\n";
            std::cout << "  2) Use Food (Heal " << 20 + player.level * 2 << " HP)\n";

            // Mage gets a special spell option
            if (player.pClass == PlayerClass::MAGE) {
                std::cout << "  3) Cast Fireball (1.8x damage, 20% miss chance)\n";
            } else if (player.pClass == PlayerClass::RANGER) {
                std::cout << "  3) Aimed Shot (1.5x damage, guaranteed hit)\n";
            } else {
                std::cout << "  3) Power Strike (2x damage, 30% miss chance)\n";
            }

            int choice;
            std::cout << Color::BRIGHT_WHITE << "Choice: " << Color::RESET;
            std::cin >> choice;

            int damage = 0;
            bool missed = false;

            if (choice == 1) {
                // Normal attack: slight random variance
                damage = getPlayerAttack(player) + randInt(-3, 5);
                damage = std::max(1, damage - enemy.defense);
            } else if (choice == 2) {
                // Use food to heal
                if (player.resources["Food"] > 0) {
                    int healAmt = 20 + player.level * 2;
                    healPlayer(player, healAmt);
                    player.resources["Food"]--;
                    printColor("You eat food and recover " + std::to_string(healAmt) + " HP!", Color::BRIGHT_GREEN);
                    playerTurn = false;
                    pause(500);
                    continue;
                } else {
                    printColor("You have no food!", Color::BRIGHT_RED);
                    pause(400);
                    continue; // Don't skip turn
                }
            } else if (choice == 3) {
                // Class special ability
                if (player.pClass == PlayerClass::MAGE) {
                    int missRoll = randInt(1, 5);
                    if (missRoll == 1) {
                        missed = true;
                    } else {
                        damage = static_cast<int>(getPlayerAttack(player) * 1.8) + randInt(0, 8);
                        damage = std::max(1, damage - enemy.defense);
                    }
                } else if (player.pClass == PlayerClass::RANGER) {
                    damage = static_cast<int>(getPlayerAttack(player) * 1.5) + randInt(0, 5);
                    damage = std::max(1, damage - enemy.defense);
                } else {
                    int missRoll = randInt(1, 10);
                    if (missRoll <= 3) {
                        missed = true;
                    } else {
                        damage = static_cast<int>(getPlayerAttack(player) * 2.0) + randInt(0, 6);
                        damage = std::max(1, damage - enemy.defense);
                    }
                }
            } else {
                printColor("Invalid choice. You hesitate!", Color::YELLOW);
                playerTurn = false;
                pause(400);
                continue;
            }

            if (missed) {
                printColor("Your attack missed!", Color::YELLOW);
                pause(500);
            } else {
                enemy.currentHP -= damage;
                std::cout << Color::BRIGHT_RED << "You deal " << Color::BOLD << damage
                          << Color::RESET << Color::BRIGHT_RED << " damage to "
                          << enemy.name << "!\n" << Color::RESET;
                pause(400);
            }

            playerTurn = false;

        } else {
            // ── Enemy's Turn (simple AI) ─────────────────────────────────
            std::cout << Color::BRIGHT_RED << "\n--- " << enemy.name << "'s TURN ---\n" << Color::RESET;
            pause(600);

            int enemyDmg = 0;
            std::string attackName = "attacks";

            // Enemy AI: power attack when below 30% HP
            float hpPercent = (float)enemy.currentHP / enemy.maxHP;
            if (hpPercent < 0.3f) {
                // Desperate power attack
                int missRoll = randInt(1, 4);
                if (missRoll == 1) {
                    printColor(enemy.name + " tries a desperate attack but misses!", Color::YELLOW);
                    playerTurn = true;
                    pause(400);
                    continue;
                }
                enemyDmg = static_cast<int>(enemy.attackPower * 1.6) + randInt(0, 5);
                attackName = "desperately attacks";
            } else {
                // Normal attack with small variance
                enemyDmg = enemy.attackPower + randInt(-2, 4);
            }

            // Apply player defense
            enemyDmg = std::max(1, enemyDmg - getPlayerDefense(player));
            player.currentHP -= enemyDmg;

            std::cout << enemy.color << enemy.name << " " << attackName << " you for "
                      << Color::BOLD << enemyDmg << Color::RESET
                      << Color::BRIGHT_RED << " damage!\n" << Color::RESET;
            pause(400);

            playerTurn = true;
        }
    }

    // ── Combat Result ────────────────────────────────────────────────────
    if (player.currentHP <= 0) {
        player.currentHP = 1; // Forgiving death: survive on 1 HP
        printBanner("You were defeated!", Color::BRIGHT_RED);
        printColor("You barely escape with your life... (respawned at 1 HP)", Color::YELLOW);

        // Lose some resources as penalty (forgiving — not all)
        int goldLost = player.gold / 4;
        player.gold -= goldLost;
        printColor("You lost " + std::to_string(goldLost) + " gold in the chaos.", Color::YELLOW);
        pause(1000);

    } else {
        printBanner("Victory! " + enemy.name + " defeated!", Color::BRIGHT_GREEN);

        // Award XP and gold
        gainXP(player, enemy.xpReward);
        player.gold += enemy.goldReward;

        std::cout << Color::BRIGHT_YELLOW << "  +" << enemy.xpReward << " XP  |  "
                  << "+" << enemy.goldReward << " Gold\n" << Color::RESET;

        // Drop loot using <random> for drop chance
        int dropRoll = randInt(1, 100);
        if (dropRoll <= 70) { // 70% drop chance
            addResource(player, enemy.dropResource, enemy.dropAmount);
            printColor("  Dropped: " + enemy.dropResource + " x" + std::to_string(enemy.dropAmount), Color::BRIGHT_CYAN);
        }

        // Bonus food drop chance
        if (randInt(1, 100) <= 40) {
            player.resources["Food"] += 1;
            printColor("  Dropped: Food x1", Color::GREEN);
        }

        pause(1000);
    }
}