#pragma once
#include "player.h"

// ── File I/O (new research topic #3) ──────────────────────────────────────
// Functions to save and load game state from a .txt file.
// Uses fstream — not taught in class, researched independently.

void saveGame(const Player& player, const std::string& filename = "craftquest_save.txt");
bool loadGame(Player& player, const std::string& filename = "craftquest_save.txt");
bool saveExists(const std::string& filename = "craftquest_save.txt");
void deleteSave(const std::string& filename = "craftquest_save.txt");