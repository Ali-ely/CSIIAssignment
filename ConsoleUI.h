#ifndef SIMWORLD_CONSOLE_UI_H
#define SIMWORLD_CONSOLE_UI_H

#include "Simulation.h"
#include <iosfwd>
#include <string>

// Owns the running application and parses complete input lines. EOF exits even
// partway through a form; invalid tokens never poison the input stream.
class ConsoleUI {
    Simulation simulation_;
    std::istream& in_;
    std::ostream& out_;
    bool readLine(const std::string& prompt, std::string& result);
    bool readInt(const std::string& prompt, int minimum, int maximum, int& result);
    bool readName(std::string& result);
    bool readSettings(EntitySettings& settings);
    bool readId(const std::string& prompt, int& result);
    void menu() const;
public:
    ConsoleUI(std::istream& input, std::ostream& output) : in_(input), out_(output) {}
    void run();
};

#endif
