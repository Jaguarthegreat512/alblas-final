#include "fileio.h"
#include "utils.h"
#include <cstdio>
#include <fstream>
#include <iostream>

// ── saveGame ───────────────────────────────────────────────────────────────
// Writes the entire player state to a plain text file.
// Each field is on its own line so loadGame can read them back in order.
// This is the new research topic: using fstream for file output.
void saveGame(const Player& player, const std::string& filename) {
    std::ofstream file(filename); // Open file for writing (creates if not exists)

    if (!file.is_open()) {
        printColor("ERROR: Could not save game!", Color::BRIGHT_RED);
        return;
    }

    // Write core stats
    file << player.name << "\n";
    file << static_cast<int>(player.pClass) << "\n";
    file << player.className << "\n";
    file << player.maxHP << "\n";
    file << player.currentHP << "\n";
    file << player.attackPower << "\n";
    file << player.defense << "\n";
    file << player.level << "\n";
    file << player.xp << "\n";
    file << player.xpToNextLevel << "\n";
    file << player.currentBiome << "\n";
    file << player.depth << "\n";
    file << player.day << "\n";
    file << player.hour << "\n";
    file << player.gold << "\n";
    file << player.equippedWeapon << "\n";
    file << player.equippedArmor << "\n";

    // Write resources (loop through map)
    file << player.resources.size() << "\n";
    for (const auto& [res, amt] : player.resources) {
        file << res << "\n" << amt << "\n";
    }

    // Write inventory items (loop through vector)
    file << player.inventory.size() << "\n";
    for (const auto& item : player.inventory) {
        file << item.name << "\n";
        file << item.type << "\n";
        file << item.value << "\n";
        file << item.quantity << "\n";
    }

    file.close();
    printColor("Game saved!", Color::BRIGHT_GREEN);
}

// ── loadGame ───────────────────────────────────────────────────────────────
// Reads player state back from the save file.
// Returns true if successful, false if file not found or corrupted.
bool loadGame(Player& player, const std::string& filename) {
    std::ifstream file(filename); // Open file for reading

    if (!file.is_open()) {
        return false; // No save file found
    }

    int pClassInt;
    file >> std::ws; std::getline(file, player.name);
    file >> pClassInt; file.ignore();
    player.pClass = static_cast<PlayerClass>(pClassInt);
    std::getline(file, player.className);
    file >> player.maxHP;
    file >> player.currentHP;
    file >> player.attackPower;
    file >> player.defense;
    file >> player.level;
    file >> player.xp;
    file >> player.xpToNextLevel;
    file >> player.currentBiome;
    file >> player.depth;
    file >> player.day;
    file >> player.hour;
    file >> player.gold;
    file.ignore();
    std::getline(file, player.equippedWeapon);
    std::getline(file, player.equippedArmor);

    // Read resources
    int resCount;
    file >> resCount;
    file.ignore();
    player.resources.clear();
    for (int i = 0; i < resCount; i++) {
        std::string resName;
        int resAmt;
        std::getline(file, resName);
        file >> resAmt;
        file.ignore();
        player.resources[resName] = resAmt;
    }

    // Read inventory
    int invCount;
    file >> invCount;
    file.ignore();
    player.inventory.clear();
    for (int i = 0; i < invCount; i++) {
        Item item;
        std::getline(file, item.name);
        std::getline(file, item.type);
        file >> item.value;
        file >> item.quantity;
        file.ignore();
        player.inventory.push_back(item);
    }

    file.close();
    return true;
}

// ── saveExists ─────────────────────────────────────────────────────────────
bool saveExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.is_open();
}

// ── deleteSave ─────────────────────────────────────────────────────────────
void deleteSave(const std::string& filename) {
    std::remove(filename.c_str());
}