#include "crafting.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

// ── getAllRecipes ──────────────────────────────────────────────────────────
// Returns the full list of craftable items.
// Organized in tiers: Wood → Stone → Iron → Diamond → Netherite
std::vector<Recipe> getAllRecipes() {
    return {
        // ── Weapons ──────────────────────────────────────────────────────
        {
            "Wooden Sword", "weapon", 5,
            {{"Wood", 3}},
            "A basic sword. Better than nothing."
        },
        {
            "Stone Sword", "weapon", 10,
            {{"Stone", 3}, {"Wood", 1}},
            "A solid stone blade."
        },
        {
            "Iron Sword", "weapon", 18,
            {{"Iron", 3}, {"Wood", 1}},
            "A reliable iron sword."
        },
        {
            "Diamond Sword", "weapon", 30,
            {{"Diamond", 3}, {"Wood", 1}},
            "The blade of a true adventurer."
        },
        {
            "Netherite Sword", "weapon", 45,
            {{"Netherite", 2}, {"Diamond", 2}},
            "Forged in the Nether. Unstoppable."
        },
        // Ranger special
        {
            "Iron Bow", "weapon", 20,
            {{"Iron", 2}, {"String", 3}},
            "A powerful ranged weapon. (+25% for Rangers)"
        },
        // Mage special
        {
            "Blaze Staff", "weapon", 35,
            {{"Blaze Rod", 2}, {"Iron", 2}},
            "Channels the power of the Nether. (+30% for Mages)"
        },

        // ── Armor ────────────────────────────────────────────────────────
        {
            "Leather Armor", "armor", 4,
            {{"Wood", 5}},
            "Scraped together from scraps."
        },
        {
            "Stone Chestplate", "armor", 8,
            {{"Stone", 6}},
            "Heavy but protective."
        },
        {
            "Iron Chestplate", "armor", 15,
            {{"Iron", 6}},
            "Standard adventurer gear."
        },
        {
            "Diamond Chestplate", "armor", 25,
            {{"Diamond", 6}},
            "Gleaming and nearly impenetrable."
        },
        {
            "Netherite Armor", "armor", 38,
            {{"Netherite", 3}, {"Diamond", 4}},
            "The pinnacle of protection."
        },
    };
}

// ── printRecipe ────────────────────────────────────────────────────────────
// Displays a single recipe. Shows green if craftable, red if not.
void printRecipe(const Recipe& recipe, const Player& player) {
    // Check if player has all ingredients
    bool canCraft = true;
    for (const auto& [res, amt] : recipe.ingredients) {
        if (!hasResource(player, res, amt)) {
            canCraft = false;
            break;
        }
    }

    std::string statusColor = canCraft ? Color::BRIGHT_GREEN : Color::BRIGHT_RED;
    std::string statusIcon  = canCraft ? "[✓]" : "[✗]";

    std::cout << statusColor << statusIcon << " " << Color::BOLD
              << recipe.resultName << Color::RESET
              << Color::WHITE << " (+" << recipe.resultValue
              << (recipe.resultType == "weapon" ? " ATK" : " DEF") << ")\n"
              << Color::YELLOW << "     " << recipe.description << "\n"
              << Color::CYAN << "     Needs: ";

    for (size_t i = 0; i < recipe.ingredients.size(); i++) {
        auto& [res, amt] = recipe.ingredients[i];
        bool hasEnough = hasResource(player, res, amt);
        std::cout << (hasEnough ? Color::GREEN : Color::RED)
                  << res << " x" << amt;
        if (i + 1 < recipe.ingredients.size()) std::cout << ", ";
    }
    std::cout << Color::RESET << "\n\n";
}

// ── showCraftingMenu ───────────────────────────────────────────────────────
// Displays all recipes and lets the player craft items.
// Uses a loop to keep the menu open until player exits.
void showCraftingMenu(Player& player) {
    std::vector<Recipe> recipes = getAllRecipes();

    bool inMenu = true;
    while (inMenu) {
        clearScreen();
        printBanner("Crafting Table", Color::BRIGHT_CYAN);

        // Show resources first
        std::cout << Color::BRIGHT_YELLOW << "Your Resources:\n" << Color::RESET;
        for (const auto& [res, amt] : player.resources) {
            if (amt > 0) {
                std::cout << "  " << Color::CYAN << res << ": "
                          << Color::WHITE << amt << Color::RESET << "  ";
            }
        }
        std::cout << "\n\n";

        // List all recipes with index numbers
        std::cout << Color::BRIGHT_WHITE << "Available Recipes:\n\n" << Color::RESET;
        for (size_t i = 0; i < recipes.size(); i++) {
            std::cout << Color::BRIGHT_WHITE << "  " << (i + 1) << ") ";
            printRecipe(recipes[i], player);
        }

        std::cout << Color::BRIGHT_WHITE << "  0) Back\n\n" << Color::RESET;
        std::cout << "Craft which item? ";

        int choice;
        std::cin >> choice;

        if (choice == 0) {
            inMenu = false;
            break;
        }

        // Validate choice range using a conditional
        if (choice < 1 || choice > (int)recipes.size()) {
            printColor("Invalid choice.", Color::BRIGHT_RED);
            pause(400);
            continue;
        }

        Recipe& recipe = recipes[choice - 1];

        // Check if player has all ingredients
        bool canCraft = true;
        for (const auto& [res, amt] : recipe.ingredients) {
            if (!hasResource(player, res, amt)) {
                canCraft = false;
                printColor("You don't have enough " + res + "!", Color::BRIGHT_RED);
                break;
            }
        }

        if (canCraft) {
            // Remove ingredients
            for (const auto& [res, amt] : recipe.ingredients) {
                removeResource(player, res, amt);
            }

            // Add item to inventory
            Item crafted;
            crafted.name     = recipe.resultName;
            crafted.type     = recipe.resultType;
            crafted.value    = recipe.resultValue;
            crafted.quantity = 1;

            // Class bonuses on craft
            if (player.pClass == PlayerClass::RANGER && recipe.resultName == "Iron Bow") {
                crafted.value = static_cast<int>(crafted.value * 1.25);
                printColor("Ranger bonus! Iron Bow power increased!", Color::BRIGHT_GREEN);
            } else if (player.pClass == PlayerClass::MAGE && recipe.resultName == "Blaze Staff") {
                crafted.value = static_cast<int>(crafted.value * 1.30);
                printColor("Mage bonus! Blaze Staff power increased!", Color::BRIGHT_MAGENTA);
            }

            addToInventory(player, crafted);

            printBanner("Crafted: " + recipe.resultName + "!", Color::BRIGHT_GREEN);

            // Auto-equip if better than current gear
            bool shouldEquip = false;
            if (recipe.resultType == "weapon") {
                if (player.equippedWeapon.empty()) {
                    shouldEquip = true;
                }
            } else if (recipe.resultType == "armor") {
                if (player.equippedArmor.empty()) {
                    shouldEquip = true;
                }
            }

            if (shouldEquip) {
                equipItem(player, recipe.resultName);
            } else {
                std::cout << "  Equip it? (y/n): ";
                char eq;
                std::cin >> eq;
                if (eq == 'y' || eq == 'Y') {
                    equipItem(player, recipe.resultName);
                }
            }
        }

        pause(500);
    }
}