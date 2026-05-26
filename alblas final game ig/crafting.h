#pragma once
#include "player.h"
#include <string>
#include <vector>

// ── Recipe Struct ──────────────────────────────────────────────────────────
struct Recipe {
    std::string resultName;
    std::string resultType;   // "weapon" or "armor"
    int resultValue;          // damage or defense bonus
    std::vector<std::pair<std::string, int>> ingredients; // {resource, amount}
    std::string description;
};

// ── Function Declarations ──────────────────────────────────────────────────
std::vector<Recipe> getAllRecipes();
void showCraftingMenu(Player& player);
void printRecipe(const Recipe& recipe, const Player& player);