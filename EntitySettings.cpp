#include "EntitySettings.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

void EntitySettings::validate() const {
    if (name.empty() || name.size() > 60 ||
        name.find_first_not_of(" \t\r\n") == std::string::npos ||
        std::any_of(name.begin(), name.end(), [](unsigned char character) {
            return std::iscntrl(character) != 0;
        })) {
        throw std::invalid_argument("Name must contain 1-60 readable characters.");
    }
    if (maxHealth < 1 || maxHealth > 1000 || health < 0 || health > maxHealth)
        throw std::invalid_argument("Health must be 0..maximum; maximum must be 1..1000.");
    if (active && health == 0)
        throw std::invalid_argument("An entity with zero health cannot be active.");
    if (maxEnergy < 1 || maxEnergy > 1000 || energy < 0 || energy > maxEnergy)
        throw std::invalid_argument("Energy must be 0..maximum; maximum must be 1..1000.");
    if (skill < 1 || skill > 10)
        throw std::invalid_argument("Skill/efficiency must be 1..10.");
    if (supplies < 0 || supplies > 10000)
        throw std::invalid_argument("Supplies must be 0..10000.");
}
