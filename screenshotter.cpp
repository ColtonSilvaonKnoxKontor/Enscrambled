// screenshotter.cpp
#include "screenshotter.hpp"
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

bool takeScreenshot(const std::string& filename) {
    std::string cmd1 = "gnome-screenshot -f " + filename + " 2>/dev/null";
    std::string cmd2 = "scrot " + filename + " 2>/dev/null";
    std::string cmd3 = "maim " + filename + " 2>/dev/null";
    std::string cmd4 = "import -window root " + filename + " 2>/dev/null";

    if (system(cmd1.c_str()) == 0 && fs::exists(filename)) return true;
    if (system(cmd2.c_str()) == 0 && fs::exists(filename)) return true;
    if (system(cmd3.c_str()) == 0 && fs::exists(filename)) return true;
    if (system(cmd4.c_str()) == 0 && fs::exists(filename)) return true;
    return false;
}