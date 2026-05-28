#include "player.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

// ── createPlayer ───────────────────────────────────────────────────────────
// Builds a new Player struct with starting stats based on chosen class.
// Each class has different strengths — Warrior is tanky, Ranger is speedy,
// Mage hits hard but is fragile.
Player createPlayer(const std::string& name, PlayerClass pClass) {
    Player p;
    p.name        = name;
    p.pClass      = pClass;
    p.level       = 1;
    p.xp          = 0;
    p.xpToNextLevel = 100;
    p.currentBiome = 0;
    p.depth        = 0;
    p.gold         = 0;
    p.equippedWeapon = "";
    p.equippedArmor  = "";

    // Starting resources for all classes
    p.resources["Wood"]      = 10;
    p.resources["Stone"]     = 5;
    p.resources["Iron"]      = 0;
    p.resources["Diamond"]   = 0;
    p.resources["Netherite"] = 0;
    p.resources["Food"]      = 5;

    p.day  = 1;
    p.hour = 8;

    // Class-specific stats (conditionals used here)
    if (pClass == PlayerClass::WARRIOR) {
        p.className   = "Warrior";
        p.maxHP       = 120;
        p.currentHP   = 120;
        p.attackPower = 15;
        p.defense     = 10;
    } else if (pClass == PlayerClass::RANGER) {
        p.className   = "Ranger";
        p.maxHP       = 90;
        p.currentHP   = 90;
        p.attackPower = 12;
        p.defense     = 6;
    } else {
        p.className   = "Mage";
        p.maxHP       = 70;
        p.currentHP   = 70;
        p.attackPower = 22;
        p.defense     = 3;
    }

    return p;
}

// ── printPlayerStats ───────────────────────────────────────────────────────
// Displays the player's current stats in a colored panel.
// Takes a const reference so we don't accidentally modify the player.
void printPlayerStats(const Player& player) {
    std::string classColor;
    if (player.pClass == PlayerClass::WARRIOR)     classColor = Color::BRIGHT_RED;
    else if (player.pClass == PlayerClass::RANGER) classColor = Color::BRIGHT_GREEN;
    else                                           classColor = Color::BRIGHT_MAGENTA;

    printBanner(player.name + " the " + player.className, classColor);

    std::cout << Color::BRIGHT_YELLOW << "  Level : " << Color::WHITE << player.level
              << Color::BRIGHT_YELLOW << "  XP: " << Color::WHITE
              << player.xp << "/" << player.xpToNextLevel << "\n";

    std::cout << Color::BRIGHT_RED << "  HP    : " << Color::WHITE
              << player.currentHP << "/" << player.maxHP << "\n";

    std::cout << Color::BRIGHT_CYAN << "  ATK   : " << Color::WHITE
              << getPlayerAttack(player)
              << Color::BRIGHT_BLUE << "   DEF: " << Color::WHITE
              << getPlayerDefense(player) << "\n";

    std::cout << Color::BRIGHT_YELLOW << "  Gold  : " << Color::WHITE
              << player.gold << "\n";

    std::cout << Color::BRIGHT_YELLOW << "  Day   : " << Color::WHITE
              << player.day << "  " << Color::BRIGHT_YELLOW << "Time: "
              << Color::WHITE << (player.hour < 10 ? "0" : "") << player.hour << ":00\n";

    // Print biome
    std::string biome = (player.currentBiome == 0) ? "Overworld" : "Nether";
    std::string biomeColor = (player.currentBiome == 0) ? Color::BRIGHT_GREEN : Color::BRIGHT_RED;
    std::cout << Color::WHITE << "  Biome : " << biomeColor << biome
              << Color::WHITE << "  (Depth: " << player.depth << ")\n";

    // Equipped gear
    std::cout << Color::BRIGHT_CYAN << "  Weapon: " << Color::WHITE
              << (player.equippedWeapon.empty() ? "None" : player.equippedWeapon) << "\n";
    std::cout << Color::BRIGHT_BLUE << "  Armor : " << Color::WHITE
              << (player.equippedArmor.empty() ? "None" : player.equippedArmor) << "\n";

    std::cout << Color::RESET;
}

// ── printInventory ─────────────────────────────────────────────────────────
void printInventory(const Player& player) {
    printBanner("Inventory", Color::BRIGHT_CYAN);

    // Print resources using a loop
    std::cout << Color::BRIGHT_YELLOW << "  -- Resources --\n" << Color::RESET;
    for (const auto& [res, amt] : player.resources) {
        if (amt > 0) {
            std::cout << "    " << Color::CYAN << res << ": "
                      << Color::WHITE << amt << Color::RESET << "\n";
        }
    }

    // Print items
    if (!player.inventory.empty()) {
        std::cout << Color::BRIGHT_YELLOW << "  -- Items --\n" << Color::RESET;
        for (const auto& item : player.inventory) {
            std::cout << "    " << Color::GREEN << item.name
                      << Color::WHITE << " (x" << item.quantity << ")"
                      << Color::RESET << "\n";
        }
    }

    std::cout << Color::BRIGHT_YELLOW << "  Gold: " << Color::WHITE
              << player.gold << Color::RESET << "\n";
}

// ── addToInventory ─────────────────────────────────────────────────────────
// Adds an item to the player's inventory. If the item already exists,
// increases the quantity instead of adding a duplicate entry.
void addToInventory(Player& player, const Item& item) {
    // Loop through existing inventory to check for duplicates
    for (auto& existing : player.inventory) {
        if (existing.name == item.name) {
            existing.quantity += item.quantity;
            return;
        }
    }
    player.inventory.push_back(item);
}

// ── Resource helpers ───────────────────────────────────────────────────────
bool hasResource(const Player& player, const std::string& resource, int amount) {
    auto it = player.resources.find(resource);
    if (it == player.resources.end()) return false;
    return it->second >= amount;
}

void removeResource(Player& player, const std::string& resource, int amount) {
    player.resources[resource] -= amount;
    if (player.resources[resource] < 0) player.resources[resource] = 0;
}

void addResource(Player& player, const std::string& resource, int amount) {
    player.resources[resource] += amount;
}

// ── gainXP ─────────────────────────────────────────────────────────────────
// Awards XP and handles leveling up. Loops until all pending level-ups
// are processed (in case XP gained spans multiple levels).
void gainXP(Player& player, int amount) {
    player.xp += amount;
    while (player.xp >= player.xpToNextLevel) {
        player.xp -= player.xpToNextLevel;
        player.level++;
        player.xpToNextLevel = static_cast<int>(player.xpToNextLevel * 1.4);

        // Level up bonuses by class
        if (player.pClass == PlayerClass::WARRIOR) {
            player.maxHP       += 20;
            player.attackPower += 4;
            player.defense     += 3;
        } else if (player.pClass == PlayerClass::RANGER) {
            player.maxHP       += 12;
            player.attackPower += 5;
            player.defense     += 2;
        } else {
            player.maxHP       += 8;
            player.attackPower += 8;
            player.defense     += 1;
        }
        player.currentHP = player.maxHP; // full heal on level up

        printBanner("LEVEL UP! You are now level " + std::to_string(player.level), Color::BRIGHT_YELLOW);
        pause(800);
    }
}

// ── equipItem ─────────────────────────────────────────────────────────────
// Equips a weapon or armor from the player's inventory by name.
void equipItem(Player& player, const std::string& itemName) {
    for (const auto& item : player.inventory) {
        if (item.name == itemName) {
            if (item.type == "weapon") {
                player.equippedWeapon = itemName;
                printColor("Equipped " + itemName + " as your weapon!", Color::BRIGHT_GREEN);
            } else if (item.type == "armor") {
                player.equippedArmor = itemName;
                printColor("Equipped " + itemName + " as your armor!", Color::BRIGHT_BLUE);
            }
            return;
        }
    }
    printColor("You don't have that item.", Color::BRIGHT_RED);
}

// ── getPlayerAttack / getPlayerDefense ─────────────────────────────────────
// Returns effective attack/defense including equipped gear bonuses.
int getPlayerAttack(const Player& player) {
    int atk = player.attackPower;
    for (const auto& item : player.inventory) {
        if (item.name == player.equippedWeapon && item.type == "weapon") {
            atk += item.value;
        }
    }
    return atk;
}

int getPlayerDefense(const Player& player) {
    int def = player.defense;
    for (const auto& item : player.inventory) {
        if (item.name == player.equippedArmor && item.type == "armor") {
            def += item.value;
        }
    }
    return def;
}

// ── healPlayer ─────────────────────────────────────────────────────────────
void healPlayer(Player& player, int amount) {
    player.currentHP += amount;
    if (player.currentHP > player.maxHP) player.currentHP = player.maxHP;
}