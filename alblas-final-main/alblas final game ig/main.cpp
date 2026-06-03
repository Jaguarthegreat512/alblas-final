#include "combat.h"
#include "crafting.h"
#include "fileio.h"
#include "player.h"
#include "utils.h"

#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace {

std::mt19937 rng(std::random_device{}());

int readIntChoice(const std::string& prompt) {
    int choice = 0;
    while (true) {
        std::cout << prompt;
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter a valid number.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return choice;
    }
}

int randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

void advanceTime(Player& player, int minHours, int maxHours) {
    int hours = randomInt(minHours, maxHours);
    player.hour += hours;
    while (player.hour >= 24) {
        player.hour -= 24;
        player.day += 1;
    }
}

bool consumeInventoryItem(Player& player, const std::string& itemName, int amount) {
    for (auto it = player.inventory.begin(); it != player.inventory.end(); ++it) {
        if (it->name == itemName && it->quantity >= amount) {
            it->quantity -= amount;
            if (it->quantity <= 0) {
                player.inventory.erase(it);
            }
            return true;
        }
    }
    return false;
}

void mineResources(Player& player) {
    std::vector<std::string> resources = {"Wood", "Stone", "Iron", "Diamond"};
    std::discrete_distribution<int> roll({40, 30, 20, 10});
    int index = roll(rng);
    int amount = randomInt(1, 3);

    addResource(player, resources[index], amount);
    printColor("You mined " + std::to_string(amount) + " " + resources[index] + "!", Color::BRIGHT_GREEN);

    if (resources[index] == "Diamond" && randomInt(1, 20) == 1) {
        addResource(player, "Netherite", 1);
        printColor("Lucky find! You also discovered 1 Netherite.", Color::BRIGHT_MAGENTA);
    }

    advanceTime(player, 2, 5);
}

void huntForFood(Player& player) {
    int amount = randomInt(1, 4);
    addResource(player, "Food", amount);
    printColor("You gathered " + std::to_string(amount) + " food supplies.", Color::BRIGHT_GREEN);
    advanceTime(player, 1, 3);
}

void cookFood(Player& player) {
    if (!hasResource(player, "Food", 2)) {
        printColor("You need at least 2 food to cook a meal.", Color::BRIGHT_RED);
        return;
    }

    removeResource(player, "Food", 2);
    Item meal;
    meal.name = "Cooked Meal";
    meal.type = "food";
    meal.value = 35;
    meal.quantity = 1;
    addToInventory(player, meal);

    printColor("You cooked a Cooked Meal. It will heal more when eaten.", Color::BRIGHT_YELLOW);
    advanceTime(player, 1, 2);
}

void eatFood(Player& player) {
    if (consumeInventoryItem(player, "Cooked Meal", 1)) {
        healPlayer(player, 35);
        printColor("You ate a Cooked Meal and restored health.", Color::BRIGHT_GREEN);
        advanceTime(player, 1, 1);
        return;
    }

    if (hasResource(player, "Food", 1)) {
        removeResource(player, "Food", 1);
        healPlayer(player, 20);
        printColor("You ate some food and recovered health.", Color::BRIGHT_GREEN);
        advanceTime(player, 1, 1);
        return;
    }

    printColor("You have no food to eat.", Color::BRIGHT_RED);
}

void sleepAtCamp(Player& player) {
    if (hasResource(player, "Food", 1)) {
        removeResource(player, "Food", 1);
        healPlayer(player, 40);
        printColor("You slept well and used 1 food to recover.", Color::BRIGHT_GREEN);
    } else {
        healPlayer(player, 20);
        printColor("You slept without food and recovered a little.", Color::BRIGHT_YELLOW);
    }
    player.hour = 7;
    player.day += 1;
}

void returnToOverworld(Player& player) {
    if (player.currentBiome == 0 && player.depth == 0) {
        printColor("You are already on the surface.", Color::BRIGHT_YELLOW);
        return;
    }

    player.depth = 0;
    player.currentBiome = 0;
    printColor("You returned to the Overworld and surfaced safely.", Color::BRIGHT_CYAN);
}

void showOverworldMenu(Player& player) {
    bool inMenu = true;
    while (inMenu) {
        clearScreen();
        printBanner("Overworld Actions", Color::BRIGHT_GREEN);
        printPlayerStats(player);
        std::cout << "\n  1) Mine for resources\n";
        std::cout << "  2) Hunt for food\n";
        std::cout << "  3) Cook food\n";
        std::cout << "  4) Eat food\n";
        std::cout << "  5) Sleep (advance to next day)\n";
        std::cout << "  6) Return to the Overworld surface\n";
        std::cout << "  0) Back\n\n";

        int choice = readIntChoice("Choice: ");
        clearScreen();

        switch (choice) {
            case 1:
                mineResources(player);
                break;
            case 2:
                huntForFood(player);
                break;
            case 3:
                cookFood(player);
                break;
            case 4:
                eatFood(player);
                break;
            case 5:
                sleepAtCamp(player);
                break;
            case 6:
                returnToOverworld(player);
                break;
            case 0:
                inMenu = false;
                continue;
            default:
                printColor("Invalid choice.", Color::BRIGHT_RED);
                break;
        }

        pressEnter();
    }
}

PlayerClass chooseClass() {
    while (true) {
        std::cout << "Choose your class:\n";
        std::cout << "  1) Warrior\n";
        std::cout << "  2) Ranger\n";
        std::cout << "  3) Mage\n";

        int choice = readIntChoice("Choice: ");
        if (choice == 1) return PlayerClass::WARRIOR;
        if (choice == 2) return PlayerClass::RANGER;
        if (choice == 3) return PlayerClass::MAGE;

        std::cout << "Invalid class selection.\n\n";
    }
}

std::string readPlayerName() {
    std::string name;
    while (true) {
        std::cout << "Enter your character name: ";
        std::getline(std::cin, name);
        if (!name.empty()) {
            return name;
        }
        std::cout << "Name cannot be empty.\n";
    }
}

void showMainMenu(const Player& player) {
    std::cout << Color::BRIGHT_CYAN << "=== Terminal Adventure ===" << Color::RESET << "\n\n";
    printPlayerStats(player);
    std::cout << "\n";
    std::cout << "  1) Explore the next area\n";
    std::cout << "  2) Overworld actions\n";
    std::cout << "  3) Craft items\n";
    std::cout << "  4) View inventory\n";
    std::cout << "  5) Save game\n";
    std::cout << "  6) Load saved game\n";
    std::cout << "  7) Quit\n\n";
}

void updateBiome(Player& player) {
    player.currentBiome = (player.depth >= 5) ? 1 : 0;
}

} // namespace

int main() {
    clearScreen();
    printBanner("CraftQuest Terminal", Color::BRIGHT_CYAN);

    Player player;

    int mode = readIntChoice("  1) New Game\n  2) Load Save\nChoice: ");
    if (mode == 2 && loadGame(player)) {
        printColor("Loaded your saved adventure!", Color::BRIGHT_GREEN);
        pressEnter();
    } else {
        std::string name = readPlayerName();
        PlayerClass pClass = chooseClass();
        player = createPlayer(name, pClass);
        printColor("Welcome, " + player.name + "!", Color::BRIGHT_GREEN);
        pressEnter();
    }

    bool running = true;
    while (running) {
        clearScreen();
        showMainMenu(player);

        int choice = readIntChoice("Choice: ");

        switch (choice) {
            case 1: {
                clearScreen();
                updateBiome(player);
                std::string biomeName = (player.currentBiome == 0) ? "Overworld" : "Nether";
                printBanner("Exploring: " + biomeName, Color::BRIGHT_YELLOW);

                Enemy enemy = generateEnemy(player.depth, player.currentBiome);
                runCombat(player, enemy);
                player.depth += 1;
                updateBiome(player);

                std::cout << Color::BRIGHT_CYAN << "You continue deeper into the "
                          << biomeName << ".\n" << Color::RESET;
                pressEnter();
                break;
            }
            case 2:
                clearScreen();
                showOverworldMenu(player);
                break;
            case 3:
                clearScreen();
                showCraftingMenu(player);
                break;
            case 4:
                clearScreen();
                printInventory(player);
                pressEnter();
                break;
            case 5:
                saveGame(player);
                pressEnter();
                break;
            case 6:
                if (loadGame(player)) {
                    printColor("Save loaded successfully.", Color::BRIGHT_GREEN);
                } else {
                    printColor("No save file found.", Color::BRIGHT_RED);
                }
                pressEnter();
                break;
            case 7:
                clearScreen();
                printBanner("Thanks for playing!", Color::BRIGHT_MAGENTA);
                running = false;
                break;
            default:
                printColor("Invalid choice.", Color::BRIGHT_RED);
                pressEnter();
                break;
        }
    }

    return 0;
}
