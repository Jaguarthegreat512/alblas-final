#pragma once
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

// ── ANSI Escape Codes (new research topic #1) ──────────────────────────────
// These are special character sequences that tell the terminal to change
// text color, background, or style. Not taught in class — researched independently.
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";

    // Foreground colors
    const std::string BLACK   = "\033[30m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";

    // Bright foreground colors
    const std::string BRIGHT_RED     = "\033[91m";
    const std::string BRIGHT_GREEN   = "\033[92m";
    const std::string BRIGHT_YELLOW  = "\033[93m";
    const std::string BRIGHT_BLUE    = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN    = "\033[96m";
    const std::string BRIGHT_WHITE   = "\033[97m";

    // Background colors
    const std::string BG_BLACK  = "\033[40m";
    const std::string BG_RED    = "\033[41m";
    const std::string BG_GREEN  = "\033[42m";
    const std::string BG_BLUE   = "\033[44m";
    const std::string BG_YELLOW = "\033[43m";
}

// Helper: print a colored line
inline void printColor(const std::string& text, const std::string& color) {
    std::cout << color << text << Color::RESET << "\n";
}

// Helper: print a styled header banner
inline void printBanner(const std::string& text, const std::string& color) {
    std::string border(text.size() + 4, '=');
    std::cout << color << Color::BOLD
              << border << "\n"
              << "  " << text << "\n"
              << border
              << Color::RESET << "\n";
}

// Helper: small delay for dramatic effect
inline void pause(int milliseconds = 600) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// Helper: clear the screen
inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}

// Helper: press enter to continue
inline void pressEnter() {
    std::cout << Color::BRIGHT_WHITE << "\n[ Press ENTER to continue... ]" << Color::RESET;
    std::cin.ignore();
    std::cin.get();
}