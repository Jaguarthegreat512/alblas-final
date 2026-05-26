#include "combat.h"
#include "crafting.h"
#include "fileio.h"
#include "player.h"
#include "utils.h"

#include <iostream>
#include <limits>
#include <string>

namespace {

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
    std::cout << "  2) Craft items\n";
    std::cout << "  3) View inventory\n";
    std::cout << "  4) Save game\n";
    std::cout << "  5) Load saved game\n";
    std::cout << "  6) Quit\n\n";
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
                showCraftingMenu(player);
                break;
            case 3:
                clearScreen();
                printInventory(player);
                pressEnter();
                break;
            case 4:
                saveGame(player);
                pressEnter();
                break;
            case 5:
                if (loadGame(player)) {
                    printColor("Save loaded successfully.", Color::BRIGHT_GREEN);
                } else {
                    printColor("No save file found.", Color::BRIGHT_RED);
                }
                pressEnter();
                break;
            case 6:
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
