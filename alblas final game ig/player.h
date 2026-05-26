#pragma once
#include <string>
#include <vector>
#include <map>

// ── Player Class ───────────────────────────────────────────────────────────
// Stores all player data: stats, inventory, and equipment.
// Using a struct-like class here so all player data travels together
// and can be passed by reference to other functions.

enum class PlayerClass {
    WARRIOR,
    RANGER,
    MAGE
};

struct Item {
    std::string name;
    std::string type;   // "weapon", "armor", "resource", "food"
    int value;          // damage bonus, armor bonus, or heal amount
    int quantity;
};

struct Player {
    std::string name;
    PlayerClass pClass;
    std::string className;

    // Core stats
    int maxHP;
    int currentHP;
    int attackPower;
    int defense;
    int level;
    int xp;
    int xpToNextLevel;

    // World state
    int currentBiome;   // 0 = Overworld, 1 = Nether
    int depth;          // how many rooms deep the player is
    int gold;

    // Inventory: map of item name -> Item
    // Using a vector of Items so we can loop over it easily
    std::vector<Item> inventory;

    // Equipped items (by name, "" = nothing)
    std::string equippedWeapon;
    std::string equippedArmor;

    // Resource counts (key Minecraft resources)
    std::map<std::string, int> resources;
};

// ── Function Declarations ──────────────────────────────────────────────────
// These are defined in player.cpp

Player createPlayer(const std::string& name, PlayerClass pClass);
void printPlayerStats(const Player& player);
void addToInventory(Player& player, const Item& item);
bool hasResource(const Player& player, const std::string& resource, int amount);
void removeResource(Player& player, const std::string& resource, int amount);
void addResource(Player& player, const std::string& resource, int amount);
void gainXP(Player& player, int amount);
void equipItem(Player& player, const std::string& itemName);
int getPlayerAttack(const Player& player);
int getPlayerDefense(const Player& player);
void healPlayer(Player& player, int amount);
void printInventory(const Player& player);